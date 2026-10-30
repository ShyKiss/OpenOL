#!/usr/bin/env python3
"""Protocol tests for the standalone relay. Runs without the game.

Builds server/src with ASan and UBSan into a temp directory, then talks to it over
UDP. Requires cmake, ninja and a C/C++ compiler.

    python3 Tests/test_relay.py [-v]
"""
import os
import random
import re
import select
import shutil
import socket
import struct
import subprocess
import tempfile
import time
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
MP = os.path.dirname(HERE)
SERVER_SRC = os.path.join(MP, "server", "src")

PORT = 27790
MAGIC = 0xF3
ROOM = "PUBLIC"


def load_protocol():
    ids = {}
    with open(os.path.join(SERVER_SRC, "openol_protocol.h"), encoding="utf-8") as f:
        for name, value in re.findall(r"^#define (\w+)\s+(0x[0-9A-Fa-f]+|\d+)", f.read(), re.M):
            ids[name] = int(value, 0)
    return ids


P = load_protocol()


def lane_for(channel, ptype):
    if channel == P["CH_HERO"] and ptype in (P["HERO_STATE"], P["HERO_HEAD_ROT"]):
        return 0
    if channel == P["CH_ENEMY"] and ptype == P["ENEMY_LOC"]:
        return 0
    if channel == P["CH_SRV"] and ptype == P["SRV_PING"]:
        return 0
    return 1


class Client:
    """Minimal client of the ol_transport protocol over a lossless loopback."""

    def __init__(self, room=ROOM):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(("127.0.0.1", 0))
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 1 << 22)
        self.sock.connect(("127.0.0.1", PORT))
        self.inbox = []
        self.conn_id = 0
        self.seq = 0
        self.out_id = 0
        self.in_next = 0
        self.in_buf = {}
        self.sock.send(("HELLO,%s,\n" % room).encode())
        end = time.monotonic() + 2.0
        ready = None
        while time.monotonic() < end and not ready:
            if select.select([self.sock], [], [], end - time.monotonic())[0]:
                data = self.sock.recv(4096)
                if data[0] == P["SRV_READY"]:
                    ready = data
        if not ready:
            raise AssertionError("no READY")
        self.player_id = struct.unpack_from("<I", ready, 5)[0]
        name_len = ready[9]
        self.token = ready[10 + name_len:10 + name_len + 32]
        self.conn_id = struct.unpack_from("<Q", ready, 10 + name_len + 32)[0]

    def enter(self, cp="CP_A", persist="MapA", epoch=0, reset=False):
        body = struct.pack("<HB", epoch, 1 if reset else 0) + wstr(cp) + wstr(persist)
        self.send(P["CH_SESS"], P["SESS_ENTER"], body)

    def header(self):
        self.seq = (self.seq + 1) & 0xFFFF
        return struct.pack("<BQHHI", MAGIC, self.conn_id, self.seq, self.in_next, 0)

    def raw_message(self, lane, msg):
        if lane == 0:
            return bytes([0]) + struct.pack("<H", len(msg)) + msg
        body = bytes([1]) + struct.pack("<HH", self.out_id, len(msg)) + msg
        self.out_id = (self.out_id + 1) & 0xFFFF
        return body

    def send(self, channel, ptype, payload=b""):
        msg = bytes([channel, ptype]) + struct.pack("<I", self.player_id) + payload
        self.sock.send(self.header() + self.raw_message(lane_for(channel, ptype), msg))

    def on_datagram(self, data):
        if data[0] != MAGIC:
            return
        off, acked = 17, False
        while off < len(data):
            lane = data[off]
            if lane == 0:
                n = struct.unpack_from("<H", data, off + 1)[0]
                self.inbox.append(data[off + 3:off + 3 + n])
                off += 3 + n
            else:
                mid, n = struct.unpack_from("<HH", data, off + 1)
                if mid not in self.in_buf and ((mid - self.in_next) & 0xFFFF) < 0x8000:
                    self.in_buf[mid] = data[off + 5:off + 5 + n]
                while self.in_next in self.in_buf:
                    self.inbox.append(self.in_buf.pop(self.in_next))
                    self.in_next = (self.in_next + 1) & 0xFFFF
                off += 5 + n
                acked = True
        if acked:
            self.sock.send(self.header())

    def pump(self, timeout):
        if select.select([self.sock], [], [], timeout)[0]:
            self.on_datagram(self.sock.recv(4096))
        msgs, self.inbox = self.inbox, []
        return msgs

    def recv_all(self, duration):
        packets, end = [], time.monotonic() + duration
        while True:
            left = end - time.monotonic()
            if left <= 0:
                return packets
            packets += self.pump(left)

    def wait_for(self, match, timeout=2.0):
        end = time.monotonic() + timeout
        while time.monotonic() < end:
            msgs = self.pump(end - time.monotonic())
            for i, data in enumerate(msgs):
                if match(data):
                    self.inbox = msgs[i + 1:] + self.inbox
                    return data
        raise AssertionError("expected packet did not arrive")

    def close(self):
        self.sock.close()


