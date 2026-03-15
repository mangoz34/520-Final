#ifndef __CEILING_AGENT__H
#define __CEILING_AGENT__H

#include "enviro.h"

using namespace enviro;

/**
 * @brief Process controller for the invisible Ceiling boundary.
 * This controller requires no active logic (empty update loop) because the ceiling
 * acts purely as a static physical barrier handled natively by the Box2D physics engine.
 */
class ceilingController : public Process, public AgentInterface {

public:
    /**
     * @brief Constructs a new ceilingController.
     */
    ceilingController() : Process(), AgentInterface() {}

    void init() {}
    void start() {}
    void update() {}
    void stop() {}

};

/**
 * @brief The ceiling agent representing the upper invisible boundary of the game world.
 * Prevents the T-Rex or other dynamic entities from flying off the top of the screen.
 */
class ceiling : public Agent {
public:
    /**
     * @brief Constructs a new ceiling agent.
     * @param spec JSON specification for the agent.
     * @param world Reference to the Enviro world.
     */
    ceiling(json spec, World& world) : Agent(spec, world) {
        add_process(c);
    }
private:
    ceilingController c;
};

DECLARE_INTERFACE(ceiling)

#endif