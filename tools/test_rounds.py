#!/usr/bin/env python3
"""Headless check of the round loop: lobby, round, result, 40 second break.

The real game (game/game.c) is compiled for a host with the stubs of
tools/ui_preview.py and then run frame by frame at 60 fps without a window. The
test does not look at pixels, it watches the state machine:

  * the first lobby lasts lobby_first, every later one exactly lobby_break;
  * a round ends either on the clock (survivors win) or the moment the killer
    takes the last life of the survivor (the killer wins immediately);
  * the roles are always opposite and the survivor never swings - only the
    killer can punch;
  * a round clock never exceeds round_time (three minutes).

The second part is balance: the player is driven by the same escape AI as the
bot, so a competent survivor should be able to live noticeably longer than one
standing still.

Run from the repository root:

    python3 gen.py
    python3 tools/test_rounds.py
"""
from __future__ import annotations

import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

import ui_preview  # noqa: E402  (reuses its stubs and the autostub linker)

MAIN = r"""
/* ==================== headless run of the round loop ==================== */
static double clock_s;

static void log_line(const char *kind, double a, double b) {
    printf("%s %.3f %.3f %.3f\n", kind, clock_s, a, b);
}

int main(int argc, char **argv) {
    int mode = argc > 1 && strcmp(argv[1], "flee") == 0;   /* 1: player runs */
    double seconds = argc > 2 ? atof(argv[2]) : 900.0;
    screen_w = 2400; screen_h = 1080;
    srand(mode ? 11 : 3);
    ds_main();
    ds_fn_init();
    dt = 1.0 / 60.0;
    double prev_phase = phase, phase_started = 0;
    int survivor_swings = 0, role_clash = 0, over_clock = 0;
    clock_s = 0;
    while (clock_s < seconds) {
        /* A survivor who can play: the same escape steering the bot uses. */
        if (mode && phase == PH_ROUND && hero->role == ROLE_SURVIVOR) {
            ds_fn_bot_think(hero);
            ds_fn_bot_flee(hero, foe);
        }
        ds_fn_update();
        clock_s += dt;
        if (phase == PH_ROUND) {
            if (hero->role == foe->role) role_clash++;
            if (hero->role == ROLE_SURVIVOR && hero->punch_t > 0) survivor_swings++;
            if (foe->role == ROLE_SURVIVOR && foe->punch_t > 0) survivor_swings++;
            if (round_left > round_time + 0.001) over_clock++;
        }
        if (phase != prev_phase) {
            double held = clock_s - phase_started;
            if (prev_phase == PH_LOBBY) log_line("lobby", held, 0);
            else if (prev_phase == PH_ROUND) log_line("round", held, result);
            else log_line("result", held, result);
            prev_phase = phase;
            phase_started = clock_s;
        }
    }
    printf("stats %d %d %d %.3f %.3f %.3f %.3f %.3f\n", survivor_swings, role_clash, over_clock,
           lobby_first, lobby_break, result_time, round_time, announce_time);
    return 0;
}
"""


def run(binary: Path, mode: str, seconds: float):
    done = subprocess.run([str(binary), mode, str(seconds)], capture_output=True, text=True)
    if done.returncode != 0:
        sys.exit(f"the game crashed on a host ({mode}):\n{done.stderr}")
    events, stats = [], None
    for line in done.stdout.splitlines():
        parts = line.split()
        if parts[0] == "stats":
            stats = [float(p) for p in parts[1:]]
        else:
            events.append((parts[0], float(parts[1]), float(parts[2]), float(parts[3])))
    return events, stats


def check(ok: bool, message: str, failures: list[str]):
    print(("  ok   " if ok else "  FAIL ") + message)
    if not ok:
        failures.append(message)


def main() -> int:
    if not (ROOT / "game" / "game.c").is_file():
        sys.exit("game/game.c is missing: run python3 gen.py first")
    failures: list[str] = []
    with tempfile.TemporaryDirectory(prefix="round-test-") as td:
        binary = ui_preview.compile_program(Path(td), MAIN, "round_test")

        print("idle player (stands still):")
        events, stats = run(binary, "idle", 900)
        swings, clashes, over, first, brk, res_t, round_t, announce = stats
        lobbies = [e for e in events if e[0] == "lobby"]
        rounds = [e for e in events if e[0] == "round"]
        results = [e for e in events if e[0] == "result"]
        check(len(rounds) >= 3, f"rounds played: {len(rounds)}", failures)
        check(abs(lobbies[0][2] - first) < 0.05,
              f"the first lobby lasts {lobbies[0][2]:.2f}s (waiting for {first:.0f}s)", failures)
        later = [e[2] for e in lobbies[1:]]
        check(all(abs(v - brk) < 0.05 for v in later),
              f"the break between rounds is {brk:.0f}s: {[round(v, 2) for v in later]}", failures)
        check(all(abs(e[2] - res_t) < 0.05 for e in results),
              f"the result plate holds {res_t:.0f}s", failures)
        check(all(e[3] in (1.0, 2.0) for e in rounds),
              "every round ends with a winner", failures)
        check(swings == 0, f"the survivor never swings (swings: {int(swings)})", failures)
        check(clashes == 0, f"the roles are always opposite (clashes: {int(clashes)})", failures)
        check(over == 0, "the round clock never exceeds three minutes", failures)
        idle_survivor = [e[1] for e in rounds if e[3] == 2.0]
        print(f"  info  rounds won by the killer: {len(idle_survivor)} of {len(rounds)}")

        print("player who runs away (the escape AI at the stick):")
        events, stats = run(binary, "flee", 900)
        swings, clashes, over, first, brk, res_t, round_t, announce = stats
        rounds = [e for e in events if e[0] == "round"]
        killer_rounds = [e for e in rounds if e[3] == 2.0]
        longest = max((e[2] for e in rounds), default=0)
        print(f"  info  rounds: {len(rounds)}, the killer took {len(killer_rounds)}, "
              f"the longest round {longest:.1f}s")
        check(longest > 25, f"a running survivor lives longer than 25s ({longest:.1f}s)", failures)
        check(all(e[2] <= round_t + announce + 0.2 for e in rounds),
              "no round runs past three minutes plus the role reveal", failures)

    if failures:
        print(f"\n{len(failures)} check(s) failed")
        return 1
    print("\nthe round loop is fine")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