def wstr(text):
    return bytes([len(text)]) + text.encode()


def world_msg(data):
    epoch = struct.unpack_from("<H", data, 6)[0]
    n = data[8]
    cp = data[9:9 + n].decode()
    m = data[9 + n]
    return epoch, cp, data[10 + n:10 + n + m].decode()


def sess(ptype):
    return lambda d: d[0] == P["CH_SESS"] and d[1] == P[ptype]


def enemy(name, tail=b""):
    return bytes([len(name)]) + name + tail


def push_state(key, pushing):
    return struct.pack("<iiiiI", *key, 1000, 1) + bytes([1 if pushing else 0])


class RelayTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.build_dir = tempfile.mkdtemp(prefix="ol_relay_test_")
        flags = "-fsanitize=address,undefined -g"
        subprocess.check_call(["cmake", "-S", SERVER_SRC, "-B", cls.build_dir, "-G", "Ninja",
                               "-DCMAKE_BUILD_TYPE=Debug", "-DCMAKE_C_FLAGS=" + flags,
                               "-DCMAKE_CXX_FLAGS=" + flags], stdout=subprocess.DEVNULL)
        subprocess.check_call(["cmake", "--build", cls.build_dir], stdout=subprocess.DEVNULL)
        cls.exe = os.path.join(cls.build_dir, "OpenOL_Relay")

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.build_dir, ignore_errors=True)

    def setUp(self):
        self.clients = []
        db = os.path.join(self.build_dir, "relay.db")
        if os.path.exists(db):
            os.remove(db)
        self.log = tempfile.TemporaryFile()
        self.server = subprocess.Popen([self.exe, "--port", str(PORT)], cwd=self.build_dir,
                                       stdin=subprocess.DEVNULL, stdout=self.log,
                                       stderr=subprocess.STDOUT)
        time.sleep(0.3)
        self.assertIsNone(self.server.poll(), "server failed to start")

    def tearDown(self):
        for c in self.clients:
            c.close()
        crashed = self.server.poll() is not None
        self.server.terminate()
        self.server.wait()
        self.log.seek(0)
        output = self.log.read().decode("utf-8", "replace")
        self.log.close()
        self.assertFalse(crashed, "server exited during the test:\n" + output[-4000:])
        self.assertNotIn("Sanitizer", output, output[-4000:])

    def client(self, enter=True):
        c = Client()
        self.clients.append(c)
        if enter:
            c.enter()
            c.wait_for(sess("SESS_BASELINE_END"))
        return c

    def assert_alive(self, client):
        client.send(P["CH_SRV"], P["SRV_PING"], b"\x01\x02\x03\x04")
        client.wait_for(lambda d: d[0] == P["CH_SRV"] and d[1] == P["SRV_PING"])

    def test_password_never_logged(self):
        with open(os.path.join(SERVER_SRC, "server.c"), encoding="utf-8") as f:
            for line in f:
                if "server_log" in line or "history_push" in line:
                    self.assertNotIn("password", line)

    def test_session_tokens_unique(self):
        a, b = self.client(), self.client()
        self.assertEqual(len(a.token), 32)
        self.assertNotEqual(a.token, b.token)
        self.assertNotEqual(a.token, bytes(32))

    def test_unknown_room_rejected(self):
        with self.assertRaises(AssertionError):
            self.clients.append(Client("NO_SUCH_ROOM"))

    def test_relay_preserves_sender(self):
        a, b = self.client(), self.client()
        a.send(P["CH_HERO"], P["HERO_HEAD_ROT"], b"\x11" * 8)
        data = b.wait_for(lambda d: d[0] == P["CH_HERO"] and d[1] == P["HERO_HEAD_ROT"])
        self.assertEqual(struct.unpack_from("<I", data, 2)[0], a.player_id)

    def test_enemy_snapshot_uses_client_numbering(self):
        a = self.client()
        name = b"OLEnemyPawn_3"
        a.send(P["CH_ENEMY"], P["ENEMY_SPAWN"], enemy(name, b"S" * 20))
        a.send(P["CH_ENEMY"], P["ENEMY_LOC"], enemy(name, b"\0" + b"L" * 40))
        a.send(P["CH_ENEMY"], P["ENEMY_DOOR_BREAK"], enemy(name, b"B" * 13))
        time.sleep(0.1)
        b = self.client(enter=False)
        b.enter()
        got = [d for d in b.recv_all(0.5) if d[0] == P["CH_ENEMY"]]
        self.assertEqual([d[1] for d in got], [P["ENEMY_SPAWN"], P["ENEMY_LOC"]])
        self.assertIn(b"L" * 40, got[1])

    def test_push_denied_wire_format(self):
        a, b = self.client(), self.client()
        key = (10, -20, 30)
        a.send(P["CH_PUSH"], P["PUSH_STATE"], push_state(key, True))
        b.wait_for(lambda d: d[0] == P["CH_PUSH"] and d[1] == P["PUSH_STATE"])
        b.send(P["CH_PUSH"], P["PUSH_STATE"], push_state(key, True))
        deny = b.wait_for(lambda d: d[0] == P["CH_PUSH"] and d[1] == P["PUSH_DENIED"])
        # The client parser needs the 6-byte header and at least 12 payload bytes.
        self.assertEqual(len(deny), P["OL_HEADER_SIZE"] + 12)
        self.assertEqual(struct.unpack_from("<I", deny, 2)[0], 0)
        self.assertEqual(struct.unpack_from("<iii", deny, 6), key)
        self.assertFalse([d for d in a.recv_all(0.2) if d[:2] == bytes([P["CH_PUSH"], P["PUSH_STATE"]])],
                         "denied push must not be relayed")

    def test_push_owner_released_on_stop_and_disconnect(self):
        a, b = self.client(), self.client()
        key = (1, 2, 3)
        is_state = lambda d: d[0] == P["CH_PUSH"] and d[1] == P["PUSH_STATE"]
        a.send(P["CH_PUSH"], P["PUSH_STATE"], push_state(key, True))
        b.wait_for(is_state)
        a.send(P["CH_PUSH"], P["PUSH_STATE"], push_state(key, False))
        b.wait_for(is_state)
        b.send(P["CH_PUSH"], P["PUSH_STATE"], push_state(key, True))
        a.wait_for(is_state)
        b.send(P["CH_WORLD"], P["WORLD_DISCONNECT"])
        a.wait_for(lambda d: d[0] == P["SRV_DISCONNECT"])
        a.send(P["CH_PUSH"], P["PUSH_STATE"], push_state(key, True))
        self.assertFalse([d for d in a.recv_all(0.2) if d[:2] == bytes([P["CH_PUSH"], P["PUSH_DENIED"]])])

    def test_door_lock_conflict(self):
        a, b = self.client(), self.client()
        key = struct.pack("<iii", 5, 6, 7) + b"\0" * 8
        a.send(P["CH_DOOR"], P["DOOR_LOCK"], key)
        b.wait_for(lambda d: d[0] == P["CH_DOOR"] and d[1] == P["DOOR_LOCK"])
        b.send(P["CH_DOOR"], P["DOOR_LOCK"], key)
        b.wait_for(lambda d: d[0] == P["SRV_DOOR_DENY"])

    def test_snapshot_drip_independent_of_load(self):
        a = self.client()
        count = 400
        for i in range(count):
            a.send(P["CH_WORLD"], P["WORLD_PICKUP_STATE"], struct.pack("<iii", i, i, i))
            if i % 50 == 0:
                time.sleep(0.01)
        time.sleep(0.1)
        b = self.client(enter=False)
        b.enter()
        start = time.monotonic()
        arrivals = []
        end = start + 3.0
        while time.monotonic() < end and len(arrivals) < count:
            # Keep the server's main loop busy: this used to speed up the drip.
            a.send(P["CH_SRV"], P["SRV_PING"], b"\0\0\0\0")
            for d in b.pump(0.001):
                if d[0] == P["CH_WORLD"] and d[1] == P["WORLD_PICKUP_STATE"]:
                    arrivals.append(time.monotonic())
        self.assertEqual(len(arrivals), count, "snapshots were lost")
        rate = count / max(arrivals[-1] - arrivals[0], 1e-3)
        self.assertLess(rate, 1200, "drip rate must stay near 800 packets/s under load")

    def test_join_same_world_gets_baseline(self):
        a = self.client()
        a.send(P["CH_WORLD"], P["WORLD_PICKUP_STATE"], struct.pack("<iii", 1, 2, 3))
        time.sleep(0.05)
        b = self.client(enter=False)
        self.assertFalse([d for d in b.recv_all(0.2) if d[0] == P["CH_WORLD"]],
                         "no world data before SESS_ENTER")
        b.enter()
        epoch = world_msg(b.wait_for(sess("SESS_WORLD")))[0]
        self.assertGreater(epoch, 0)
        msgs = b.recv_all(0.3)
        order = [(d[0], d[1]) for d in msgs]
        self.assertIn((P["CH_WORLD"], P["WORLD_PICKUP_STATE"]), order)
        self.assertLess(order.index((P["CH_WORLD"], P["WORLD_PICKUP_STATE"])),
                        order.index((P["CH_SESS"], P["SESS_BASELINE_END"])))

    def test_fresh_joiner_is_sent_to_running_world(self):
        a = self.client()
        b = self.client(enter=False)
        b.enter(cp="", persist="DLC_Intro_Persistent")
        self.assertEqual(world_msg(b.wait_for(sess("SESS_TRAVEL")))[1:], ("CP_A", "MapA"))
        a.send(P["CH_HERO"], P["HERO_HEAD_ROT"], b"\x11" * 8)
        self.assertFalse([d for d in b.recv_all(0.2) if d[0] == P["CH_HERO"]],
                         "not in the world until it arrives")
        b.enter(cp="CP_A", persist="MapA", epoch=1)
        b.wait_for(sess("SESS_WORLD"))
        a.send(P["CH_HERO"], P["HERO_HEAD_ROT"], b"\x11" * 8)
        b.wait_for(lambda d: d[0] == P["CH_HERO"])

    def test_alone_in_other_map_replaces_world(self):
        a = self.client()
        a.enter(cp="CP_B", persist="MapB", epoch=1)
        epoch, cp, persist = world_msg(a.wait_for(sess("SESS_WORLD")))
        self.assertEqual((epoch, cp, persist), (2, "CP_B", "MapB"))

    def test_reset_clears_world_and_moves_others(self):
        a, b = self.client(), self.client()
        a.send(P["CH_WORLD"], P["WORLD_PICKUP_STATE"], struct.pack("<iii", 7, 7, 7))
        b.wait_for(lambda d: d[0] == P["CH_WORLD"])
        a.enter(cp="CP_B", persist="MapB", epoch=1, reset=True)
        epoch, cp, persist = world_msg(b.wait_for(sess("SESS_TRAVEL")))
        self.assertEqual((epoch, cp, persist), (2, "CP_B", "MapB"))
        b.enter(cp="CP_B", persist="MapB", epoch=2)
        msgs = b.recv_all(0.3)
        self.assertIn((P["CH_SESS"], P["SESS_BASELINE_END"]), [(d[0], d[1]) for d in msgs])
        self.assertFalse([d for d in msgs if d[0] == P["CH_WORLD"]], "old world leaked into the new one")

    def test_stale_enter_does_not_replace_world(self):
        a, b = self.client(), self.client()
        a.enter(cp="CP_B", persist="MapB", epoch=1, reset=True)
        b.wait_for(sess("SESS_TRAVEL"))
        # b finished respawning on the old map before it applied the travel.
        b.enter(cp="CP_A", persist="MapA", epoch=1)
        self.assertFalse([d for d in a.recv_all(0.2) if d[:2] == bytes([P["CH_SESS"], P["SESS_TRAVEL"]])])

    def test_checkpoint_updates_world_for_joiners(self):
        a = self.client()
        a.send(P["CH_SESS"], P["SESS_CHECKPOINT"], wstr("CP_A2") + wstr("MapA"))
        time.sleep(0.05)
        b = self.client(enter=False)
        b.enter(cp="", persist="Lobby")
        self.assertEqual(world_msg(b.wait_for(sess("SESS_TRAVEL")))[1:], ("CP_A2", "MapA"))

    def test_checkpoint_on_new_map_moves_others(self):
        a, b = self.client(), self.client()
        a.send(P["CH_SESS"], P["SESS_CHECKPOINT"], wstr("CP_C") + wstr("MapC"))
        self.assertEqual(world_msg(b.wait_for(sess("SESS_TRAVEL")))[1:], ("CP_C", "MapC"))

    def test_leave_notifies_and_stops_traffic(self):
        a, b = self.client(), self.client()
        a.send(P["CH_SESS"], P["SESS_LEAVE"])
        left = b.wait_for(sess("SESS_PLAYER_LEFT"))
        self.assertEqual(struct.unpack_from("<I", left, 6)[0], a.player_id)
        a.send(P["CH_HERO"], P["HERO_HEAD_ROT"], b"\x11" * 8)
        self.assertFalse([d for d in b.recv_all(0.2) if d[0] == P["CH_HERO"]])

    def test_empty_room_forgets_world(self):
        a = self.client()
        a.send(P["CH_WORLD"], P["WORLD_DISCONNECT"])
        time.sleep(0.05)
        b = self.client(enter=False)
        b.enter(cp="CP_Z", persist="MapZ")
        self.assertEqual(world_msg(b.wait_for(sess("SESS_WORLD")))[1:], ("CP_Z", "MapZ"))

    def test_malformed_session_packets(self):
        a = self.client()
        for body in (b"", b"\x00", b"\x01\x00\x00", b"\x01\x00\x00\xff", b"\x01\x00\x00\x01a\x7fb",
                     b"\x01\x00\x00\x00\x00", b"\xff" * 300):
            a.send(P["CH_SESS"], P["SESS_ENTER"], body)
            a.send(P["CH_SESS"], P["SESS_CHECKPOINT"], body)
        self.assert_alive(a)

    def fire(self, client, path, max_count):
        client.send(P["CH_WORLD"], P["WORLD_TRIGGER_FIRE"], bytes([max_count]) + wstr(path))

    def test_one_shot_trigger_fires_once(self):
        a, b = self.client(), self.client()
        is_fire = lambda d: d[:2] == bytes([P["CH_WORLD"], P["WORLD_TRIGGER_FIRE"]])
        self.fire(a, "Map.Touch_1", 1)
        self.fire(b, "Map.Touch_1", 1)
        got_a = [d for d in a.recv_all(0.2) if is_fire(d)]
        got_b = b.recv_all(0.2)
        self.assertEqual(len(got_a), 1, "the instigator hears its own commit once")
        self.assertEqual(struct.unpack_from("<I", got_a[0], 2)[0], a.player_id)
        self.assertEqual(len([d for d in got_b if is_fire(d)]), 1)
        denied = [d for d in got_b if d[:2] == bytes([P["CH_WORLD"], P["WORLD_TRIGGER_DENIED"]])]
        self.assertEqual(len(denied), 1)
        self.assertEqual(denied[0][6:], wstr("Map.Touch_1"))

    def test_multi_use_trigger_respects_limit(self):
        a = self.client()
        is_fire = lambda d: d[:2] == bytes([P["CH_WORLD"], P["WORLD_TRIGGER_FIRE"]])
        for _ in range(4):
            self.fire(a, "Map.Touch_2", 2)
        self.assertEqual(len([d for d in a.recv_all(0.2) if is_fire(d)]), 2)

    def test_repeatable_trigger_dedupes_only_inside_window(self):
        a, b = self.client(), self.client()
        is_fire = lambda d: d[:2] == bytes([P["CH_WORLD"], P["WORLD_TRIGGER_FIRE"]])
        self.fire(a, "Map.Touch_3", 0)
        self.fire(b, "Map.Touch_3", 0)
        self.assertEqual(len([d for d in a.recv_all(0.1) if is_fire(d)]), 1)
        time.sleep(0.3)
        self.fire(b, "Map.Touch_3", 0)
        self.assertEqual(len([d for d in a.recv_all(0.1) if is_fire(d)]), 1)

    def test_new_world_forgets_triggers(self):
        a = self.client()
        is_fire = lambda d: d[:2] == bytes([P["CH_WORLD"], P["WORLD_TRIGGER_FIRE"]])
        self.fire(a, "Map.Touch_4", 1)
        self.assertEqual(len([d for d in a.recv_all(0.1) if is_fire(d)]), 1)
        a.enter(cp="CP_A", persist="MapA", epoch=1, reset=True)
        a.wait_for(sess("SESS_BASELINE_END"))
        self.fire(a, "Map.Touch_4", 1)
        self.assertEqual(len([d for d in a.recv_all(0.1) if is_fire(d)]), 1)

    def claim(self, client, kind, path, max_count):
        client.send(P["CH_WORLD"], P["WORLD_CLAIM"], bytes([kind, max_count]) + wstr(path))

    def test_claim_has_one_winner_and_loser_learns_first(self):
        a, b = self.client(), self.client()
        self.claim(a, P["CLAIM_CSA"], "Map.CSA_1", 1)
        self.claim(b, P["CLAIM_CSA"], "Map.CSA_1", 1)
        msgs = [d for d in b.recv_all(0.2) if d[0] == P["CH_WORLD"]]
        types = [d[1] for d in msgs]
        self.assertEqual(types, [P["WORLD_CLAIM"], P["WORLD_CLAIM_DENIED"]])
        self.assertEqual(struct.unpack_from("<I", msgs[0], 2)[0], a.player_id)
        self.assertEqual(msgs[0][-1], 1, "the grant carries the use count")
        self.assertEqual(msgs[1][6:], bytes([P["CLAIM_CSA"]]) + wstr("Map.CSA_1"))
        self.assertEqual([d[1] for d in a.recv_all(0.1) if d[0] == P["CH_WORLD"]], [P["WORLD_CLAIM"]])

    def test_released_claim_can_be_taken_again(self):
        a, b = self.client(), self.client()
        body = bytes([P["CLAIM_PICKUP"], 1]) + wstr("Map.Key")
        self.claim(a, P["CLAIM_PICKUP"], "Map.Key", 1)
        b.wait_for(lambda d: d[:2] == bytes([P["CH_WORLD"], P["WORLD_CLAIM"]]))
        a.send(P["CH_WORLD"], P["WORLD_CLAIM_RELEASE"], body)
        rel = b.wait_for(lambda d: d[:2] == bytes([P["CH_WORLD"], P["WORLD_CLAIM_RELEASE"]]))
        self.assertEqual(rel[-1], 0, "the release carries the new use count")
        self.claim(b, P["CLAIM_PICKUP"], "Map.Key", 1)
        got = b.wait_for(lambda d: d[0] == P["CH_WORLD"] and d[1] in (P["WORLD_CLAIM"], P["WORLD_CLAIM_DENIED"]))
        self.assertEqual(got[1], P["WORLD_CLAIM"])

    def test_matinee_state_only_reaches_joiners(self):
        a, b = self.client(), self.client()
        state = bytes([1]) + wstr("Map.Interp") + struct.pack("<ff", 2.5, 1.0)
        a.send(P["CH_WORLD"], P["WORLD_MATINEE_STATE"], state)
        is_matinee = lambda d: d[:2] == bytes([P["CH_WORLD"], P["WORLD_MATINEE_STATE"]])
        self.assertFalse([d for d in b.recv_all(0.1) if is_matinee(d)], "no live relay")
        b.enter(epoch=1)
        self.assertEqual(len([d for d in b.recv_all(0.2) if is_matinee(d)]), 1)
        a.send(P["CH_WORLD"], P["WORLD_MATINEE_STATE"], bytes([0]))
        time.sleep(0.05)
        b.enter(epoch=1)
        self.assertFalse([d for d in b.recv_all(0.2) if is_matinee(d)], "finished matinee is forgotten")

    def test_consumed_item_replays_after_its_pickup(self):
        a = self.client()
        a.send(P["CH_WORLD"], P["WORLD_PICKUP_STATE"], struct.pack("<iii", 4, 4, 4))
        a.send(P["CH_WORLD"], P["WORLD_ITEM_CONSUME"], wstr("Key"))
        time.sleep(0.05)
        b = self.client(enter=False)
        b.enter()
        types = [d[1] for d in b.recv_all(0.2) if d[0] == P["CH_WORLD"]]
        self.assertEqual(types, [P["WORLD_PICKUP_STATE"], P["WORLD_ITEM_CONSUME"]])

    def owner_of(self, client, timeout=0.3):
        last = None
        for d in client.recv_all(timeout):
            if d[:2] == bytes([P["CH_SESS"], P["SESS_SIM_OWNER"]]):
                last = struct.unpack_from("<I", d, 6)[0]
        return last

    def test_first_in_world_owns_enemy_ai(self):
        a = self.client(enter=False)
        a.enter()
        self.assertEqual(self.owner_of(a), a.player_id)
        b = self.client(enter=False)
        self.assertEqual(self.owner_of(b), a.player_id, "known before b loads a map")
        b.enter()
        self.assertEqual(self.owner_of(b), a.player_id)

    def test_enemies_only_from_owner(self):
        a, b = self.client(), self.client()
        name = b"Enemy_1"
        b.send(P["CH_ENEMY"], P["ENEMY_SPAWN"], enemy(name, b"S" * 20))
        self.assertFalse([d for d in a.recv_all(0.15) if d[0] == P["CH_ENEMY"]])
        a.send(P["CH_ENEMY"], P["ENEMY_SPAWN"], enemy(name, b"S" * 20))
        b.wait_for(lambda d: d[0] == P["CH_ENEMY"])
        ev = struct.pack("<I", a.player_id) + b"\0" * 60
        b.send(P["CH_HERO"], P["HERO_PLAYER_EVENT"], ev)
        self.assertFalse([d for d in a.recv_all(0.15) if d[:2] == bytes([P["CH_HERO"], P["HERO_PLAYER_EVENT"]])])

    def test_owner_disconnect_hands_over_and_forgets_enemies(self):
        a, b = self.client(), self.client()
        a.send(P["CH_ENEMY"], P["ENEMY_SPAWN"], enemy(b"Enemy_2", b"S" * 20))
        b.wait_for(lambda d: d[0] == P["CH_ENEMY"])
        a.send(P["CH_WORLD"], P["WORLD_DISCONNECT"])
        msgs = b.recv_all(0.3)
        kinds = [(d[0], d[1]) for d in msgs]
        owner_at = kinds.index((P["CH_SESS"], P["SESS_SIM_OWNER"]))
        self.assertEqual(struct.unpack_from("<I", msgs[owner_at], 6)[0], b.player_id)
        self.assertLess(owner_at, kinds.index((P["SRV_DISCONNECT"], 0)),
                        "the new owner must get its dummies before they are removed")
        c = self.client(enter=False)
        c.enter()
        self.assertFalse([d for d in c.recv_all(0.2) if d[0] == P["CH_ENEMY"]])

    def test_owner_death_hands_over_but_other_death_does_not(self):
        a, b = self.client(), self.client()
        b.send(P["CH_HERO"], P["HERO_PLAYER_LIFECYCLE"], bytes([0]))
        self.assertIsNone(self.owner_of(a, 0.15))
        a.send(P["CH_HERO"], P["HERO_PLAYER_LIFECYCLE"], bytes([1]))
        self.assertIsNone(self.owner_of(b, 0.15), "respawning changes nothing")
        a.send(P["CH_HERO"], P["HERO_PLAYER_LIFECYCLE"], bytes([0]))
        self.assertEqual(self.owner_of(b), b.player_id)

    def test_owner_alone_keeps_ownership_when_dying(self):
        a = self.client()
        a.send(P["CH_HERO"], P["HERO_PLAYER_LIFECYCLE"], bytes([0]))
        self.assertIsNone(self.owner_of(a, 0.15))

    def test_owner_leave_hands_over(self):
        a, b = self.client(), self.client()
        a.send(P["CH_SESS"], P["SESS_LEAVE"])
        self.assertEqual(self.owner_of(b), b.player_id)

    def test_new_world_keeps_owner(self):
        a, b = self.client(), self.client()
        b.enter(cp="CP_B", persist="MapB", epoch=1, reset=True)
        b.wait_for(sess("SESS_BASELINE_END"))
        a.recv_all(0.1)
        a.enter(cp="CP_B", persist="MapB", epoch=2)
        self.assertEqual(self.owner_of(a), a.player_id)

    def test_claim_kinds_do_not_collide(self):
        a = self.client()
        self.claim(a, P["CLAIM_CSA"], "Map.X", 1)
        self.claim(a, P["CLAIM_PICKUP"], "Map.X", 1)
        a.send(P["CH_WORLD"], P["WORLD_TRIGGER_FIRE"], bytes([1]) + wstr("Map.X"))
        got = [d[1] for d in a.recv_all(0.2) if d[0] == P["CH_WORLD"]]
        self.assertEqual(got, [P["WORLD_CLAIM"], P["WORLD_CLAIM"], P["WORLD_TRIGGER_FIRE"]])

    def test_claim_rejects_bad_packets(self):
        a = self.client()
        for body in (b"", b"\x01", b"\x01\x01", b"\x01\x01\x00", b"\x01\x01\x05ab", b"\x09\x01\x01a"):
            a.send(P["CH_WORLD"], P["WORLD_CLAIM"], body)
        self.assertFalse([d for d in a.recv_all(0.1) if d[0] == P["CH_WORLD"]])
        self.assert_alive(a)

    def test_trigger_needs_world_and_valid_packet(self):
        a = self.client(enter=False)
        self.fire(a, "Map.Touch_5", 1)
        self.assertFalse([d for d in a.recv_all(0.1) if d[0] == P["CH_WORLD"]])
        b = self.client()
        for body in (b"", b"\x01", b"\x01\x00", b"\x01\x05ab", b"\x01\xff"):
            b.send(P["CH_WORLD"], P["WORLD_TRIGGER_FIRE"], body)
        self.assert_alive(b)

    def test_respawn_releases_own_door(self):
        a, b = self.client(), self.client()
        key = struct.pack("<iii", 8, 8, 8) + b"\0" * 8
        a.send(P["CH_DOOR"], P["DOOR_LOCK"], key)
        b.wait_for(lambda d: d[:2] == bytes([P["CH_DOOR"], P["DOOR_LOCK"]]))
        a.enter(epoch=1)
        msgs = a.recv_all(0.2)
        self.assertFalse([d for d in msgs if d[:2] == bytes([P["CH_DOOR"], P["DOOR_LOCK"]])],
                         "own lock must not be replayed")
        b.send(P["CH_DOOR"], P["DOOR_LOCK"], key)
        self.assertFalse([d for d in b.recv_all(0.2) if d[0] == P["SRV_DOOR_DENY"]])

    def test_transport_unit(self):
        exe = os.path.join(self.build_dir, "test_transport")
        subprocess.check_call(["cc", "-std=c11", "-g", "-O1", "-fsanitize=address,undefined",
                               "-I", SERVER_SRC, os.path.join(HERE, "test_transport.c"),
                               os.path.join(SERVER_SRC, "ol_transport.c"), "-o", exe])
        out = subprocess.run([exe], capture_output=True, text=True, timeout=120)
        self.assertEqual(out.returncode, 0, out.stdout + out.stderr)

    def test_retransmits_until_acked(self):
        a, b = self.client(), self.client()
        a.send(P["CH_HERO"], P["HERO_SMT_TYPE"], b"\x07" * 8)
        # Read without acking: the server must send the message again.
        seen = 0
        end = time.monotonic() + 1.5
        while time.monotonic() < end and seen < 2:
            if select.select([b.sock], [], [], 0.1)[0]:
                if bytes([P["CH_HERO"], P["HERO_SMT_TYPE"]]) in b.sock.recv(4096):
                    seen += 1
        self.assertGreaterEqual(seen, 2)

    def test_address_change_keeps_session(self):
        a, b = self.client(), self.client()
        moved = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        moved.bind(("127.0.0.1", 0))
        moved.connect(("127.0.0.1", PORT))
        old, a.sock = a.sock, moved
        old.close()
        a.send(P["CH_SRV"], P["SRV_PING"], b"\x01\x02\x03\x04")
        a.wait_for(lambda d: d[0] == P["CH_SRV"] and d[1] == P["SRV_PING"])

    def test_raw_game_packet_rejected(self):
        a, b = self.client(), self.client()
        pid = struct.pack("<I", a.player_id)
        a.sock.send(bytes([P["CH_HERO"], P["HERO_HEAD_ROT"]]) + pid + b"\x11" * 8)
        self.assertFalse([d for d in b.recv_all(0.2) if d[:2] == bytes([P["CH_HERO"], P["HERO_HEAD_ROT"]])])

    def test_sender_id_cannot_be_forged(self):
        a, b = self.client(), self.client()
        msg = bytes([P["CH_HERO"], P["HERO_HEAD_ROT"]]) + struct.pack("<I", b.player_id) + b"\x11" * 8
        a.sock.send(a.header() + a.raw_message(0, msg))
        data = b.wait_for(lambda d: d[0] == P["CH_HERO"] and d[1] == P["HERO_HEAD_ROT"])
        self.assertEqual(struct.unpack_from("<I", data, 2)[0], a.player_id)

    def test_fuzz_random_datagrams(self):
        rng = random.Random(1234)
        raw = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        raw.connect(("127.0.0.1", PORT))
        a = self.client()
        for _ in range(5000):
            raw.send(rng.randbytes(rng.randint(0, 1400)))
        self.assert_alive(a)
        raw.close()

    def test_fuzz_authenticated_packets(self):
        # Valid header from a real session, so the payload reaches every parser.
        rng = random.Random(5678)
        a = self.client()
        lengths = [0, 1, 2, 6, 7, 11, 12, 13, 17, 18, 26, 27, 64, 255, 256, 513, 1100]
        for i in range(30000):
            channel = rng.randint(0, 6)
            ptype = rng.randint(0, 0x12)
            if channel == P["CH_WORLD"] and ptype == P["WORLD_DISCONNECT"]:
                continue
            if channel == P["CH_SESS"] and ptype == P["SESS_LEAVE"]:
                continue
            a.send(channel, ptype, rng.randbytes(rng.choice(lengths)))
            if i % 40 == 0:
                a.pump(0)
            if i % 7 == 0:
                # Garbage transport framing with a valid conn_id.
                n = rng.choice(lengths)
                a.sock.send(a.header() + rng.randbytes(n))
            if i % 1000 == 0:
                time.sleep(0.005)
        # Let the per-second rate window reset, otherwise the ping itself is dropped.
        time.sleep(1.1)
        a.recv_all(0.1)
        self.assert_alive(a)


if __name__ == "__main__":
    unittest.main()
