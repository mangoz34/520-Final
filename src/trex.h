#ifndef TREX_H
#define TREX_H

#include "enviro.h"
#include <iostream>

using namespace enviro;

/**
 * @brief The initial ready state of the T-Rex before the game begins.
 * Teleports the T-Rex to the starting position and resets its momentum.
 */
class Ready : public State, public AgentInterface {
public:
    Ready() : State("Ready") {}
    void entry(const Event& e) { teleport(-100, 110, 0); track_velocity(0, 0); omni_apply_force(0, 0); }
    void during() {}
    void exit(const Event& e) {}
};

/**
 * @brief The normal running state of the T-Rex.
 * Continuously clears residual momentum to keep the T-Rex grounded.
 */
class Running : public State, public AgentInterface {
public:
    Running() : State("Running") {}
    void entry(const Event& e) { teleport(x(), 110, 0); omni_apply_force(0, 0); }
    void during() {}
    void exit(const Event& e) {}
};

/**
 * @brief The jumping state of the T-Rex.
 * Applies an upward force and handles landing detection.
 */
class Jumping : public State, public AgentInterface {
    int ticks;
public:
    Jumping() : State("Jumping") {}

    /**
     * @brief Initializes the jump. Resets Y-position if jumped early to clear momentum,
     * then applies upward force.
     * @param e The event that triggered the transition.
     */
    void entry(const Event& e) {
        ticks = 0;
        if (y() > 90) teleport(x(), 110, 0);
        omni_apply_force(0, -8500);
    }

    /**
     * @brief Applies downward gravity and checks for landing conditions.
     */
    void during() {
        ticks++;
        omni_apply_force(0, 1000);
        if (ticks > 5 && y() >= 109) emit(Event("land"));
    }
    void exit(const Event& e) {}
};

/**
 * @brief The game over state representing the dead T-Rex.
 * Freezes the T-Rex exactly where it crashed.
 */
class Dead : public State, public AgentInterface {
    double crash_x, crash_y;
public:
    Dead() : State("Dead") {}
    void entry(const Event& e) {
        crash_x = x(); crash_y = y();
        std::cout << "[TRex] 💀 GAME OVER! Frozen in place." << std::endl;
    }
    void during() { teleport(crash_x, crash_y, 0); }
    void exit(const Event& e) {}
};

/**
 * @brief The main state machine controller for the T-Rex agent.
 * Manages lives, invincibility frames, state transitions, and collision handling.
 */
class TRexController : public StateMachine, public AgentInterface {
    int lives;
    bool is_invincible;
    double invincibility_timer;
public:
    TRexController() : StateMachine(), lives(3), is_invincible(false), invincibility_timer(0) {
        set_initial(ready);
        add_transition("jump", ready, jumping);
        add_transition("jump", running, jumping);
        add_transition("jump", jumping, jumping);
        add_transition("land", jumping, running);
        add_transition("die", running, dead);
        add_transition("die", jumping, dead);
        add_transition("die", ready, dead);
        add_transition("restart", dead, ready);
    }

