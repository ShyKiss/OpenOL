#!/usr/bin/env python3
"""End to end: standalone relay, a real -simnode process and a scripted player.

Needs the game install and the built OLGame.exe (no display or GPU), takes 3 to 4 minutes.

    python3 Tests/test_simnode.py
"""
import os
import shutil
import struct
import subprocess
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import test_relay as t

P = t.P
ROOT = os.path.abspath(os.path.join(HERE, "..", "..", "..", ".."))
OUTLAST = os.environ.get("OUTLAST_DIR", "/mnt/ntfs2/SteamAPP/steamapps/common/Outlast")
EXE = "OLGame_simtest.exe"
LOG = OUTLAST + "/OLGame/Logs/simtest.log"

PRISON = ("Prison_IsolationCells02_Soldier", "prison_persistent")
ADMIN = ("Admin_MainHall", "adminblock_persistent")
# A one-shot trigger near the prison checkpoint whose Kismet spawns a soldier.
SOLDIER_TRIGGER = b"prison_01-ld.TheWorld:PersistentLevel.Main_Sequence.SeqEvent_Touch_10"
NEAR_TRIGGER = (3471.409, -1569.709, 424.491)


def hero_state(x, y, z):
    body = struct.pack("<fffHHhhh", x, y, z, 0, 0, 0, 0, 0) + bytes(4) + struct.pack("<hhhh", 0, 0, 0, 100)
    return body + bytes(63 - len(body)) + b"\x00\x00\x04Test"


class Session:
    def __init__(self):
        self.player = t.Client()
        self.player.send(P["CH_SRV"], P["SRV_HELLO"], b"\x04Test")
        self.spawned = []
        self.hits = 0
        self.owner = None

    def run(self, seconds, at=NEAR_TRIGGER):
        self.run_until(lambda: False, seconds, at)

    def run_until(self, done, seconds, at=NEAR_TRIGGER):
        end = time.monotonic() + seconds
        next_ping = 0
        while time.monotonic() < end and not done():
            self.player.send(P["CH_HERO"], P["HERO_STATE"], hero_state(*at))
            if time.monotonic() > next_ping:
                next_ping = time.monotonic() + 2
                self.player.send(P["CH_SRV"], P["SRV_PING"], b"\x01\x02\x03\x04")
            for d in self.player.recv_all(0.05):
                if d[0] == P["CH_ENEMY"] and d[1] == P["ENEMY_SPAWN"]:
                    self.spawned.append(d[7:7 + d[6]].decode())
                elif d[0] == P["CH_HERO"] and d[1] == P["HERO_PLAYER_EVENT"]:
                    self.hits += 1
                elif d[0] == P["CH_SESS"] and d[1] == P["SESS_SIM_OWNER"]:
                    self.owner = struct.unpack_from("<I", d, 6)[0]

    def enter(self, world, reset=False):
        self.player.enter(cp=world[0], persist=world[1], epoch=1 if reset else 0, reset=reset)

    def fire(self):
        self.player.send(P["CH_WORLD"], P["WORLD_TRIGGER_FIRE"],
                         bytes([1, len(SOLDIER_TRIGGER)]) + SOLDIER_TRIGGER)

    def soldiers(self):
        return [n for n in self.spawned if "Soldier" in n]


def check(condition, what):
    print(("ok    " if condition else "FAIL  ") + what, flush=True)
    return bool(condition)


def scenario(sim):
    results = []
    s = Session()
    s.enter(PRISON)
    # The simulator loads the whole map first, up to a minute with a cold disk cache.
    s.run_until(lambda: s.spawned, 90)
    results.append(check(s.owner != s.player.player_id and s.owner, "simulator owns the AI of a new world"))
    results.append(check(s.spawned and not s.soldiers(), "checkpoint Kismet spawned its scripted enemy: %s" % s.spawned))

    s.fire()
    s.run(30)
    results.append(check(len(s.soldiers()) == 1, "trigger spawned one soldier: %s" % s.soldiers()))
    results.append(check(s.hits > 0, "soldier attacked the player, %d events" % s.hits))

    s.spawned, s.hits = [], 0
    s.enter(PRISON, reset=True)
    s.run_until(lambda: s.spawned, 30)
    s.fire()
    s.run(5)
    results.append(check(len(s.soldiers()) == 1, "after a world reset the trigger works again, no duplicates: %s" % s.spawned))

    s.spawned = []
    s.enter(ADMIN, reset=True)
    s.run(40, at=(-9271.640, 1.562, 7.013))
    s.enter(PRISON, reset=True)
    # Fired while the map is still loading on the simulator.
    s.run(1)
    s.fire()
    s.run_until(lambda: s.soldiers(), 90)
    s.run(3)
    results.append(check(len(s.soldiers()) == 1, "trigger during map load is applied after it: %s" % s.spawned))

    results.append(check(sim.poll() is None, "simulator still running"))
    s.player.close()
    return all(results)


def wait_connected():
    end = time.monotonic() + 180
    while time.monotonic() < end:
        if os.path.exists(LOG):
            with open(LOG, "rb") as f:
                if b"MpConn: Connected" in f.read():
                    time.sleep(1)
                    return
        time.sleep(1)
    raise AssertionError("simulator did not connect")


def main():
    work = tempfile.mkdtemp(prefix="ol_simnode_test_")
    bin_dir = OUTLAST + "/Binaries/Win64"
    subprocess.check_call(["cmake", "-S", t.SERVER_SRC, "-B", work, "-G", "Ninja", "-DCMAKE_BUILD_TYPE=Release"],
                          stdout=subprocess.DEVNULL)
    subprocess.check_call(["cmake", "--build", work], stdout=subprocess.DEVNULL)
    relay = subprocess.Popen([work + "/OpenOL_Relay", "--port", str(t.PORT)], cwd=work, stdin=subprocess.DEVNULL,
                             env=dict(os.environ, OPENOL_SIM_SECRET=t.SIM_SECRET),
                             stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
    time.sleep(0.5)
    if os.path.exists(LOG):
        os.remove(LOG)
    shutil.copy(ROOT + "/Binaries/Win64/OLGame.exe", bin_dir + "/" + EXE)
    env = dict(os.environ, WINEPREFIX=os.environ.get("WINEPREFIX", os.path.expanduser("~/OL1Editor")), WINEDEBUG="-all",
               OPENOL_SIM_SECRET=t.SIM_SECRET)
    sim = subprocess.Popen(["xvfb-run", "-a", "wine", EXE, "server", "OLEntry?Game=Multiplayer.MultiplayerGame",
                            "-simnode", "-simaddr=127.0.0.1", "-simport=%d" % t.PORT, "-simroom=" + t.ROOM,
                            "-seekfreeloadingpcconsole", "-log", "-nosteam", "-nohomedir", "-nosound", "-unattended",
                            "-nopause", "-forcelogflush", "LOG=simtest.log"],
                           cwd=bin_dir, env=env, stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL,
                           stderr=subprocess.STDOUT)
    try:
        # Loading takes from seconds to a minute depending on the disk cache.
        wait_connected()
        return 0 if scenario(sim) else 1
    finally:
        subprocess.call(["pkill", "-f", r"^[^ ]*" + EXE.replace(".", r"\.")])
        time.sleep(2)
        relay.terminate()
        relay.wait()
        os.remove(bin_dir + "/" + EXE)
        shutil.rmtree(work, ignore_errors=True)
        print("Simulator log:", LOG)


if __name__ == "__main__":
    sys.exit(main())
