# Final Project Proposal – UW vs OSU: The Running Back Challenge

## Author
*EE/CSE 520 – Software Engineering for Embedded Systems*

---

## Project Summary

This project is a **multi-agent football arcade game** built on top of the Enviro simulation platform. The player controls a UW Huskies running back and must navigate from one end of a field to the opposite endzone while avoiding AI-controlled OSU Buckeyes linebackers. AI-controlled UW offensive linemen assist by physically blocking defenders.

---

## Goals

1. **Demonstrate multi-agent coordination** – three distinct agent types (player, enemy AI, ally AI) interact in real time through physics and events.
2. **Implement a non-trivial state machine** – each OSU linebacker runs a three-state machine (Patrol → Chase → Recover) that reacts to the player's position.
3. **Create an engaging, playable game** – keyboard controls, scoring, automatic resets, and visual feedback make the simulation feel like an actual game.

---

## Agent Design

| Agent | Role | Control | Key Behavior |
|-------|------|---------|--------------|
| **RunningBack** | Player | WASD keyboard | Move toward endzone; detect touchdown and tackle |
| **Linebacker** (×5) | Enemy AI | Autonomous | Patrol zone → Chase when RB nearby → Recover if stuck |
| **Lineman** (×2) | Ally AI | Autonomous | Guard formation → Block nearest threatening LB |

---

## Technical Approach

- **Inter-agent communication:** The running back broadcasts `"rb_position"` each tick. Linebackers and linemen watch this event to make decisions. Linebackers also broadcast `"lb_position"` so linemen can prioritise threats.
- **Deadlock prevention:** A `RECOVER` state with random impulse prevents linebackers from permanently freezing when they collide.
- **Invincibility window:** After touchdown or tackle, a cooldown timer prevents immediate re-collision at the spawn point.
- **Normalised movement:** Diagonal keyboard input is normalised to prevent faster-than-intended movement.

---

## Milestones

| Week | Deliverable |
|------|-------------|
| 1 | Environment layout, running back movement, collision detection |
| 2 | Linebacker AI (Patrol / Chase / Recover), lineman blocking |
| 3 | Polish, README, documentation, edge-case testing |

---

## Tools & Libraries

- **Enviro v1.6** – multi-agent simulator
- **Elma** – event loop and state machine framework
- **Chipmunk2D** – 2D rigid-body physics
- **Docker** – `klavins/enviro:v1.6`
- **C++17** with g++

---

## Expected Outcome

A fully playable browser-based game where a human player manoeuvres through AI defenders with the help of AI blockers. The project exercises event-driven programming, state machines, multi-agent communication, and physics-based interaction — all core topics of EE/CSE 520.
