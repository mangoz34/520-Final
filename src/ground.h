#ifndef __GROUND_AGENT__H
#define __GROUND_AGENT__H

#include "enviro.h"

using namespace enviro;

/**
 * @brief Process controller for the static Ground boundary.
 * Requires no active update loop as it serves purely as a static physical foundation
 * managed natively by the Box2D physics engine.
 */
class groundController : public Process, public AgentInterface {

public:
    /**
     * @brief Constructs a new groundController.
     */
    groundController() : Process(), AgentInterface() {}

    void init() {}
    void start() {}
    void update() {}
    void stop() {}

};

/**
 * @brief The ground agent representing the floor of the game world.
 * Provides the solid surface on which the T-Rex runs and lands after jumping.
 */
class ground : public Agent {
public:
    /**
     * @brief Constructs a new ground agent.
     * @param spec JSON specification for the agent.
     * @param world Reference to the Enviro world.
     */
    ground(json spec, World& world) : Agent(spec, world) {
        add_process(c);
    }
private:
    groundController c;
};

DECLARE_INTERFACE(ground)

#endif