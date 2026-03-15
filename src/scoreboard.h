#ifndef SCOREBOARD_H
#define SCOREBOARD_H

#include "enviro.h"
#include <string>
#include <chrono>

using namespace enviro;

/**
 * @brief Process controller for the primary Scoreboard.
 * Manages game time, scores, global speed-up events, and background visual effects.
 */
class ScoreController : public Process, public AgentInterface {
    double score_ticks;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    bool is_playing;
    bool is_game_over;

    int final_time;
    int final_score;
    int next_speed_threshold;

    /// @brief Flag to track the current background theme (white/gray toggle).
    bool is_dark_mode;

public:
    /**
     * @brief Constructs a new ScoreController and initializes tracking variables.
     */
    ScoreController() : Process(), score_ticks(0.0), is_playing(false), is_game_over(false), final_time(0), final_score(0), next_speed_threshold(500), is_dark_mode(false) {}

    /**
     * @brief Initializes event listeners for game progression, scoring, and visual effects.
     */
    void init() {
        watch("keydown", [this](Event& e) {
            std::string key = e.value()["key"];
            if (key == " " && !is_playing && !is_game_over) {
                is_playing = true;
                start_time = std::chrono::high_resolution_clock::now();

                /// @brief Ensures the background is white when the game starts,
                /// adding a 1-second smooth CSS transition.
                decorate("<style>body { background-color: white; transition: background-color 1s ease; }</style>");
            }
        });

        watch("game_over", [this](Event& e) {
            if (!is_game_over) {
                is_game_over = true;
                auto current_time = std::chrono::high_resolution_clock::now();
                final_time = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();
                final_score = (int)score_ticks;
            }
        });

        watch("game_restart", [this](Event& e) {
            is_game_over = false;
            is_playing = false;
            score_ticks = 0.0;
            final_time = 0;
            final_score = 0;
            next_speed_threshold = 500;
            is_dark_mode = false;

            /// @brief Resets the background to white when restarting the game.
            decorate("<style>body { background-color: white; transition: background-color 1s ease; }</style>");
        });

        watch("multiply_score", [this](Event& e) {
            if (is_playing) {
                score_ticks *= 1.1;
            }
        });
    }

    void start() {}

    /**
     * @brief Core update loop for UI rendering and difficulty curve progression.
     */
    void update() {
        if (is_game_over) {
            std::string text = "Time: " + std::to_string(final_time) + "s   |   Score: " + std::to_string(final_score);
            label(text, -85, -30);
            return;
        }

        if (!is_playing) {
            std::string text = "Time: 0s   |   Score: 0";
            label(text, -85, -30);
            return;
        }

        score_ticks += 1.0;

        /**
         * @brief Difficulty Curve & Visual Feedback Mechanic.
         * Every 500 points, it emits a speed_up event and toggles the background color
         * between white and gray to simulate a "day/night" progression cycle.
         */
        while (score_ticks >= next_speed_threshold) {
            emit(Event("speed_up"));
            next_speed_threshold += 500;

            is_dark_mode = !is_dark_mode;
            if (is_dark_mode) {
                /// @brief Switches to a cool gray background.
                decorate("<style>body { background-color: #dddddd; transition: background-color 1s ease; }</style>");
            } else {
                /// @brief Switches back to the white background.
                decorate("<style>body { background-color: white; transition: background-color 1s ease; }</style>");
            }
        }

        auto current_time = std::chrono::high_resolution_clock::now();
        int time_sec = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();

        std::string display_text = "Time: " + std::to_string(time_sec) + "s   |   Score: " + std::to_string((int)score_ticks);
        label(display_text, -85, -30);
    }

    void stop() {}
};

/**
 * @brief The ScoreBoard agent responsible for global state tracking and UI overlays.
 */
class ScoreBoard : public Agent {
public:
    ScoreBoard(json spec, World& world) : Agent(spec, world) {
        add_process(c);
        prevent_rotation();
    }
private:
    ScoreController c;
};

DECLARE_INTERFACE(ScoreBoard)

#endif