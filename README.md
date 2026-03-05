# UW vs OSU – The Running Back Challenge 🏈

A multi-agent simulation game built with [**Enviro**](https://github.com/klavinslab/enviro) and [**Elma**](https://github.com/klavinslab/elma). You play as a **University of Washington (UW) Huskies** running back trying to reach the endzone while dodging **Ohio State University (OSU) Buckeyes** linebackers. Your AI-controlled UW offensive linemen will try to block defenders for you — but you still need quick reflexes to score!

![Game Preview](https://img.shields.io/badge/enviro-v1.6-blue) ![C++17](https://img.shields.io/badge/C%2B%2B-17-green) ![License](https://img.shields.io/badge/license-MIT-yellow)

---

## Overview

This project is a **football-themed multi-agent arcade game** that demonstrates:

| Feature | Description |
|---|---|
| **Player Control** | WASD / Arrow key control of an omni-directional running back |
| **AI Defenders** | 5 OSU Linebackers with a three-state machine (Patrol → Chase → Recover) |
| **AI Allies** | 2 UW Linemen that autonomously intercept the nearest threatening defender |
| **Game Logic** | Touchdown scoring, tackle detection, automatic reset, and a Reset button |
| **Inter-Agent Communication** | Agents broadcast and listen for positions via the Elma event system |

### How to Play

- Use **W A S D** (or **Arrow Keys**) to move the purple UW running back.
- Dodge the red OSU linebackers and reach the **right side** of the field past the red goal-line markers.
- Your dark-purple UW linemen will try to body-block defenders — use them as shields!
- **Score a touchdown** → you earn a point and the field resets.
- **Get tackled** → the field resets (no point).
- Click **Reset Game** to start over with score = 0.

---

## Agents

### 1. Running Back (Player – UW Huskies)

- **Color:** Purple with gold border
- **Type:** Omni-directional, lightweight, fast
- **Behavior:** Reads `keydown` / `keyup` events and tracks the desired velocity. Broadcasts its position every tick via the `"rb_position"` event so all other agents can react.

### 2. Linebacker (AI Enemy – OSU Buckeyes)

- **Color:** Scarlet with gray border
- **Type:** Omni-directional, medium weight, medium speed
- **State Machine:**

```
  ┌──────────┐   detect RB   ┌──────────┐
  │  PATROL   │─────────────▶│   CHASE   │
  │ (wander)  │◀─────────────│ (pursue)  │
  └──────────┘   lost RB     └────┬──────┘
                                   │ stuck
                              ┌────▼──────┐
                              │  RECOVER   │
                              │ (un-stuck) │
                              └───────────┘
```

| State | Trigger | Behavior |
|-------|---------|----------|
| **Patrol** | Default | Oscillate vertically in assigned lane |
| **Chase** | RB within 200 px | Calculate unit vector toward RB, track velocity at chase speed |
| **Recover** | Velocity ≈ 0 for 50 ticks (deadlock) | Apply random impulse to break free, then resume patrol |

### 3. Lineman (AI Ally – UW Huskies)

- **Color:** Dark purple with gold-cream border
- **Type:** Omni-directional, very heavy, slow
- **Behavior:**

| Mode | Condition | Action |
|------|-----------|--------|
| **Guard** | No linebacker within 250 px of RB | Stay in loose formation ahead of the running back |
| **Block** | Linebacker threatening RB | Move toward the midpoint between RB and the nearest linebacker, physically pushing the defender |

---

## Key Challenges and Solutions

### Challenge 1 – Multi-Agent Deadlock

**Problem:** Two or more OSU linebackers chasing the same target can collide and cancel each other's forces, causing them to freeze in place permanently.

**Solution:** The `RECOVER` state. Each linebacker tracks its displacement per tick. If it has barely moved (`< 0.3 px/tick`) for 50 consecutive ticks while not adjacent to the target, it transitions to `RECOVER`, applies a random impulse, and returns to `PATROL`. This breaks the physical deadlock.

### Challenge 2 – Invincibility Window

**Problem:** After a tackle or touchdown, the running back teleports to the start. If a linebacker happens to be near the start, the player is immediately tackled again before they can react.

**Solution:** A `msg_timer` (50 ticks ≈ 0.8 s) acts as an invincibility window after every reset. During this window, collision events with linebackers are ignored, and the "TOUCHDOWN!" or "TACKLED!" message is displayed.

### Challenge 3 – Diagonal Speed Boost

**Problem:** Pressing two directional keys simultaneously (e.g., W + D) would result in a speed of `√2 × SPEED`, giving the player an unfair advantage.

**Solution:** When both x and y velocity components are nonzero, each is multiplied by `0.707 (≈ 1/√2)` to normalize the total speed.

### Challenge 4 – Lineman Target Selection

**Problem:** With five linebackers on the field, the lineman needs to decide whom to block. Blocking the wrong defender wastes its effort.

**Solution:** Each tick, every linebacker broadcasts `"lb_position"`. The lineman iterates over all known positions and picks the one closest to the running back (within a 250 px threat radius). It then moves to the midpoint between the running back and that defender, maximizing the chance of a successful block.

---

## Installation and Running

### Prerequisites

- [Docker](https://www.docker.com/) installed and running.

### Quick Start

```bash
# 1. Clone the repository
git clone https://github.com/<your-username>/520-Final-Project.git
cd 520-Final-Project

# 2. Start the Enviro Docker container
docker run -p80:80 -p8765:8765 -v $PWD:/source -it klavins/enviro:v1.6 bash

# 3. Inside the container, build and run
esm start
make
enviro
```

Then open **http://localhost** in your browser. You should see the football field with all agents.

### Controls

| Key | Action |
|-----|--------|
| **W** / **↑** | Move up |
| **A** / **←** | Move left |
| **S** / **↓** | Move down |
| **D** / **→** | Move right |
| **Reset Game** button | Reset score and positions |

### Stopping

Press `Ctrl-C` in the Docker terminal to stop the enviro server.

---

## Project Structure

```
520-Final-Project/
├── Makefile              # Top-level build (delegates to src/)
├── config.json           # Enviro world configuration
├── defs/
│   ├── running_back.json # RB physical properties
│   ├── linebacker.json   # LB physical properties
│   └── lineman.json      # LM physical properties
├── src/
│   ├── Makefile           # Builds .so shared libraries
│   ├── running_back.h     # RunningBack controller
│   ├── running_back.cc
│   ├── linebacker.h       # Linebacker AI controller
│   ├── linebacker.cc
│   ├── lineman.h          # Lineman AI controller
│   └── lineman.cc
├── lib/                   # Compiled shared objects (generated)
├── README.md
├── LICENSE
└── PROPOSAL.md
```

---

## Acknowledgments

- **[Enviro](https://github.com/klavinslab/enviro)** – Multi-agent simulation environment by Eric Klavins (UW ECE).
- **[Elma](https://github.com/klavinslab/elma)** – Event Loop Manager used for process scheduling and event handling.
- **[Chipmunk2D](https://chipmunk-physics.net/)** – 2D physics engine underlying Enviro.
- **UW EE/CSE 520** – Software Engineering for Embedded Systems course that inspired this project.
- Game concept inspired by the storied **Big Ten rivalry** between the University of Washington Huskies and Ohio State University Buckeyes.

---

## License

This project is licensed under the **MIT License** – see the [LICENSE](LICENSE) file for details.
