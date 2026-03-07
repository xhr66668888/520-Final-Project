#ifndef __LINEBACKER_AGENT__H
#define __LINEBACKER_AGENT__H

#include "enviro.h"
#include <string>
#include <cmath>
#include <cstdlib>

using namespace enviro;

/** Linebacker state modes. */
enum class LBState { PATROL, CHASE, RECOVER };

/**
 * @brief Controller for one OSU linebacker.
 *
 * Reads running-back position events and switches between patrol,
 * chase, and recover behavior. Also returns to home position on
 * "game_reset".
 */
class LinebackerController : public Process, public AgentInterface {

public:
    /** Constructor. */
    LinebackerController() : Process(), AgentInterface(),
        state(LBState::PATROL),
        rb_x(0), rb_y(0),
        patrol_dir(1.0), patrol_timer(0),
        recover_timer(0),
        home_x(0), home_y(0),
        stuck_counter(0), last_x(0), last_y(0) {}

    /** Initialize event handlers and home position. */
    void init() {
        prevent_rotation();
        home_x = x();
        home_y = y();
        last_x = x();
        last_y = y();

        // Randomize initial patrol direction so defenders do not stay synchronized.
        if (std::rand() % 2 == 0) patrol_dir = -1.0;

        // Track the running back's broadcast position
        watch("rb_position", [&](Event& e) {
            rb_x = e.value()["x"];
            rb_y = e.value()["y"];
        });

        // Reset on game restart
        watch("game_reset", [&](Event& e) {
            state = LBState::PATROL;
            recover_timer = 0;
            stuck_counter = 0;
            teleport(home_x, home_y, 0);
        });

        // Center marker for easier visual tracking.
        decorate("<circle cx='0' cy='0' r='3' style='fill:white'></circle>");
    }

    /** @brief Start callback (unused). */
    void start() {}

    /** Main update loop for state transitions and movement. */
    void update() {
        // Vector from this agent to the running back
        double dx   = rb_x - x();
        double dy   = rb_y - y();
        double dist = std::sqrt(dx * dx + dy * dy);

        // Broadcast own position for UW Linemen
        emit(Event("lb_position", {{"x", x()}, {"y", y()}, {"id", id()}}));

        // Movement since last tick (used for stuck detection)
        double mdx   = x() - last_x;
        double mdy   = y() - last_y;
        double moved  = std::sqrt(mdx * mdx + mdy * mdy);
        last_x = x();
        last_y = y();

        // ---- State machine ----
        switch (state) {

        case LBState::PATROL:
            do_patrol();
            if (dist < DETECTION_RANGE) {
                state = LBState::CHASE;
                stuck_counter = 0;
            }
            label("OSU", -10, 5);
            break;

        case LBState::CHASE:
            do_chase(dx, dy, dist);
            // Stuck detection: if barely moving but far from target
            if (moved < STUCK_THRESHOLD && dist > 30.0) {
                stuck_counter++;
                if (stuck_counter > STUCK_LIMIT) {
                    state = LBState::RECOVER;
                    recover_timer = RECOVER_DURATION;
                    stuck_counter = 0;
                }
            } else {
                stuck_counter = 0;
            }
            // Lost the runner
            if (dist > LOST_RANGE) {
                state = LBState::PATROL;
            }
            label("OSU!", -12, 5);
            break;

        case LBState::RECOVER:
            do_recover();
            recover_timer--;
            if (recover_timer <= 0) {
                state = LBState::PATROL;
            }
            label("OSU..", -14, 5);
            break;
        }
    }

    /** @brief Stop callback (unused). */
    void stop() {}

private:
    /** Patrol behavior: vertical lane movement with periodic direction swap. */
    void do_patrol() {
        patrol_timer++;
        if (patrol_timer > PATROL_SWITCH) {
            patrol_timer = 0;
            patrol_dir = -patrol_dir;
        }
        omni_track_velocity(0, patrol_dir * PATROL_SPEED, 10);
    }

    /** Chase behavior: move toward current running-back position. */
    void do_chase(double dx, double dy, double dist) {
        if (dist > 1.0) {
            double target_vx = (dx / dist) * CHASE_SPEED;
            double target_vy = (dy / dist) * CHASE_SPEED;
            omni_track_velocity(target_vx, target_vy, 15);
        }
    }

    /** Recover behavior: apply random impulse to break collision deadlock. */
    void do_recover() {
        double rfx = static_cast<double>((std::rand() % 200) - 100);
        double rfy = static_cast<double>((std::rand() % 200) - 100);
        omni_apply_force(rfx, rfy);
        omni_damp_movement();
    }

    // State
    LBState state;                    ///< Current behavioural state
    double  rb_x, rb_y;              ///< Last-known running-back position
    double  patrol_dir;              ///< Current patrol direction (+1 / −1)
    int     patrol_timer;            ///< Ticks in current patrol direction
    int     recover_timer;           ///< Remaining recovery ticks
    double  home_x, home_y;         ///< Starting position (for reset)
    int     stuck_counter;           ///< Consecutive "barely moved" ticks
    double  last_x, last_y;         ///< Position at previous tick

    // Tuning constants
    static constexpr double DETECTION_RANGE = 200.0;  ///< Begin chasing
    static constexpr double LOST_RANGE      = 350.0;  ///< Give up chasing
    static constexpr double PATROL_SPEED    = 15.0;   ///< Patrol velocity
    static constexpr double CHASE_SPEED     = 50.0;   ///< Chase velocity
    static constexpr double STUCK_THRESHOLD = 0.3;    ///< Min movement / tick
    static constexpr int    STUCK_LIMIT     = 50;     ///< Ticks before recover
    static constexpr int    RECOVER_DURATION = 30;    ///< Recovery ticks
    static constexpr int    PATROL_SWITCH   = 80;     ///< Ticks per patrol leg
};

/**
 * @brief OSU linebacker agent.
 */
class Linebacker : public Agent {
public:
    Linebacker(json spec, World& world) : Agent(spec, world) {
        add_process(c);
    }
private:
    LinebackerController c;
};

DECLARE_INTERFACE(Linebacker)

#endif