    /**
     * @brief Initializes the controller, sets up key listeners, and collision detection logic.
     */
    void init() {
        StateMachine::init();

        set_style({
            {"fill", "rgba(0,0,0,0)"},
            {"stroke", "rgba(0,0,0,0)"},
            {"strokeWidth", "0"}
        });

        decorate(
            "<svg width='40' height='40' viewBox='0 0 40 40' style='transform: translate(-20px, -20px);'>"
            "<rect x='20' y='5' width='15' height='10' fill='#53b56c'/>"
            "<rect x='22' y='7' width='3' height='3' fill='black'/>"
            "<rect x='15' y='5' width='10' height='20' fill='#53b56c'/>"
            "<rect x='10' y='15' width='20' height='15' fill='#53b56c'/>"
            "<rect x='5' y='20' width='5' height='5' fill='#53b56c'/>"
            "<rect x='25' y='18' width='6' height='3' fill='#53b56c'/>"
            "<rect x='12' y='30' width='4' height='10' fill='#53b56c'/>"
            "<rect x='22' y='30' width='4' height='10' fill='#53b56c'/>"
            "</svg>"
        );

        watch("keydown", [this](Event& e) {
            std::string key = e.value()["key"];
            if (key == " ") {
                if (current().name() == "Dead") {
                    lives = 3; is_invincible = false;
                    emit(Event("update_lives", {{"lives", lives}}));
                    emit(Event("restart"));
                    emit(Event("game_restart"));
                }
                else if (current().name() == "Ready" || current().name() == "Running") {
                    emit(Event("jump"));
                }
                else if (current().name() == "Jumping" && y() > 90) {
                    emit(Event("jump"));
                }
            }
        });

        /**
         * @note Ensure that the strings "cactus", "heart", and "star" exactly match
         * the "name" property defined in their respective JSON configuration files.
         */
        notice_collisions_with("cactus", [this](Event& e) {
            int target_id = e.value()["id"];
            emit(Event("vanish_cactus", {{"id", target_id}}));

            if (current().name() != "Dead" && !is_invincible) {
                lives--;
                std::cout << "[TRex] Collided with Cactus ID: " << target_id << " | Lives left: " << lives << std::endl;
                emit(Event("update_lives", {{"lives", lives}}));

                if (lives <= 0) {
                    emit(Event("die"));
                    emit(Event("game_over"));
                } else {
                    is_invincible = true;
                    invincibility_timer = 1.0;
                }
            }
        });

        notice_collisions_with("crow", [this](Event& e) {
            int target_id = e.value()["id"];
            emit(Event("vanish_crow", {{"id", target_id}}));

            if (current().name() != "Dead" && !is_invincible) {
                lives--;
                std::cout << "[TRex] Collided with Crow ID: " << target_id << " | Lives left: " << lives << std::endl;
                emit(Event("update_lives", {{"lives", lives}}));

                if (lives <= 0) {
                    emit(Event("die"));
                    emit(Event("game_over"));
                } else {
                    is_invincible = true;
                    invincibility_timer = 1.0;
                }
            }
        });

        notice_collisions_with("heart", [this](Event& e) {
            int target_id = e.value()["id"];
            emit(Event("vanish_heart", {{"id", target_id}}));

            if (current().name() != "Dead") {
                if (lives < 3) lives++;
                std::cout << "[TRex] Ate Heart ID: " << target_id << " | Lives left: " << lives << std::endl;
                emit(Event("update_lives", {{"lives", lives}}));
            }
        });

        notice_collisions_with("star", [this](Event& e) {
            int target_id = e.value()["id"];
            emit(Event("vanish_star", {{"id", target_id}}));

            if (current().name() != "Dead") {
                std::cout << "[TRex] Ate Star ID: " << target_id << " | Score x 1.1!" << std::endl;

                /// @brief Broadcasts an event to the Scoreboard to multiply the score by 1.1
                emit(Event("multiply_score"));
            }
        });
    }

    /**
     * @brief Updates the controller state and handles visual effects like invincibility flickering.
     */
    void update() {
        StateMachine::update();

        /// @brief Handles the invincibility flickering effect using CSS opacity
        if (is_invincible) {
            invincibility_timer -= 0.02;
            if (invincibility_timer <= 0) {
                is_invincible = false;
                set_style({{"opacity", "1.0"}}); // 恢復完全不透明
            } else {
                // 每 0.1 秒切換一次透明度，產生閃爍效果
                if ( (int)(invincibility_timer * 10) % 2 == 0 ) {
                    set_style({{"opacity", "0.4"}}); // 半透明
                } else {
                    set_style({{"opacity", "1.0"}}); // 實體
                }
            }
        } else if (current().name() != "Dead") {
            set_style({{"opacity", "1.0"}}); // 確保平時是完全不透明的
        }
    }

    Ready ready;
    Running running;
    Jumping jumping;
    Dead dead;
};

/**
 * @brief The main TRex Agent class encapsulating the state machine controller.
 */
class TRex : public Agent {
public:
    TRex(json spec, World& world) : Agent(spec, world) {
        add_process(c);
        prevent_rotation();
    }
private:
    TRexController c;
};

DECLARE_INTERFACE(TRex)

#endif