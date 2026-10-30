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
ROOM = "PUBLIC"


def load_protocol():
    ids = {}
    with open(os.path.join(SERVER_SRC, "openol_protocol.h"), encoding="utf-8") as f:
        for name, value in re.findall(r"^#define (\w+)\s+(0x[0-9A-Fa-f]+|\d+)", f.read(), re.M):
            ids[name] = int(value, 0)
    return ids


P = load_protocol()


class Client:
    def __init__(self, room=ROOM):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(("127.0.0.1", 0))
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 1 << 20)
        self.sock.connect(("127.0.0.1", PORT))
        self.sock.send(("HELLO,%s,\n" % room).encode())
        ready = self.wait_for(lambda d: d[0] == P["SRV_READY"])
        self.player_id = struct.unpack_from("<I", ready, 5)[0]
        name_len = ready[9]
        self.token = ready[10 + name_len:10 + name_len + 32]

    def send(self, channel, ptype, payload=b""):
        self.sock.send(bytes([channel, ptype]) + struct.pack("<I", self.player_id) + payload)

    def recv_all(self, duration):
        packets, end = [], time.monotonic() + duration
        while True:
            left = end - time.monotonic()
            if left <= 0:
                return packets
            if select.select([self.sock], [], [], left)[0]:
                packets.append(self.sock.recv(4096))

    def wait_for(self, match, timeout=2.0):
        end = time.monotonic() + timeout
        while time.monotonic() < end:
            if select.select([self.sock], [], [], end - time.monotonic())[0]:
                data = self.sock.recv(4096)
                if match(data):
                    return data
        raise AssertionError("expected packet did not arrive")

    def close(self):
        self.sock.close()


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

    def client(self):
        c = Client()
        self.clients.append(c)
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
        b = self.client()
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
        b = self.client()
        start = time.monotonic()
        arrivals = []
        end = start + 3.0
        while time.monotonic() < end and len(arrivals) < count:
            # Keep the server's main loop busy: this used to speed up the drip.
            a.send(P["CH_SRV"], P["SRV_PING"], b"\0\0\0\0")
            if select.select([b.sock], [], [], 0.001)[0]:
                d = b.sock.recv(4096)
                if d[0] == P["CH_WORLD"] and d[1] == P["WORLD_PICKUP_STATE"]:
                    arrivals.append(time.monotonic())
        self.assertEqual(len(arrivals), count, "snapshots were lost")
        rate = count / max(arrivals[-1] - arrivals[0], 1e-3)
        self.assertLess(rate, 1200, "drip rate must stay near 800 packets/s under load")

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
        pid = struct.pack("<I", a.player_id)
        lengths = [0, 1, 2, 6, 7, 11, 12, 13, 17, 18, 26, 27, 64, 255, 256, 513, 1400, 4090]
        for i in range(30000):
            channel = rng.randint(0, 5)
            ptype = rng.randint(0, 0x12)
            if channel == P["CH_WORLD"] and ptype == P["WORLD_DISCONNECT"]:
                continue
            a.sock.send(bytes([channel, ptype]) + pid + rng.randbytes(rng.choice(lengths)))
            if i % 1000 == 0:
                time.sleep(0.005)
        # Let the per-second rate window reset, otherwise the ping itself is dropped.
        time.sleep(1.1)
        a.recv_all(0.1)
        self.assert_alive(a)


if __name__ == "__main__":
    unittest.main()
