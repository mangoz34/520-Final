# Enviro T-Rex Runner Game

## 1. Overview
This is a 2D side-scrolling runner game developed using the **Enviro** and **Elma** frameworks. Inspired by the classic Google Chrome Dino game, the player controls a T-Rex that must jump over approaching cacti. The project demonstrates advanced usage of multi-agent systems, state machine logic, collision optimization, and event-driven life systems.

## 2. Installation

### Prerequisites
* **Docker** must be installed on your system.

### Setup Instructions
1. Clone this repository:
   ```
   git clone https://github.com/mangoz34/520-Final.git
   cd 520-Final-main
   ```
   
2. Start the Enviro Docker container:
   ```
   docker run -p80:80 -p8765:8765 -v $PWD:/source -it klavins/enviro:v1.61 bash
   ```
3. Compile the project inside the container:
   ```
   make
   ```
4. Run the game:
   ```
   enviro
   ```
5. Open the link in your browser:
   ```
   http://localhost
   ```

## 3. How to Play

* **Start the Game**: Press the Space to start running.
* **Jump**: Press the Spacebar while running to jump over the obstacles or collect the items.
* **Restart**: If you run out of lives (hearts) and die, press the Space on the "Game Over" screen to reset the score and play again.

## 4. Features
###  Core Mechanics
* **Endless Survival & Dynamic Difficulty**: The game progressively speeds up to test your reflexes. Every 500 points, a global `speed_up` event is broadcasted, permanently increasing the velocity of all incoming obstacles and items.
* **Day/Night Cycle**: As you hit those 500-point milestones, the environment seamlessly transitions between day and night background.
* **Health System**: T-Rex starts with 3 lives. Taking damage triggers a 1-second invincibility window, accompanied by a flickering effect, preventing unfair instant deaths.

###  Obstacles
* **Cactus (Ground Threat)**: The classic hazard. These spiked plants move swiftly along the ground, requiring perfectly timed jumps to clear.
* **Crow (Aerial Threat)**: A highly dynamic flying enemy. It spawns high in the sky and executes dive-bomb attack when it detects the T-Rex is within range, forcing you to stay grounded or time your jumps carefully.

###  Power-Ups
* **Heart**: A life-saving collectible. Grabbing a heart restores 1 lost life (up to a maximum of 3), which is crucial for surviving the blinding speeds of the later stages.
* **Star**: The score booster! Collecting a star instantly multiplies your current score by 1.1x, helping you snowball your points and reach the next day/night milestone much faster.

## 5. Architecture

The game utilizes a highly decoupled, multi-agent architecture to ensure scalability and performance:

* **T-Rex Agent (State Machine)**: The core player character is governed by a hierarchical State Machine (`Ready`, `Running`, `Jumping`, `Dead`). This ensures strict control over physics and animations, preventing conflicting states (e.g., jumping while already in the air).
* **GameManager (The Director)**: Acts as the invisible orchestrator of the game loop. It utilizes an **Object Pooling** design pattern, listening for `item_cleared` events to randomly deploy recycled obstacles (Cacti, Crows) and power-ups (Stars, Hearts) based on weighted probabilities.
* **Separation of Concerns in UI**: The user interface is split into two independent agents. `ScoreBoard` manages the elapsed time, fractional score multipliers, and global difficulty curves. `ScoreBoard2` is strictly dedicated to listening to `update_lives` events and rendering the player's health.
* **Event-Driven Communication**: All agents communicate asynchronously via Enviro's `emit()` and `watch()` methods, allowing for a highly modular system where obstacles and UI elements respond to global states without tight coupling.

## 6. Key Challenges and Solutions

During development, several technical hurdles were addressed to ensure smooth gameplay:

* **Physics Resistance & X-axis Drift**:
    * **Challenge**: Collisions between the T-Rex and cacti would normally exert a physical force, pushing the T-Rex backward and off the screen.
    * **Solution**: I implemented a strict `teleport(-100, y(), 0)` within the `TRexController`'s `update` loop. This locks the X-coordinate while allowing the physics engine to handle vertical movement (gravity/jumping). I also utilized `ignore_collisions_with` to prevent unwanted physical obstruction.
* **Box2D Convex Polygon Restrictions vs. Retro Aesthetics**:
    * **Challenge**: The underlying Box2D physics engine strictly requires hitboxes to be convex polygons. Attempting to define complex, concave 8-bit shapes directly in the JSON configuration caused physics engine crashes or visual tearing.
    * **Solution**: I decoupled the physics from the rendering. The JSON physical hitboxes were simplified to basic rectangles and made 100% invisible (`rgba(0,0,0,0)`). I then used Enviro's `decorate()` method to inject custom SVG `<rect>` composites directly into the DOM, perfectly overlaying an authentic 8-bit pixel art character over the invisible physics engine.
* **Memory Overhead & Continuous Object Spawning**:
    * **Challenge**: Continuously instantiating and destroying obstacle agents as the game progressed would create massive memory overhead and frame drops.
    * **Solution**: Implemented an **Object Pooling** system. A set number of dynamic entities (Cacti, Crows, Hearts, Stars) are spawned off-screen at initialization. The `GameManager` simply updates their states and teleports them onto the screen when needed, recycling them back to the off-screen pool once they pass the player.
* **Advanced Aerial AI & Physics Glitches**:
    * **Challenge**: Implementing the flying "Crow" enemy that dives at the player. Initial attempts using instant teleportation caused the Crow to clip into the invisible static ceiling boundary, permanently freezing the Box2D physics engine.
    * **Solution**: Replaced coordinate manipulation with pure velocity math. I implemented a proximity-triggered diagonal dive using `track_velocity(vx, vy)` to apply combined velocity vectors. This avoids clipping and creates a smooth, predictable, and highly dynamic attack angle that scales perfectly with the game's speed.
* **Dynamic Difficulty Curve & Global State**:
    * **Challenge**: Scaling the speed of multiple independent enemies without hardcoding values or tightly coupling the classes.
    * **Solution**: Utilized a global event broadcaster. Every 500 points, the `ScoreBoard` emits a `speed_up` event. All moving agents independently listen to this event to increase their `base_vx`. Simultaneously, the `ScoreBoard` uses CSS `transition` injection to smoothly toggle the background color between white and gray, simulating a day/night cycle that acts as visual feedback for the increased difficulty.
* **Invincibility Frames**:
    * **Challenge**: Without a cooldown, a single collision would trigger multiple "hit" events in a split second, depleting all lives instantly.
    * **Solution**: An `is_invincible` state was introduced. Upon collision, the T-Rex enters a 1.5-second invincibility period where further hits are ignored. Visual feedback is provided using an update loop that manipulates CSS `opacity` to create a classic retro flickering effect.
## 7. License
This project is licensed under the **MIT License**. See the [LICENSE](./LICENSE) file for details.

## 8. Reference
* [Enviro](https://github.com/klavinslab/enviro) 
* Google Gemini
