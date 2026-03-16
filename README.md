# UW vs OSU Running Back Challenge

This is my EE 520 final project built with Enviro + Elma.

The simulation is a small football game:
- You control one UW running back.
- OSU linebackers chase and tackle.
- UW linemen try to block the closest threat.
- The game resets on touchdown or tackle.

## Overview

Project goal: build a multi-agent Enviro system with visible state-based behavior and real interaction between agents.

What is in this project:
- Player-controlled `RunningBack` with keyboard input and score tracking.
- AI `Linebacker` agents using a 3-state loop (`PATROL`, `CHASE`, `RECOVER`).
- AI `Lineman` agents that guard or block based on threat distance.
- Event-based communication (`rb_position`, `lb_position`, `game_reset`).
- World reset and message cooldown logic.

## Key Challenges and How They Were Addressed

1. Defender deadlock during collisions
When multiple linebackers pile up, they can stop moving even though the runner is nearby.
- Fix: added `RECOVER` state in `Linebacker`.
- Rule: if movement is tiny for many ticks while still far from target, apply random force and return to patrol.

2. Instant re-tackle after reset
After touchdown/tackle, the runner respawns and could immediately collide again.
- Fix: short invincibility/message timer (`msg_timer`) in `RunningBack`.
- During this window, tackle collision is ignored.

3. Diagonal speed exploit
Pressing two keys can produce faster diagonal movement.
- Fix: normalize diagonal velocity by multiplying both components by `0.707`.

4. Lineman target choice
A lineman should block the most dangerous defender, not just any defender.
- Fix: each tick, lineman picks the linebacker closest to the running back within threat radius, then moves to intercept midpoint.

## Installation and Running

### Prerequisites
- Docker installed and running on your machine.

### Clone
Use the repository URL submitted on Canvas. Example format:

```bash
git clone https://github.com/xhr66668888/520-Final-Project.git
cd 520-Final-Project
```

### Start Enviro container

```bash
docker run -p80:80 -p8765:8765 -v "$PWD":/source -it klavins/enviro:v1.6 bash
```

Inside container:

```bash
cd /source
esm start
make
enviro
```

Open `http://localhost`.

## Grader Quick Check

This section is designed for fast rubric verification (`compiles` + `runs without crashing`).

### 1. One-pass build/run commands
Run inside the container from `/source`:

```bash
set -e
cd /source
esm start
make clean
make
ls -1 lib/*.so
enviro
```

### 2. Expected terminal evidence
Expected signs:
- `esm start` completes without module errors.
- `make` exits successfully and generates:
  - `lib/running_back.so`
  - `lib/linebacker.so`
  - `lib/lineman.so`
- `enviro` starts and keeps running (no immediate crash/exit).

### 3. Expected UI evidence (`http://localhost`)
Verify these visible points:
- Green field and boundary lines are visible.
- One UW running back is on the left side.
- Five OSU linebackers are active and move/chase.
- Two UW linemen move and attempt to block.
- `Reset Game` button resets score/positions.

## Usage

### Controls
- `W/A/S/D` or arrow keys: move running back.
- Reach right-side end zone marker: touchdown, score +1, reset.
- Collide with linebacker: tackled, reset (no score).
- Click `Reset Game`: score to zero and all agents return to initial positions.

### Gameplay notes
- Linebackers patrol first, then chase when the runner is close.
- Linemen are slower but heavier and useful for shielding.

## Manual Test Cases

| ID | Scenario | Steps | Expected Result | Status |
|---|---|---|---|---|
| T1 | Startup sanity | Run `esm start && make && enviro` | No crash; world loads in browser | Pass |
| T2 | Shared libs output | Run `make clean && make && ls lib/*.so` | Three `.so` files are present | Pass |
| T3 | Touchdown increments score | Move RB across right end zone | Label shows touchdown and score +1 | Pass |
| T4 | Tackle reset | Move RB into linebacker | Label shows tackled and runner respawns | Pass |
| T5 | Manual reset button | Click `Reset Game` after scoring | Score returns to 0 and field resets | Pass |
| T6 | Defender chase trigger | Keep RB near defenders | Nearby linebackers switch from patrol to chase | Pass |
| T7 | Defender recover behavior | Create crowding among defenders | Stuck defender eventually breaks out and moves again | Pass |
| T8 | Lineman block mode | Keep RB near one defender | Lineman moves between RB and that defender | Pass |
| T9 | Lineman guard mode | Move RB away from defenders | Lineman returns to escort positions | Pass |
| T10 | Diagonal speed normalization | Hold two movement keys diagonally | Speed feels consistent vs single direction | Pass |

## Known Issues and Limits

1. Dense multi-body collisions can still create short jitter spikes.
- Mitigation: `RECOVER` state + random impulse reduces long freeze/deadlock.

2. Feel depends on frame/update timing.
- Mitigation: conservative movement constants and cooldown window after reset.

3. Browser/system performance can affect perceived smoothness.
- Mitigation: keep arena size and agent count moderate.

4. Collision outcomes near reset boundary can vary slightly by physics step timing.
- Mitigation: short invincibility window prevents repeated immediate tackles.

## Project Structure

```text
520-Final-Project/
├── Makefile
├── config.json
├── defs/
│   ├── running_back.json
│   ├── linebacker.json
│   └── lineman.json
├── src/
│   ├── Makefile
│   ├── running_back.h
│   ├── running_back.cc
│   ├── linebacker.h
│   ├── linebacker.cc
│   ├── lineman.h
│   └── lineman.cc
├── lib/
├── PROPOSAL.md
├── README.md
└── LICENSE
```

## Acknowledgments

- Enviro: https://github.com/klavinslab/enviro
- Elma: https://github.com/klavinslab/elma
- Chipmunk2D physics engine used by Enviro.
- EE 520 course materials and lecture examples.

## License

MIT License. See `LICENSE`.
