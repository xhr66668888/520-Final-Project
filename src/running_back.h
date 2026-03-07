#ifndef __RUNNING_BACK_AGENT__H
#define __RUNNING_BACK_AGENT__H

#include "enviro.h"
#include <string>

using namespace enviro;

/**
 * @brief Controller for the player-controlled UW Running Back.
 * 
 * Handles WASD keyboard input for omni-directional movement, detects
 * touchdown events when the running back reaches the endzone, and
 * handles tackle collisions with OSU Linebackers. Broadcasts its 
 * position each tick so teammates and opponents can react.
 */
class RunningBackController : public Process, public AgentInterface {

public:
    /**
     * @brief Construct a new RunningBackController.
     * Initializes all keyboard flags and game state variables.
     */
    RunningBackController() : Process(), AgentInterface(),
        UP(false), DOWN(false), LEFT(false), RIGHT(false),
        score(0), tackled_flag(false), msg_timer(0) {}

    /**
     * @brief Initialize the controller.
     * Sets up keyboard event watchers (WASD + Arrow keys),
     * collision handlers for Linebackers, and the reset button.
     */
    void init() {
        prevent_rotation();

        watch("keydown", [&](Event& e) {
            std::string k = e.value()["key"];
            if      (k == "w" || k == "ArrowUp")    UP    = true;
            else if (k == "s" || k == "ArrowDown")  DOWN  = true;
            else if (k == "a" || k == "ArrowLeft")  LEFT  = true;
            else if (k == "d" || k == "ArrowRight") RIGHT = true;
        });

        watch("keyup", [&](Event& e) {
            std::string k = e.value()["key"];
            if      (k == "w" || k == "ArrowUp")    UP    = false;
            else if (k == "s" || k == "ArrowDown")  DOWN  = false;
            else if (k == "a" || k == "ArrowLeft")  LEFT  = false;
            else if (k == "d" || k == "ArrowRight") RIGHT = false;
        });

        notice_collisions_with("Linebacker", [&](Event& e) {
            if (msg_timer <= 0) {
                tackled_flag = true;
            }
        });

        watch("button_click", [&](Event& e) {
            if (e.value()["name"] == "reset") {
                reset_game();
            }
        });
    }

    /** @brief Start callback (unused). */
    void start() {}

    /**
     * @brief Main update loop.
     * Processes keyboard input, applies movement forces, broadcasts
     * position for other agents, and checks touchdown / tackle conditions.
     */
    void update() {
        // Always broadcast position for teammates and enemies
        emit(Event("rb_position", {{"x", x()}, {"y", y()}}));

        // ---- Movement ----
        double vx_target = 0, vy_target = 0;
        if (UP)    vy_target -= SPEED;
        if (DOWN)  vy_target += SPEED;
        if (LEFT)  vx_target -= SPEED;
        if (RIGHT) vx_target += SPEED;

        // Normalize diagonal movement to prevent faster diagonal speed
        if (vx_target != 0 && vy_target != 0) {
            vx_target *= 0.707;
            vy_target *= 0.707;
        }
        omni_track_velocity(vx_target, vy_target, 20);

        // ---- Message display cooldown (invincibility window) ----
        if (msg_timer > 0) {
            msg_timer--;
            return;
        }

        // ---- Touchdown detection ----
        if (x() > ENDZONE_X) {
            score++;
            finish_play("TOUCHDOWN! Score: " + std::to_string(score));
            return;
        }

        // ---- Tackle detection ----
        if (tackled_flag) {
            tackled_flag = false;
            finish_play("TACKLED!");
            return;
        }

        // ---- Normal HUD label ----
        label("UW  Score:" + std::to_string(score), -30, 5);
    }

    /** @brief Stop callback (unused). */
    void stop() {}

private:
    // Shared path for touchdown and tackle handling.
    void finish_play(const std::string& message) {
        label(message, -50, 5);
        emit(Event("game_reset"));
        teleport(START_X, START_Y, 0);
        UP = DOWN = LEFT = RIGHT = false;
        msg_timer = MSG_DURATION;
    }

    /**
     * @brief Fully reset the game: score, flags, and position.
     */
    void reset_game() {
        score = 0;
        tackled_flag = false;
        msg_timer = 0;
        teleport(START_X, START_Y, 0);
        UP = DOWN = LEFT = RIGHT = false;
        emit(Event("game_reset"));
    }

    bool UP, DOWN, LEFT, RIGHT;  ///< Keyboard state flags
    bool tackled_flag;            ///< Set true on collision with Linebacker
    int  score;                   ///< Number of touchdowns scored
    int  msg_timer;               ///< Cooldown timer after touchdown / tackle

    static constexpr double SPEED      = 70.0;   ///< Movement speed
    static constexpr double ENDZONE_X  = 350.0;  ///< X threshold for touchdown
    static constexpr double START_X    = -400.0;  ///< Respawn X position
    static constexpr double START_Y    = 0.0;     ///< Respawn Y position
    static constexpr int    MSG_DURATION = 50;    ///< Invincibility ticks after event
};

/**
 * @brief UW Running Back agent.
 * Player-controlled omni-directional agent representing the UW Huskies
 * running back. Goal: reach the right endzone without being tackled.
 */
class RunningBack : public Agent {
public:
    RunningBack(json spec, World& world) : Agent(spec, world) {
        add_process(c);
    }
private:
    RunningBackController c;
};

DECLARE_INTERFACE(RunningBack)

#endif
