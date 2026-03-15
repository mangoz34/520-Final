#ifndef SCOREBOARD2_H
#define SCOREBOARD2_H

#include "enviro.h"
#include <string>

using namespace enviro;

/**
 * @brief Process controller for the secondary Scoreboard.
 * Dedicated to managing and rendering the player's remaining life count (Hearts).
 * Separating this from the primary scoreboard ensures a cleaner architecture.
 */
class ScoreController2 : public Process, public AgentInterface {
    bool is_playing;
    bool is_game_over;
    int lives;

public:
    /**
     * @brief Constructs a new ScoreController2 and initializes the life tracking variables.
     */
    ScoreController2() : Process(), is_playing(false), is_game_over(false), lives(3) {}

    /**
     * @brief Initializes event listeners for game progression and life updates.
     */
    void init() {
        /**
         * @brief Starts the UI tracking when the game begins.
         */
        watch("keydown", [this](Event& e) {
            std::string key = e.value()["key"];
            if (key == " " && !is_playing && !is_game_over) {
                is_playing = true;
            }
        });

        /**
         * @brief Updates the UI state to freeze upon receiving a game over signal.
         */
        watch("game_over", [this](Event& e) {
            if (!is_game_over) {
                is_game_over = true;
            }
        });

        /**
         * @brief Resets the UI state and replenishes the life count for a new game session.
         */
        watch("game_restart", [this](Event& e) {
            is_game_over = false;
            is_playing = false;
            lives = 3;
        });

        /**
         * @brief Listens for the life update event emitted by the T-Rex after taking damage or healing.
         */
        watch("update_lives", [this](Event& e) {
            lives = e.value()["lives"];
        });
    }

    void start() {}

    /**
     * @brief Core update loop for UI rendering.
     * Displays the game over message, start prompt, or current life count depending on the game state.
     */
    void update() {
        /// @brief State 3: Game Over. Displays the restart prompt.
        if (is_game_over) {
            label("GAME OVER!! PRESS SPACE TO RESTART", -150, -5);
            return;
        }

        /// @brief State 1: Waiting to start. Displays the start prompt.
        if (!is_playing) {
            label("PRESS SPACE TO START", -100, -5);
            return;
        }

        /// @brief State 2: Game in progress. Displays the remaining lives as heart symbols.
        label("♥ : " + std::to_string(lives), -35, -5);
    }

    void stop() {}
};

/**
 * @brief The secondary ScoreBoard agent responsible for rendering the life count interface.
 */
class ScoreBoard2 : public Agent {
public:
    /**
     * @brief Constructs a new ScoreBoard2 agent.
     * @param spec JSON specification for the agent.
     * @param world Reference to the Enviro world.
     */
    ScoreBoard2(json spec, World& world) : Agent(spec, world) {
        add_process(c);
        prevent_rotation();
    }
private:
    ScoreController2 c;
};

DECLARE_INTERFACE(ScoreBoard2)

#endif