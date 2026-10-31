#!/usr/bin/env python3
"""Protocol tests for the standalone relay. Runs without the game.

Builds server/src with ASan and UBSan into a temp directory, then talks to it over
UDP. Requires cmake, ninja and a C/C++ compiler.

    python3 Tests/test_relay.py [-v]
"""
import atexit
import ctypes
import hashlib
import json
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
HS_VERSION = 1
SIM_SECRET = "test simulator secret"


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


class Crypto:
    """The vendored Monocypher through ctypes, so the tests need no Python packages."""

    def __init__(self):
        self.dir = tempfile.mkdtemp(prefix="ol_crypto_test_")
        lib = os.path.join(self.dir, "mono.so")
        subprocess.check_call(["cc", "-O1", "-shared", "-fPIC", "-o", lib,
                               os.path.join(SERVER_SRC, "third_party", "monocypher", "monocypher.c")])
        self.lib = ctypes.CDLL(lib)

    def x25519_public(self, priv):
        out = ctypes.create_string_buffer(32)
        self.lib.crypto_x25519_public_key(out, priv)
        return out.raw

    def x25519(self, priv, pub):
        out = ctypes.create_string_buffer(32)
        self.lib.crypto_x25519(out, priv, pub)
        return out.raw

    def argon2id(self, password, salt):
        class Config(ctypes.Structure):
            _fields_ = [(n, ctypes.c_uint32) for n in ("algorithm", "nb_blocks", "nb_passes", "nb_lanes")]

        class Inputs(ctypes.Structure):
            _fields_ = [("password", ctypes.c_char_p), ("salt", ctypes.c_char_p),
                        ("password_size", ctypes.c_uint32), ("salt_size", ctypes.c_uint32)]

        class Extras(ctypes.Structure):
            _fields_ = [("key", ctypes.c_void_p), ("ad", ctypes.c_void_p),
                        ("key_size", ctypes.c_uint32), ("ad_size", ctypes.c_uint32)]

        out, work = ctypes.create_string_buffer(32), ctypes.create_string_buffer(16384 * 1024)
        self.lib.crypto_argon2(out, ctypes.c_uint32(32), work, Config(2, 16384, 3, 1),
                               Inputs(password, salt, len(password), len(salt)), Extras())
        return out.raw

    def aead_lock(self, key, nonce, ad, plain):
        mac, out = ctypes.create_string_buffer(16), ctypes.create_string_buffer(max(len(plain), 1))
        self.lib.crypto_aead_lock(out, mac, key, nonce, ad, ctypes.c_size_t(len(ad)), plain, ctypes.c_size_t(len(plain)))
        return mac.raw, out.raw[:len(plain)]

    def aead_unlock(self, key, nonce, ad, mac, sealed):
        out = ctypes.create_string_buffer(max(len(sealed), 1))
        bad = self.lib.crypto_aead_unlock(out, mac, key, nonce, ad, ctypes.c_size_t(len(ad)), sealed, ctypes.c_size_t(len(sealed)))
        return None if bad else out.raw[:len(sealed)]


crypto = Crypto()
atexit.register(shutil.rmtree, crypto.dir, True)


