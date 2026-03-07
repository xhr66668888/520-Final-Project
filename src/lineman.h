#ifndef __LINEMAN_AGENT__H
#define __LINEMAN_AGENT__H

#include "enviro.h"
#include <string>
#include <map>
#include <cmath>
#include <utility>

using namespace enviro;

/**
 * @brief Controller for one UW lineman.
 *
 * Tracks runner/defender positions and switches between guard and block logic.
 */
class LinemanController : public Process, public AgentInterface {

public:
    /** Constructor. */
    LinemanController() : Process(), AgentInterface(),
        rb_x(0), rb_y(0),
        start_x(0), start_y(0) {}

    /** Initialize event handlers and save start position. */
    void init() {
        prevent_rotation();
        start_x = x();
        start_y = y();

        // Track the running back
        watch("rb_position", [&](Event& e) {
            rb_x = e.value()["x"];
            rb_y = e.value()["y"];
        });

        // Track every linebacker
        watch("lb_position", [&](Event& e) {
            int    lb_id = e.value()["id"];
            double lx    = e.value()["x"];
            double ly    = e.value()["y"];
            lb_positions[lb_id] = std::make_pair(lx, ly);
        });

        // Reset on game restart
        watch("game_reset", [&](Event& e) {
            teleport(start_x, start_y, 0);
            lb_positions.clear();
        });

        // Center marker.
        decorate("<circle cx='0' cy='0' r='5' style='fill:#39275B'></circle>");
    }

    /** @brief Start callback (unused). */
    void start() {}

    /** Main update loop for guard/block selection and movement. */
    void update() {
        // ---- Find most threatening linebacker ----
        double  min_dist    = 1e9;
        double  target_x    = 0;
        double  target_y    = 0;
        bool    found_threat = false;

        for (const auto& entry : lb_positions) {
            double lx = entry.second.first;
            double ly = entry.second.second;
            double dx = lx - rb_x;
            double dy = ly - rb_y;
            double dist_to_rb = std::sqrt(dx * dx + dy * dy);

            if (dist_to_rb < THREAT_RANGE && dist_to_rb < min_dist) {
                min_dist    = dist_to_rb;
                target_x    = lx;
                target_y    = ly;
                found_threat = true;
            }
        }

        if (found_threat) {
            // BLOCK: move to a point between running back and the linebacker
            double mid_x = (rb_x + target_x) / 2.0;
            double mid_y = (rb_y + target_y) / 2.0;
            omni_move_toward(mid_x, mid_y, BLOCK_SPEED);
            label("BLOCK!", -14, 5);
        } else {
            // GUARD: stay slightly ahead and to the assigned side
            double side   = (start_y > 0) ? 1.0 : -1.0;
            double gx     = rb_x + 50.0;
            double gy     = rb_y + side * 35.0;
            omni_move_toward(gx, gy, GUARD_SPEED);
            label("UW OL", -14, 5);
        }
    }

    /** @brief Stop callback (unused). */
    void stop() {}

private:
    double rb_x, rb_y;            ///< Last-known running-back position
    double start_x, start_y;      ///< Starting position (for reset)

    /// Map of linebacker id -> (x, y) position
    std::map<int, std::pair<double, double>> lb_positions;

    static constexpr double THREAT_RANGE = 250.0;  ///< Range to switch to BLOCK
    static constexpr double BLOCK_SPEED  = 4.0;    ///< Speed when blocking
    static constexpr double GUARD_SPEED  = 2.0;    ///< Speed when guarding
};

/**
 * @brief UW lineman blocker agent.
 */
class Lineman : public Agent {
public:
    Lineman(json spec, World& world) : Agent(spec, world) {
        add_process(c);
    }
private:
    LinemanController c;
};

DECLARE_INTERFACE(Lineman)

#endif
