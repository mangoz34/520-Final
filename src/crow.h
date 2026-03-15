#ifndef CROW_H
#define CROW_H

#include "enviro.h"
#include <iostream>

using namespace enviro;

/**
 * @brief Process controller for the Crow obstacle.
 * Manages deployment, horizontal flight, and a speed-scaling diagonal dive mechanic.
 */
class CrowController : public Process, public AgentInterface {
    bool is_deployed;
    bool is_game_over;
    bool has_dived;
    bool needs_boost;
    double crash_x, crash_y;

    /// @brief Dynamic velocity for the crow. Starts significantly faster than ground obstacles.
    double base_vx;

public:
    /**
     * @brief Constructs a new CrowController and initializes the base flying speed.
     */
    CrowController() : Process(), is_deployed(false), is_game_over(false), has_dived(false), needs_boost(false), base_vx(-150) {}

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
            base_vx = -150;
            teleport(2000, -50, 0);
        });

        watch("deploy_crow", [this](Event& e) {
            if (!is_game_over) {
                is_deployed = true;
                has_dived = false;
                teleport(450, 75, 0);
                needs_boost = true;
            }
        });

        watch("vanish_crow", [this](Event& e) {
            int target_id = e.value()["id"];
            if (target_id == id() && is_deployed) {
                is_deployed = false;
                teleport(2000, -50, 0);
                emit(Event("item_cleared"));
            }
        });

        /**
         * @brief Increases the flying speed when receiving the global speed-up broadcast.
         */
        watch("speed_up", [this](Event& e) {
            base_vx -= 20;
            std::cout << "[Crow] ID " << id() << " speeding up to " << base_vx << "!" << std::endl;
        });
    }

    void start() {}

    /**
     * @brief Core update loop handling the dynamically scaling diagonal dive.
     */
    void update() {
        if (is_game_over) { teleport(crash_x, crash_y, 0); return; }
        if (!is_deployed) { track_velocity(0, 0); teleport(2000, -50, 0); return; }

        if (needs_boost) {
            omni_apply_force(-3000, 0);
            needs_boost = false;
        }

        if (x() < 200 && !has_dived) {
            has_dived = true;
        }

        if (has_dived) {
            /**
             * @brief Diagonal dive seamlessly scales with the dynamic speed.
             * Because base_vx is negative, -base_vx yields an equal positive downward speed,
             * preserving the exact dive angle while increasing the velocity.
             */
            track_velocity(base_vx, -base_vx);
        } else {
            /// @brief Level flight before the dive point.
            track_velocity(base_vx, 0);
        }

        if (x() < -450 || y() > 150) {
            is_deployed = false;
            teleport(2000, -50, 0);
            emit(Event("item_cleared"));
        }
    }

    void stop() {}
};

/**
 * @brief The Crow agent representing a dynamic, speed-scaling aerial obstacle.
 */
class Crow : public Agent {
public:
    Crow(json spec, World& world) : Agent(spec, world) {
        add_process(c);
        prevent_rotation();
    }
private:
    CrowController c;
};

DECLARE_INTERFACE(Crow)

#endif