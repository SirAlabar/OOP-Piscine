# 🚆 Object-Oriented Programming Piscine

Complete implementation of the Object-Oriented Programming Piscine,
including all mandatory and bonus exercises.\
The project progresses from core OOP principles to a complete railway
simulation engine with realistic physics, procedural world generation,
and hot reload support.

------------------------------------------------------------------------

# 🎬 Demo

![Simulation Demo](https://github.com/SirAlabar/OOP-Piscine/releases/download/v1.0/train_simulator.gif)

------------------------------------------------------------------------

# 📋 Table of Contents

-   Overview
-   Project Structure
-   Modules
-   Module 05 --- Railway Simulation Engine
-   Advanced Features
-   Design Patterns
-   Compilation
-   Usage
-   Documentation

------------------------------------------------------------------------

# 🎯 Overview

This project is a comprehensive Object-Oriented Programming curriculum
divided into 6 modules.

Core concepts covered:

-   Encapsulation
-   Object relationships
-   UML modeling
-   SOLID principles
-   Design Patterns
-   Simulation engine architecture
-   Physics-based modeling
-   Procedural world generation
-   Deterministic simulation
-   Hot reload architecture

------------------------------------------------------------------------

# 📁 Project Structure

    opp-piscine/
    ├── module00/          # Encapsulation
    │   ├── ex00/         # Banking system
    │   └── ex01/         # Graph visualization
    ├── module01/          # Relationships
    │   └── ex00/         # Worker-Tool-Workshop system
    ├── module02/          # UML Diagrams
    │   └── ex00/         # Car system diagrams
    ├── module03/          # SOLID Principles
    │   ├── ex00/         # [S]ingle Responsibility
    │   ├── ex01/         # [O]pen/Closed
    │   ├── ex02/         # [L]iskov Substitution
    │   ├── ex03/         # [I]nterface Segregation
    │   └── ex04/         # [D]ependency Inversion
    ├── module04/          # Design Patterns
    │   ├── ex00/         # Preparation (base classes)
    │   ├── ex01/         # Singleton
    │   ├── ex02/         # Factory & Command
    │   ├── ex03/         # Mediator
    │   ├── ex04/         # Observer
    │   └── ex05/         # Facade
    └── module05/          # Railway Simulation
        ├── docs/          # Design/architecture documents
        │   └── uml/       # UML diagrams
        ├── include/       # Header files
        ├── src/           # Source files
        ├── tests/         # Unit and integration tests
        └── examples/      # Simulation input examples

------------------------------------------------------------------------

# 🚆 Module 05 --- Railway Simulation Engine

Core features:

-   Multi-train simulation
-   Collision avoidance system
-   Deterministic simulation
-   Event-driven architecture
-   Pathfinding system
-   Procedural world generation
-   Real-time rendering
-   Hot reload support
-   Replay recording and playback
-   Monte Carlo analysis mode
-   Round-trip train mode

------------------------------------------------------------------------

# ⚙️ Physics Engine

The simulation includes a fully implemented physics engine that models realistic train movement based on physical properties and environmental constraints.

Key features:

- Realistic acceleration based on train mass and traction force
- Braking system that allows trains to slow down and stop smoothly at stations or when blocked
- Friction and resistance simulation affecting train speed over time
- Speed limits enforced per rail segment
- Continuous position and velocity updates during simulation
- State-dependent behavior (accelerating, cruising, braking, waiting, stopped)
- Deterministic timestep system ensuring consistent and reproducible results

The physics engine ensures that train movement behaves realistically while remaining stable, deterministic, and suitable for real-time simulation.


------------------------------------------------------------------------

# 🌍 Procedural World Generation

The simulation includes a procedural world generation system used to create dynamic and non-repetitive environments for rendering.

Key features:

- Biomes generated based on temperature distribution across the map
- Terrain refinement using neighborhood analysis for smoother transitions
- Deterministic generation using a seed, ensuring reproducible worlds
- Fully dynamic terrain creation without requiring predefined maps

This system allows the simulation to render unique environments while remaining consistent and predictable.

---

# 🎨 Rendering Engine

The project includes a real-time rendering engine built using SFML, responsible for visualizing the simulation and interacting with the generated world.

Key features:

- Real-time visualization of train movement across the railway network
- Integration with the procedural terrain generation system
- Camera system with full user input support for navigation
- Smooth zoom in and zoom out using linear interpolation for stable visual transitions
- Input handling for camera movement and simulation interaction
- Clear graphical representation of the simulation state and environment

The rendering engine is fully decoupled from the simulation logic, ensuring modularity, maintainability, and clean separation between visualization and core simulation systems.

---

# 🔥 Hot Reload System

The hot reload system allows simulation data to be reloaded at runtime without restarting the program.

Key features:

- Detects and reloads updated network or train configuration files
- Applies changes safely without breaking the current simulation
- Enables faster testing and iteration during development
- Reduces downtime when adjusting simulation parameters

This significantly improves development workflow and debugging efficiency.

---

# 🔔 Event System

The simulation uses an event-driven architecture to allow dynamic interaction between system components.

Key features:

- Centralized EventManager responsible for event distribution
- Trains and rails subscribe to events and react accordingly
- Enables dynamic changes during simulation
- Decouples systems for better modularity and extensibility

This architecture ensures flexible and scalable system behavior.


---

# 🧪 Advanced Runtime Modes

Additional features implemented after the core simulator:

-   **Replay system (Command pattern):** record simulation commands with `--record` and replay with `--replay=<file>`.
-   **Monte Carlo analysis:** run repeated deterministic simulations using `--monte-carlo=N` for statistical validation.
-   **Round-trip mode:** trains automatically reverse direction at destination with `--round-trip`.
-   **Pathfinding switch:** choose algorithm at runtime with `--pathfinding=dijkstra|astar`.

---

# 🧠 Design Patterns Used

The project makes extensive use of established software design patterns to ensure clean architecture and extensibility.

Implemented patterns:

- Factory Pattern for controlled object creation
- State Pattern for managing train behavior
- Strategy Pattern for pathfinding algorithms
- Observer Pattern for event handling
- Command Pattern for runtime operations such as hot reload
- Singleton Pattern for centralized simulation management

These patterns improve modularity, maintainability, and scalability of the simulation engine.


------------------------------------------------------------------------

# 🔨 Compilation

c++ -Wall -Wextra -Werror

make

------------------------------------------------------------------------

# ▶ Usage

cd module05\
make\
./railway_sim examples/network_simple.txt examples/trains_simple.txt

Hot reload:

./railway_sim examples/network_simple.txt examples/trains_simple.txt --hot-reload

Replay record / replay:

./railway_sim examples/network_simple.txt examples/trains_simple.txt --record
./railway_sim examples/network_simple.txt examples/trains_simple.txt --replay=output/replay.json

Monte Carlo:

./railway_sim examples/network_simple.txt examples/trains_simple.txt --monte-carlo=100

------------------------------------------------------------------------

# 📁 Documentation

module05/docs/

------------------------------------------------------------------------

# 🌟 Highlights

-   Simulation engine architecture
-   Procedural generation
-   Physics simulation
-   Design patterns
-   Hot reload system
-   Replay recording and playback
-   Monte Carlo analysis mode
