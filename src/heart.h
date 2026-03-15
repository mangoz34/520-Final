#ifndef HEART_H
#define HEART_H

#include "enviro.h"
#include <iostream>

using namespace enviro;

/**
 * @brief Process controller for the Heart power-up.
 * Manages deployment, dynamic horizontal movement, and speed scaling.
 */
class HeartController : public Process, public AgentInterface {
    bool is_deployed;
    bool is_game_over;
    bool needs_boost;
    double crash_x, crash_y;

    /// @brief Dynamic horizontal velocity that increases as the game progresses.
    double base_vx;

public:
    /**
     * @brief Constructs a new HeartController and initializes the base speed.
     */
    HeartController() : Process(), is_deployed(false), is_game_over(false), needs_boost(false), base_vx(-80) {}

    /**
     * @brief Initializes event listeners for game lifecycle and dynamic difficulty events.
     */
    void init() {
        watch("game_over", [this](Event& e) {
            if (!is_game_over) { is_game_over = true; crash_x = x(); crash_y = y(); }
        });

        watch("game_restart", [this](Event& e) {
            is_game_over = false;
            is_deployed = false;
            base_vx = -80;
            teleport(2000, 110, 0);
        });

        watch("deploy_heart", [this](Event& e) {
            if (!is_game_over) {
                is_deployed = true;
                teleport(450, 110, 0);
                needs_boost = true;
            }
        });

        watch("vanish_heart", [this](Event& e) {
            int target_id = e.value()["id"];
            if (target_id == id() && is_deployed) {
                is_deployed = false;
                teleport(2000, 110, 0);
                emit(Event("item_cleared"));
            }
        });

        /**
         * @brief Increases the movement speed when receiving the global speed-up broadcast.
         */
        watch("speed_up", [this](Event& e) {
            base_vx -= 20;
            std::cout << "[Heart] ID " << id() << " speeding up to " << base_vx << "!" << std::endl;
        });
    }

    void start() {}

    /**
     * @brief Core update loop applying dynamic velocity.
     */
    void update() {
        if (is_game_over) { teleport(crash_x, crash_y, 0); return; }
        if (!is_deployed) { track_velocity(0, 0); teleport(2000, 110, 0); return; }

        track_velocity(base_vx, 0);

        if (needs_boost) { omni_apply_force(-2500, 0); needs_boost = false; }

        if (x() < -450) {
            is_deployed = false;
            teleport(2000, 110, 0);
            emit(Event("item_cleared"));
        }
    }
    void stop() {}
};

/**
 * @brief The Heart agent representing a collectible health-restoring item.
 */
class Heart : public Agent {
public:
    Heart(json spec, World& world) : Agent(spec, world) { add_process(c); prevent_rotation(); }
private:
    HeartController c;
};

DECLARE_INTERFACE(Heart)

#endif