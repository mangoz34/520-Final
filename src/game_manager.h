#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include "enviro.h"
#include <stdlib.h>
#include <iostream>

using namespace enviro;

/**
 * @brief Process controller for the GameManager agent.
 * * Acts as the invisible "director" of the game, managing the core gameplay loop.
 * It handles game state transitions (start, game over, restart) and controls
 * the random deployment of obstacles and items from the object pool.
 */
class GameManagerController : public Process, public AgentInterface {
    bool is_playing;
public:
    /**
     * @brief Constructs a new GameManagerController.
     */
    GameManagerController() : Process(), is_playing(false) {}

    /**
     * @brief Initializes event listeners for game progression and item deployment.
     */
    void init() {
        /**
         * @brief Starts the game loop when the spacebar is pressed.
         */
        watch("keydown", [this](Event& e) {
            std::string key = e.value()["key"];
            if (key == " " && !is_playing) {
                is_playing = true;
                std::cout << "[Director] Game Start! Deploying first item." << std::endl;
                emit(Event("deploy_cactus"));
            }
        });

        /**
         * @brief Halts item deployment upon receiving a game over signal.
         */
        watch("game_over", [this](Event& e) {
            std::cout << "[Director] Game Over received. Stopping deployments." << std::endl;
            is_playing = false;
        });

        /**
         * @brief Resets the director's state and initiates the first deployment upon restart.
         */
        watch("game_restart", [this](Event& e) {
            std::cout << "[Director] Game Restart received." << std::endl;
            is_playing = true;
            emit(Event("deploy_cactus"));
        });

        /**
         * @brief Listens for the stage clearing signal and rolls a dice to deploy the next item.
         */
        watch("item_cleared", [this](Event& e) {
            if (!is_playing) return;

            int roll = rand() % 100;
            std::cout << "[Director] Received item_cleared! Rolled dice: " << roll << std::endl;

            /// @note Deployment probabilities: 80% Cactus, 15% Star, 5% Heart
            if (roll < 40) {
                std::cout << "[Director] -> deploy_cactus" << std::endl;
                emit(Event("deploy_cactus"));
            } else if (roll < 65) {
                std::cout << "[Director] -> deploy_crow" << std::endl;
                emit(Event("deploy_crow"));
            } else if (roll < 90) {
                std::cout << "[Director] -> deploy_star" << std::endl;
                emit(Event("deploy_star"));
            } else {
                std::cout << "[Director] -> deploy_heart" << std::endl;
                emit(Event("deploy_heart"));
            }
        });
    }

    void start() {}
    void update() {}
    void stop() {}
};

/**
 * @brief The GameManager agent class.
 * * An invisible, non-colliding agent that runs the GameManagerController
 * to direct the asynchronous event flow of the game.
 */
class GameManager : public Agent {
public:
    /**
     * @brief Constructs a new GameManager agent.
     * @param spec JSON specification for the agent.
     * @param world Reference to the Enviro world.
     */
    GameManager(json spec, World& world) : Agent(spec, world) {
        add_process(c);
        prevent_rotation();
    }
private:
    GameManagerController c;
};

DECLARE_INTERFACE(GameManager)

#endif