class Client:
    """Minimal client of the ol_transport protocol over a lossless loopback."""

    def __init__(self, room=ROOM, password="", role=0, port=None, token=None):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(("127.0.0.1", 0))
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 1 << 22)
        self.sock.connect(("127.0.0.1", port or PORT))
        self.inbox = []
        self.conn_id = 0
        self.seq = 0
        self.out_id = 0
        self.in_next = 0
        self.in_buf = {}
        self.send_counter = 0
        self.recv_counter = -1
        self.room, self.password, self.role = room, password, role
        self.sent = []
        ready = self.handshake(token)
        if ready is None or ready[0] != P["SRV_READY"]:
            raise AssertionError("refused: %r" % (ready[6:] if ready else "no answer"))
        self.player_id = struct.unpack_from("<I", ready, 5)[0]
        name_len = ready[9]
        self.token = ready[10 + name_len:10 + name_len + 32]
        self.conn_id = struct.unpack_from("<Q", ready, 10 + name_len + 32)[0]

    def hello_packet(self):
        return bytes([P["OL_HS_HELLO"], HS_VERSION]) + bytes(62)

    def auth_packet(self, challenge, token=None):
        """Builds AUTH from a challenge; independent of the C implementation of the format."""
        server_pub, cookie = challenge[2:34], challenge[34:50]
        priv = os.urandom(32)
        pub = crypto.x25519_public(priv)
        root = hashlib.blake2b(crypto.x25519(priv, server_pub) + pub + server_pub, digest_size=32).digest()
        derive = lambda label: hashlib.blake2b(label, key=root, digest_size=32).digest()
        self.result_key = derive(b"openol result")
        self.hs_priv, self.hs_root, self.server_pub = priv, root, server_pub
        if self.role == 1 or not self.password:
            secret_key = hashlib.blake2b(self.password.encode(), digest_size=32).digest()
        else:
            secret_key = crypto.argon2id(self.password.encode(), server_pub)
        proof = hashlib.blake2b(root + cookie, key=secret_key, digest_size=32).digest()
        plain = (bytes([self.role]) + proof + bytes([1 if token else 0]) + (token or bytes(32))
                 + bytes([len(self.room)]) + self.room.encode())
        head = bytes([P["OL_HS_AUTH"], HS_VERSION]) + pub + cookie
        mac, sealed = crypto.aead_lock(derive(b"openol auth"), bytes(24), head, plain)
        return head + mac + sealed

    def open_result(self, data):
        msg = crypto.aead_unlock(self.result_key, bytes(24), data[:34], data[34:50], data[50:])
        session_pub = data[2:34]
        if msg is not None and session_pub != bytes(32):
            key = hashlib.blake2b(self.hs_root + crypto.x25519(self.hs_priv, session_pub), digest_size=32).digest()
            self.c2s = hashlib.blake2b(b"openol c2s", key=key, digest_size=32).digest()
            self.s2c = hashlib.blake2b(b"openol s2c", key=key, digest_size=32).digest()
        return msg

    def next_of_type(self, first_byte):
        """Next raw datagram starting with first_byte, skipping session traffic."""
        end = time.monotonic() + 2.0
        while time.monotonic() < end:
            if select.select([self.sock], [], [], end - time.monotonic())[0]:
                data = self.sock.recv(4096)
                if data[0] == first_byte:
                    return data
        raise AssertionError("no datagram of type %#x" % first_byte)

    def fresh_challenge(self):
        self.sock.send(self.hello_packet())
        return self.next_of_type(P["OL_HS_CHALLENGE"])

    def handshake(self, token=None):
        """Returns the opened HS_RESULT, or None if the server stays silent."""
        self.sock.send(self.hello_packet())
        end = time.monotonic() + 2.0
        while time.monotonic() < end:
            if not select.select([self.sock], [], [], end - time.monotonic())[0]:
                break
            data = self.sock.recv(4096)
            if data[0] == P["OL_HS_CHALLENGE"]:
                self.sock.send(self.auth_packet(data, token))
            elif data[0] == P["OL_HS_RESULT"]:
                return self.open_result(data)
        return None

    def enter(self, cp="CP_A", persist="MapA", epoch=0, reset=False):
        body = struct.pack("<HB", epoch, 1 if reset else 0) + wstr(cp) + wstr(persist)
        self.send(P["CH_SESS"], P["SESS_ENTER"], body)

    def header(self):
        self.seq = (self.seq + 1) & 0xFFFF
        return struct.pack("<HHI", self.seq, self.in_next, 0)

    def seal(self, inner):
        """A complete datagram around header() + messages."""
        head = struct.pack("<BQI", MAGIC, self.conn_id, self.send_counter)
        nonce = struct.pack("<I", self.send_counter) + bytes(20)
        self.send_counter += 1
        mac, sealed = crypto.aead_lock(self.c2s, nonce, head, inner)
        return head + mac + sealed

    def send_inner(self, inner):
        self.sock.send(self.seal(inner))

    def raw_message(self, lane, msg):
        if lane == 0:
            return bytes([0]) + struct.pack("<H", len(msg)) + msg
        body = bytes([1]) + struct.pack("<HH", self.out_id, len(msg)) + msg
        self.out_id = (self.out_id + 1) & 0xFFFF
        return body

    def send(self, channel, ptype, payload=b""):
        msg = bytes([channel, ptype]) + struct.pack("<I", self.player_id) + payload
        self.send_inner(self.header() + self.raw_message(lane_for(channel, ptype), msg))

    def open_datagram(self, data):
        counter = struct.unpack_from("<I", data, 9)[0]
        nonce = struct.pack("<I", counter) + bytes(20)
        inner = crypto.aead_unlock(self.s2c, nonce, data[:13], data[13:29], data[29:])
        if inner is None:
            raise AssertionError("server sent a datagram that does not authenticate")
        return inner

    def on_datagram(self, data):
        if data[0] != MAGIC or len(data) < 37:
            return
        data = bytes(29) + self.open_datagram(data)
        off, acked = 37, False
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
            self.send_inner(self.header())

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
        for name in ("relay.db", "relay.db.key"):
            path = os.path.join(self.build_dir, name)
            if os.path.exists(path):
                os.remove(path)
        self.log = tempfile.TemporaryFile()
        self.server = subprocess.Popen([self.exe, "--port", str(PORT)], cwd=self.build_dir,
                                       env=dict(os.environ, OPENOL_SIM_SECRET=SIM_SECRET),
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

    def client(self, enter=True, **kwargs):
        c = Client(**kwargs)
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
        a.send(P["CH_ENEMY"], P["ENEMY_LOC"], enemy(name, b"\0" + bytes(12) + b"L" * 28))
        a.send(P["CH_ENEMY"], P["ENEMY_DOOR_BREAK"], enemy(name, b"B" * 13))
        time.sleep(0.1)
        b = self.client(enter=False)
        b.enter()
        got = [d for d in b.recv_all(0.5) if d[0] == P["CH_ENEMY"]]
        self.assertEqual([d[1] for d in got], [P["ENEMY_SPAWN"], P["ENEMY_LOC"]])
        self.assertIn(b"L" * 28, got[1])

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

    def test_enemy_anim_is_relayed_and_snapshotted(self):
        a, b = self.client(), self.client()
        name = b"Enemy_3"
        is_anim = lambda d: d[:2] == bytes([P["CH_ENEMY"], P["ENEMY_ANIM"]])
        a.send(P["CH_ENEMY"], P["ENEMY_SPAWN"], enemy(name, b"S" * 20))
        play = bytes([0]) + wstr("FullBodySlot") + wstr("Pkg.Set") + wstr("Attack") + struct.pack("<ff", 1.5, 1.0)
        a.send(P["CH_ENEMY"], P["ENEMY_ANIM"], enemy(name, play))
        b.wait_for(is_anim)
        time.sleep(0.05)
        c = self.client(enter=False)
        c.enter()
        got = [d for d in c.recv_all(0.2) if d[0] == P["CH_ENEMY"]]
        self.assertEqual([d[1] for d in got], [P["ENEMY_SPAWN"], P["ENEMY_ANIM"]], "a joiner sees the animation in progress")
        stop = bytes([P["ENEMY_ANIM_STOP"]]) + wstr("FullBodySlot")
        a.send(P["CH_ENEMY"], P["ENEMY_ANIM"], enemy(name, stop))
        b.wait_for(is_anim)
        c.recv_all(0.1)  # c is in the world too and hears the stop live
        c.enter(epoch=1)
        got = [d for d in c.recv_all(0.2) if d[0] == P["CH_ENEMY"]]
        self.assertEqual([d[1] for d in got], [P["ENEMY_SPAWN"]], "a stopped animation is forgotten")

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
                               os.path.join(SERVER_SRC, "ol_transport.c"),
                               os.path.join(SERVER_SRC, "third_party", "monocypher", "monocypher.c"), "-o", exe])
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
                if bytes([P["CH_HERO"], P["HERO_SMT_TYPE"]]) in b.open_datagram(b.sock.recv(4096)):
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
        a.send_inner(a.header() + a.raw_message(0, msg))
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

    # --- Handshake and session security ---

    def stop_server(self):
        self.server.terminate()
        self.server.wait()

    def start_server(self):
        self.server = subprocess.Popen([self.exe, "--port", str(PORT)], cwd=self.build_dir,
                                       env=dict(os.environ, OPENOL_SIM_SECRET=SIM_SECRET),
                                       stdin=subprocess.DEVNULL, stdout=self.log, stderr=subprocess.STDOUT)
        # Answers only after startup, which may include hashing passwords of an old database.
        probe = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        probe.connect(("127.0.0.1", PORT))
        for _ in range(100):
            probe.send(bytes([P["OL_HS_HELLO"], HS_VERSION]) + bytes(62))
            try:
                if select.select([probe], [], [], 0.2)[0] and probe.recv(4096):
                    break
            except ConnectionRefusedError:
                time.sleep(0.1)
        probe.close()

    def db_text(self):
        with open(os.path.join(self.build_dir, "relay.db")) as f:
            return f.read()

    def test_server_key_survives_restart(self):
        first = self.client(enter=False)
        key = first.server_pub
        self.stop_server()
        self.start_server()
        again = self.client(enter=False)
        self.assertEqual(again.server_pub, key)
        key_file = os.path.join(self.build_dir, "relay.db.key")
        self.assertEqual(os.stat(key_file).st_mode & 0o077, 0, "only the owner may read the key")

    def test_plain_password_in_old_database_is_hashed(self):
        self.restart_with_room("SAFE", "hunter2")
        self.assertNotIn("hunter2", self.db_text())
        self.assertIn("$ol1$", self.db_text())
        self.assertTrue(self.client(enter=False, room="SAFE", password="hunter2").player_id)
        self.stop_server()
        self.start_server()
        self.assertTrue(self.client(enter=False, room="SAFE", password="hunter2").player_id)
        with self.assertRaises(AssertionError):
            self.clients.append(Client(room="SAFE", password="hunter3"))

    def test_stored_hash_cannot_be_typed_as_password(self):
        self.restart_with_room("SAFE", "hunter2")
        stored = json.loads(self.db_text())["rooms"][0]["password"]
        with self.assertRaises(AssertionError):
            self.clients.append(Client(room="SAFE", password=stored))

    def test_session_keys_do_not_come_from_the_server_key_alone(self):
        # Forward secrecy: the same client key gets different session keys each time.
        results = []
        for _ in range(2):
            d = self.client(enter=False)
            d.sock.send(d.auth_packet(d.fresh_challenge()))
            results.append(d.next_of_type(P["OL_HS_RESULT"])[2:34])
        self.assertNotEqual(results[0], results[1])
        self.assertNotEqual(results[0], bytes(32))

    def test_refusal_has_no_session_key(self):
        self.restart_with_room("SAFE", "hunter2")
        c = self.client(enter=False, room="SAFE", password="hunter2")
        c.password = "wrong"
        c.sock.send(c.auth_packet(c.fresh_challenge()))
        self.assertEqual(c.next_of_type(P["OL_HS_RESULT"])[2:34], bytes(32))

    def restart_with_room(self, code, password):
        self.stop_server()
        with open(os.path.join(self.build_dir, "relay.db"), "w") as f:
            json.dump({"rooms": [{"code": code, "password": password, "trusted": [], "bans": []}], "bans": []}, f)
        self.start_server()

    def test_room_password_is_proved_not_sent(self):
        self.restart_with_room("SAFE", "hunter2")
        with self.assertRaises(AssertionError):
            self.clients.append(Client(room="SAFE", password="wrong"))
        good = self.client(enter=False, room="SAFE", password="hunter2")
        self.assertTrue(good.player_id)
        challenge = bytes([P["OL_HS_CHALLENGE"], HS_VERSION]) + bytes(48)
        self.assertNotIn(b"hunter2", good.auth_packet(challenge))
        with open(os.path.join(self.build_dir, "relay.db")) as f:
            self.assertNotIn("hunter2", f.read())

    def test_password_is_required_again_from_the_same_address(self):
        # No trust by IP: everyone arriving through the Steam bridge shares 127.0.0.1.
        self.restart_with_room("SAFE", "hunter2")
        self.client(enter=False, room="SAFE", password="hunter2")
        with self.assertRaises(AssertionError):
            self.clients.append(Client(room="SAFE", password=""))

    def test_challenge_cannot_amplify(self):
        hello = bytes([P["OL_HS_HELLO"], HS_VERSION]) + bytes(62)
        raw = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        raw.connect(("127.0.0.1", PORT))
        raw.send(hello[:20])
        self.assertFalse(select.select([raw], [], [], 0.2)[0], "short hello must be ignored")
        raw.send(hello)
        self.assertTrue(select.select([raw], [], [], 1.0)[0])
        self.assertLessEqual(len(raw.recv(4096)), len(hello))
        raw.close()

    def test_auth_needs_cookie_of_this_address(self):
        victim = self.client(enter=False)
        challenge = victim.fresh_challenge()
        spoofer = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        spoofer.connect(("127.0.0.1", PORT))
        spoofer.send(victim.auth_packet(challenge))
        self.assertFalse(select.select([spoofer], [], [], 0.3)[0])
        spoofer.close()

    def test_repeated_auth_gets_the_same_session(self):
        c = self.client(enter=False)
        auth = c.auth_packet(c.fresh_challenge())
        answers = []
        for _ in range(2):
            c.sock.send(auth)
            data = c.next_of_type(P["OL_HS_RESULT"])
            answers.append(c.open_result(data))
        self.assertEqual(answers[0], answers[1])

    def test_token_resumes_session_only_with_the_right_token(self):
        a = self.client()
        resumed = self.client(enter=False, token=a.token)
        self.assertEqual(resumed.player_id, a.player_id)
        other = self.client(enter=False, token=bytes(32))
        self.assertNotEqual(other.player_id, a.player_id)

    def test_tampered_datagram_is_ignored(self):
        a, b = self.client(), self.client()
        b.recv_all(0.1)
        msg = bytes([P["CH_HERO"], P["HERO_HEAD_ROT"]]) + struct.pack("<I", a.player_id) + b"\x11" * 8
        packet = bytearray(a.seal(a.header() + a.raw_message(0, msg)))
        packet[-1] ^= 1
        a.sock.send(bytes(packet))
        self.assertFalse([d for d in b.recv_all(0.2) if d[:2] == msg[:2]])

    def test_replayed_datagram_is_ignored(self):
        a, b = self.client(), self.client()
        b.recv_all(0.1)
        chat = struct.pack("<I", a.player_id) + b"\x11" * 8
        packet = a.seal(a.header() + a.raw_message(0, bytes([P["CH_HERO"], P["HERO_HEAD_ROT"]]) + chat))
        a.sock.send(packet)
        self.assertEqual(len([d for d in b.recv_all(0.2) if d[1] == P["HERO_HEAD_ROT"]]), 1)
        a.sock.send(packet)
        self.assertFalse([d for d in b.recv_all(0.2) if d[1] == P["HERO_HEAD_ROT"]])

    def test_stranger_cannot_redirect_a_session(self):
        a = self.client()
        stranger = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        stranger.connect(("127.0.0.1", PORT))
        # Right conn_id and a copy of a real datagram, from another address.
        stranger.send(a.seal(a.header()))
        stranger.send(struct.pack("<BQI", MAGIC, a.conn_id, 999) + bytes(40))
        time.sleep(0.1)
        self.assert_alive(a)
        self.assertFalse(select.select([stranger], [], [], 0.2)[0])
        stranger.close()

    def test_simulator_needs_the_server_secret(self):
        with self.assertRaises(AssertionError):
            self.clients.append(Client(password="", role=1))
        with self.assertRaises(AssertionError):
            self.clients.append(Client(password="guess", role=1))
        self.assertTrue(self.sim_node().player_id)

    def test_only_one_simulator_per_room(self):
        self.sim_node()
        with self.assertRaises(AssertionError):
            self.clients.append(Client(password=SIM_SECRET, role=1))

    def test_player_cannot_become_simulator_through_session_channel(self):
        a = self.client()
        a.send(P["CH_SESS"], 0x09)  # the old SESS_SIM_HELLO
        b = self.client()
        b.send(P["CH_ENEMY"], P["ENEMY_SPAWN"], enemy(b"Forged"))
        self.assertFalse([d for d in a.recv_all(0.2) if d[0] == P["CH_ENEMY"]])

    # --- Checks on what clients claim ---

    @staticmethod
    def hero_state(x=0.0, y=0.0, z=0.0, smt=0, health=100):
        body = struct.pack("<fffHHhhh", x, y, z, 0, 0, 0, 0, 0) + bytes([0, 0, smt, 0]) + struct.pack("<hhhh", 0, 0, 0, health)
        return body + bytes(63 - len(body)) + b"\x00\x00\x01T"

    @staticmethod
    def is_state(d):
        return d[0] == P["CH_HERO"] and d[1] == P["HERO_STATE"]

    def test_hero_state_with_harmful_values_is_not_relayed(self):
        a, b = self.client(), self.client()
        nan = float("nan")
        for payload in (self.hero_state(x=nan), self.hero_state(z=float("inf")), self.hero_state(y=1e9),
                        self.hero_state(smt=200), self.hero_state(health=30000), self.hero_state()[:20]):
            a.send(P["CH_HERO"], P["HERO_STATE"], payload)
        self.assertFalse([d for d in b.recv_all(0.2) if self.is_state(d)])
        a.send(P["CH_HERO"], P["HERO_STATE"], self.hero_state())
        b.wait_for(self.is_state)

    def test_implausible_movement_is_logged_but_relayed(self):
        # Lag looks like this too, so nobody gets stopped for it.
        a, b = self.client(), self.client()
        a.send(P["CH_HERO"], P["HERO_STATE"], self.hero_state())
        b.wait_for(self.is_state)
        a.send(P["CH_HERO"], P["HERO_STATE"], self.hero_state(x=50000.0))
        b.wait_for(self.is_state)
        self.log.flush()
        self.log.seek(0)
        self.assertIn(b"moved faster than possible", self.log.read())

    def test_special_move_and_new_spawn_may_teleport(self):
        a, b = self.client(), self.client()
        a.send(P["CH_HERO"], P["HERO_STATE"], self.hero_state())
        b.wait_for(self.is_state)
        a.send(P["CH_HERO"], P["HERO_STATE"], self.hero_state(x=50000.0, smt=33))
        b.wait_for(self.is_state)
        a.enter()
        a.wait_for(sess("SESS_BASELINE_END"))
        a.send(P["CH_HERO"], P["HERO_STATE"], self.hero_state(x=-50000.0))
        b.wait_for(self.is_state)
        self.log.flush()
        self.log.seek(0)
        self.assertNotIn(b"moved faster than possible", self.log.read())

    @staticmethod
    def is_event(d):
        return d[0] == P["CH_HERO"] and d[1] == P["HERO_PLAYER_EVENT"]

    def owner_and_victim(self):
        owner = self.client()
        victim = self.client()
        victim.send(P["CH_HERO"], P["HERO_STATE"], self.hero_state())
        owner.wait_for(self.is_state)
        return owner, victim

    @staticmethod
    def hit(target):
        return struct.pack("<I", target) + bytes(30)

    @staticmethod
    def enemy_loc(x=0.0, y=0.0, z=0.0):
        return enemy(b"Bot", b"\x00" + struct.pack("<fff", x, y, z) + bytes(10))

    def enemy_at(self, owner, victim, x=0.0):
        owner.send(P["CH_ENEMY"], P["ENEMY_SPAWN"], enemy(b"Bot"))
        owner.send(P["CH_ENEMY"], P["ENEMY_LOC"], self.enemy_loc(x=x))
        victim.wait_for(lambda d: d[0] == P["CH_ENEMY"] and d[1] == P["ENEMY_LOC"])

    def test_player_owner_cannot_hit_with_no_enemy_nearby(self):
        owner, victim = self.owner_and_victim()
        owner.send(P["CH_HERO"], P["HERO_PLAYER_EVENT"], self.hit(victim.player_id))
        self.assertFalse([d for d in victim.recv_all(0.2) if self.is_event(d)])
        self.enemy_at(owner, victim, x=100.0)
        owner.send(P["CH_HERO"], P["HERO_PLAYER_EVENT"], self.hit(victim.player_id))
        victim.wait_for(self.is_event)

    def test_event_for_unknown_target_is_dropped(self):
        owner, victim = self.owner_and_victim()
        self.enemy_at(owner, victim)
        owner.send(P["CH_HERO"], P["HERO_PLAYER_EVENT"], self.hit(424242))
        self.assertFalse([d for d in victim.recv_all(0.2) if self.is_event(d)])

    def test_player_events_are_rate_limited(self):
        owner, victim = self.owner_and_victim()
        self.enemy_at(owner, victim)
        for _ in range(30):
            owner.send(P["CH_HERO"], P["HERO_PLAYER_EVENT"], self.hit(victim.player_id))
        self.assertEqual(len([d for d in victim.recv_all(0.3) if self.is_event(d)]), 8)

    def test_enemy_position_with_nan_is_not_relayed(self):
        owner, victim = self.owner_and_victim()
        victim.recv_all(0.1)
        owner.send(P["CH_ENEMY"], P["ENEMY_SPAWN"], enemy(b"Bot"))
        victim.wait_for(lambda d: d[0] == P["CH_ENEMY"])
        owner.send(P["CH_ENEMY"], P["ENEMY_LOC"], self.enemy_loc(x=float("nan")))
        self.assertFalse([d for d in victim.recv_all(0.2) if d[0] == P["CH_ENEMY"]])

    # --- Headless simulator ---

    def sim_node(self):
        return self.client(enter=False, password=SIM_SECRET, role=1)

    @staticmethod
    def owner_is(player_id):
        return lambda d: (d[0] == P["CH_SESS"] and d[1] == P["SESS_SIM_OWNER"]
                          and struct.unpack_from("<I", d, 6)[0] == player_id)

    def test_sim_waits_for_a_world_then_owns_it(self):
        node = self.sim_node()
        node.enter()  # no world yet: must not create one
        time.sleep(0.1)
        a = self.client(enter=False)
        a.enter()
        travel = node.wait_for(sess("SESS_TRAVEL"))
        self.assertEqual(world_msg(travel)[1:], ("CP_A", "MapA"))
        a.wait_for(self.owner_is(node.player_id))
        node.enter(epoch=world_msg(travel)[0])
        node.wait_for(sess("SESS_BASELINE_END"))

    def test_sim_is_sent_to_running_world(self):
        a = self.client()
        node = self.sim_node()
        self.assertEqual(world_msg(node.wait_for(sess("SESS_TRAVEL")))[1:], ("CP_A", "MapA"))
        # Passive until the owner goes away: taking over now would restart every enemy.
        node.enter(epoch=1)
        node.wait_for(sess("SESS_BASELINE_END"))
        self.assertFalse([d for d in a.recv_all(0.2) if self.owner_is(node.player_id)(d)])

    def test_sim_never_changes_the_world(self):
        a = self.client()
        node = self.sim_node()
        epoch = world_msg(node.wait_for(sess("SESS_TRAVEL")))[0]
        node.enter(cp="CP_B", persist="MapB", epoch=0, reset=True)
        # Refused and sent back, while the player hears nothing.
        self.assertEqual(world_msg(node.wait_for(sess("SESS_TRAVEL")))[:1], (epoch,))
        node.enter(epoch=epoch)
        node.wait_for(sess("SESS_BASELINE_END"))
        node.send(P["CH_SESS"], P["SESS_CHECKPOINT"], wstr("CP_Z") + wstr("MapA"))
        time.sleep(0.1)
        b = self.client(enter=False)
        b.enter()
        self.assertEqual(world_msg(b.wait_for(sess("SESS_WORLD")))[1], "CP_A")
        self.assert_alive(a)

    def test_sim_does_not_count_as_player_in_world(self):
        a = self.client()
        node = self.sim_node()
        epoch = world_msg(node.wait_for(sess("SESS_TRAVEL")))[0]
        node.enter(epoch=epoch)
        node.wait_for(sess("SESS_BASELINE_END"))
        a.send(P["CH_SESS"], P["SESS_LEAVE"])
        time.sleep(0.1)
        # Alone apart from the simulator: this player moves the world, and the simulator follows.
        a.enter(cp="CP_B", persist="MapB")
        self.assertEqual(world_msg(a.wait_for(sess("SESS_WORLD")))[1:], ("CP_B", "MapB"))
        self.assertEqual(world_msg(node.wait_for(sess("SESS_TRAVEL")))[1:], ("CP_B", "MapB"))

    def test_sim_is_invisible_to_players(self):
        a = self.client()
        node = self.sim_node()
        epoch = world_msg(node.wait_for(sess("SESS_TRAVEL")))[0]
        node.enter(epoch=epoch)
        node.wait_for(sess("SESS_BASELINE_END"))
        a.recv_all(0.1)
        node.send(P["CH_HERO"], P["HERO_STATE"], b"\0" * 40)
        node.send(P["CH_HERO"], P["HERO_NICK"], b"\x03Sim")
        node.send(P["CH_DOOR"], P["DOOR_STATE"], b"\0" * 20)
        node.send(P["CH_SESS"], P["SESS_LEAVE"])
        got = a.recv_all(0.2)
        for d in got:
            self.assertNotEqual(struct.unpack_from("<I", d, 2)[0], node.player_id, d[:8])
            self.assertFalse(sess("SESS_PLAYER_LEFT")(d))

    def test_sim_enemies_and_player_events_reach_players(self):
        a = self.client(enter=False)
        node = self.sim_node()
        a.enter()
        epoch = world_msg(node.wait_for(sess("SESS_TRAVEL")))[0]
        node.enter(epoch=epoch)
        node.wait_for(sess("SESS_BASELINE_END"))
        node.send(P["CH_ENEMY"], P["ENEMY_SPAWN"], enemy(b"Bot1"))
        a.wait_for(lambda d: d[0] == P["CH_ENEMY"] and d[1] == P["ENEMY_SPAWN"])
        node.send(P["CH_HERO"], P["HERO_PLAYER_EVENT"], struct.pack("<I", a.player_id) + b"\x01" * 5)
        a.wait_for(lambda d: d[0] == P["CH_HERO"] and d[1] == P["HERO_PLAYER_EVENT"])

    def test_sim_not_counted_online(self):
        a = self.client()
        a.recv_all(0.1)
        self.sim_node()
        time.sleep(0.1)
        # It counts only until it identifies itself, which happens right after connecting.
        counts = [d for d in a.recv_all(0.1) if d[0] == P["SRV_ONLINE_COUNT"]]
        self.assertEqual(struct.unpack_from("<I", counts[-1], 5)[0], 1)

    def test_owner_falls_back_to_player_when_sim_disconnects(self):
        a = self.client(enter=False)
        node = self.sim_node()
        a.enter()
        epoch = world_msg(node.wait_for(sess("SESS_TRAVEL")))[0]
        node.enter(epoch=epoch)
        node.wait_for(sess("SESS_BASELINE_END"))
        a.wait_for(self.owner_is(node.player_id))
        node.send(P["CH_WORLD"], P["WORLD_DISCONNECT"])
        a.wait_for(self.owner_is(a.player_id))

    def test_sim_takes_over_from_dying_player(self):
        a = self.client()
        b = self.client()
        node = self.sim_node()
        epoch = world_msg(node.wait_for(sess("SESS_TRAVEL")))[0]
        node.enter(epoch=epoch)
        node.wait_for(sess("SESS_BASELINE_END"))
        a.send(P["CH_HERO"], P["HERO_PLAYER_LIFECYCLE"], b"\0")
        b.wait_for(self.owner_is(node.player_id))

    def test_room_without_players_drops_world_for_sim(self):
        node = self.sim_node()
        a = self.client(enter=False)
        a.enter()
        first = world_msg(node.wait_for(sess("SESS_TRAVEL")))[0]
        node.enter(epoch=first)
        node.wait_for(sess("SESS_BASELINE_END"))
        a.send(P["CH_WORLD"], P["WORLD_DISCONNECT"])
        time.sleep(0.2)
        b = self.client(enter=False)
        b.enter()
        # Same map, yet a fresh world: nobody is left to preserve the old one for.
        self.assertGreater(world_msg(node.wait_for(sess("SESS_TRAVEL")))[0], first)

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
                # Garbage transport framing inside an authentic datagram.
                n = rng.choice(lengths)
                a.send_inner(a.header() + rng.randbytes(n))
            if i % 1000 == 0:
                time.sleep(0.005)
        # Let the per-second rate window reset, otherwise the ping itself is dropped.
        time.sleep(1.1)
        a.recv_all(0.1)
        self.assert_alive(a)


if __name__ == "__main__":
    unittest.main()
