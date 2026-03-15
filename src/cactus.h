#ifndef CACTUS_H
#define CACTUS_H

#include "enviro.h"
#include <iostream>

using namespace enviro;

/**
 * @brief Process controller for the Cactus obstacle.
 * Manages deployment, dynamic horizontal movement, and speed scaling.
 */
class CactusController : public Process, public AgentInterface {
    bool is_deployed;
    bool is_game_over;
    bool needs_boost;
    double crash_x, crash_y;

    /// @brief Dynamic horizontal velocity that increases as the game progresses.
    double base_vx;

public:
    /**
     * @brief Constructs a new CactusController and initializes the base speed.
     */
    CactusController() : Process(), is_deployed(false), is_game_over(false), needs_boost(false), base_vx(-80) {}

    /**
     * @brief Initializes event listeners for game lifecycle and dynamic difficulty events.
     */
    void init() {
        /**
         * @brief Freezes the cactus when the game ends.
         */
        watch("game_over", [this](Event& e) {
            if (!is_game_over) {
                is_game_over = true;
                crash_x = x();
                crash_y = y();
            }
        });

        /**
         * @brief Resets the cactus state and restores its initial slow speed.
         */
        watch("game_restart", [this](Event& e) {
            is_game_over = false;
            is_deployed = false;
            base_vx = -80;
            teleport(2000, 110, 0);
        });

        /**
         * @brief Deploys the cactus to the starting line.
         */
        watch("deploy_cactus", [this](Event& e) {
            if (!is_game_over) {
                is_deployed = true;
                teleport(450, 110, 0);
                needs_boost = true;
            }
        });

        /**
         * @brief Hides the cactus upon collision and notifies the Director.
         */
        watch("vanish_cactus", [this](Event& e) {
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
            std::cout << "[Cactus] ID " << id() << " speeding up to " << base_vx << "!" << std::endl;
        });
    }

    void start() {}

    /**
     * @brief Core update loop applying dynamic velocity.
     */
    void update() {
        if (is_game_over) { teleport(crash_x, crash_y, 0); return; }
        if (!is_deployed) { track_velocity(0, 0); teleport(2000, 110, 0); return; }

        /// @brief Move left using the dynamically updated base velocity.
        track_velocity(base_vx, 0);

        if (needs_boost) {
            omni_apply_force(-2500, 0);
            needs_boost = false;
        }

        if (x() < -450) {
            is_deployed = false;
            teleport(2000, 110, 0);
            emit(Event("item_cleared"));
        }
    }
    void stop() {}
};

/**
 * @brief The Cactus agent representing a scaling ground obstacle.
 */
class Cactus : public Agent {
public:
    Cactus(json spec, World& world) : Agent(spec, world) {
        add_process(c);
        prevent_rotation();
    }
private:
    CactusController c;
};

DECLARE_INTERFACE(Cactus)

#endif