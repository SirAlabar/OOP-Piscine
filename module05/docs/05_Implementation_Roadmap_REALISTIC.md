# 🗺️ RAILWAY SIMULATION - REALISTIC ROADMAP
*8 weeks (mandatory) + remaining time (bonus)*

---

## 📍 PHASE 0: Foundation & Setup
**Duration:** Already complete
**Status:** Design docs ready

✅ Requirements analysis
✅ UML diagrams  
✅ Physics design
✅ Architecture decisions
✅ Design patterns mapping

---

## 📍 PHASE 1: Core Entities (Week 1-2)
**Goal:** Fundamental structures working

### Week 1: Graph Structure
```cpp
Priority 1 - Build Network Foundation:
□ Node.hpp/cpp
  - string name
  - Basic getters
  - Name validation

□ Rail.hpp/cpp  
  - Node* startNode, endNode
  - double length, speedLimit
  - Train* occupiedBy (for collision)
  - bool isOccupied()

□ Graph.hpp/cpp
  - vector<Node*> nodes
  - vector<Rail*> rails
  - addNode(Node*)
  - addRail(Rail*)
  - getNode(name) → Node*
  - Adjacency structure (map<Node*, vector<Rail*>>)

✅ Test: Create simple network (3 nodes, 2 rails)
```

### Week 2: Train Entity
```cpp
Priority 2 - Train Class:
□ Train.hpp/cpp
  - string name, int id
  - double mass, frictionCoef
  - double maxAccelForce, maxBrakeForce
  - double velocity, position
  - vector<Rail*> path
  - ITrainState* currentState (pointer only)

✅ Test: Create train, set properties, validate values
⚠️ No state logic yet - just data structure
```

**Milestone 1:** Core entities compile, unit tests pass (>10 tests)

---

## 📍 PHASE 2: Patterns Layer (Week 3-4)
**Goal:** Implement 3 minimum patterns

### Week 3: Factory + State Pattern ONLY
```cpp
Priority 3 - Factories (2-3 days):
□ TrainFactory.hpp/cpp
  - createTrain(TrainConfig) → Train*
  - Validation (mass > 0, stations exist)
  - Unique ID generation
  - ⚠️ Debug: Print created trains

□ NodeFactory.hpp/cpp (simple)
  - createNode(name) → Node*
  - Validation (unique name)

Priority 4 - State Pattern (3-4 days):
□ ITrainState.hpp (interface)
  - virtual void update(Train*, double dt) = 0
  - virtual string getName() = 0

□ Implement ONLY 3 states:
  □ IdleState.cpp - v = 0, do nothing
  □ AcceleratingState.cpp - v += 10 m/s (FAKE increment, no physics)
  □ StoppedState.cpp - v = 0, countdown timer

□ Train.cpp - add:
  - setState(ITrainState* newState)
  - update(dt) → currentState->update(this, dt)
  - ⚠️ Transitions HARDCODED for now:
    * After 10 ticks → switch state manually in main()

✅ Test: Train transitions Idle → Accelerating → Stopped
⚠️ Reality check: Debug state transitions (2-3 days)
⚠️ Memory management (who owns state objects?) takes time

Buffer: If ahead of schedule, start PathFinder interface
```

### Week 4: Pathfinding System
```cpp
Priority 5 - Dijkstra Implementation (4-5 days):
□ DijkstraStrategy.cpp
  - Implement Dijkstra with cost = distance/speedLimit
  - Use priority_queue<pair<double, Node*>>
  - Return vector<Rail*>
  - ⚠️ Debug: Print path step-by-step

□ IPathfindingStrategy.hpp (interface)
  - virtual Path findPath(Graph*, Node* start, Node* end) = 0

□ PathFinder.hpp/cpp (wrapper - if time)
  - setStrategy(IPathfindingStrategy*)
  - findPath(start, end) → delegates to strategy

✅ Test: Find path A→C via B (3 nodes)
✅ Test: Find shortest when multiple routes exist
⚠️ Reality check: Dijkstra bugs (wrong cost, infinite loop) take 1-2 days

Buffer Strategy:
  - If Week 4 explodes, skip PathFinder wrapper
  - Use DijkstraStrategy directly
  - Add Strategy pattern layer later if time
```

**Milestone 2:** Factory creates trains, State pattern works, Dijkstra finds paths

---

## 📍 PHASE 3: Physics & Simulation (Week 5-6)
**Goal:** Simulation works

### Week 5: Physics System (SINGLE TRAIN ONLY)
```cpp
Priority 6 - Physics Engine (5-6 days):
□ PhysicsSystem.hpp/cpp
  - calculateFriction(Train*) → double
  - calculateNetForce(Train*, currentState) → double
  - updateVelocity(Train*, force, dt)
  - updatePosition(Train*, dt)
  - calculateBrakingDistance(Train*) → double

⚠️ CRITICAL: Validate calculations MANUALLY FIRST
  Paper calculation for 80 tons, μ=0.005, F_accel=356kN:
  - F_friction = 3,920 N
  - a = 4.40 m/s²
  - Time to 250 km/h = ~16 seconds
  - Braking distance = 383m
  
  Code output MUST match these numbers!

□ Implement remaining 3 states (2-3 days):
  □ CruisingState.cpp - maintain speed limit
  □ BrakingState.cpp - apply F_brake, check v==0
  □ WaitingState.cpp - simple decel (no blocking logic yet)

□ Train state transitions:
  - Accelerating: if (v >= speedLimit) → Cruising
  - Cruising: if (needBrake()) → Braking
  - Braking: if (v == 0) → Stopped
  - ⚠️ Transition logic is TRICKY - test each separately

✅ Test: SINGLE train, A→B, accelerates/cruises/brakes/stops
✅ Verify: Braking distance = 383m ± 5m
✅ Verify: Train stops EXACTLY at destination (error < 1m)

⚠️ Reality check: Physics bugs take 3-4 days minimum
⚠️ Unit conversion errors (km/h vs m/s) are VERY common
⚠️ Floating point precision issues

NO SIMULATION MANAGER YET
Test everything in main() with single train
```

### Week 6: Multi-Train + Collision (5-6 days)
```cpp
Priority 7 - Simulation Manager (3 days):
□ SimulationManager.hpp/cpp (Singleton)
  - static getInstance()
  - vector<Train*> trains
  - Graph* network
  - double currentTime
  - addTrain(Train*)
  - update(double dt):
    * For each train: train->update(dt)
    * PhysicsSystem::update(trains, dt)
    * ⚠️ CRITICAL: Update physics AFTER state update

Priority 8 - Collision Avoidance (3 days):
□ CollisionAvoidance.hpp/cpp
  - checkCollisions(trains, rails)
  - Mark rails as occupied:
    * rail->setOccupiedBy(train) when enters
    * rail->clearOccupied() when exits
  - Before train enters new rail:
    * if (nextRail->isOccupied()) → setState(WaitingState)

□ WaitingState logic:
  - Check every timestep if rail cleared
  - If cleared: setState(AcceleratingState)

✅ Test: 2 trains, same path, T1@10h00, T2@10h05
  - T2 should WAIT before entering occupied rail
  - T2 should RESUME when T1 exits
  - Verify NO collision (never same position)

✅ Test: 3 trains, different paths (independent movement)

⚠️ Reality check: Collision bugs take 2-3 days
⚠️ Edge case: Train at rail boundary (position == rail.length)
⚠️ Edge case: Both trains want same rail simultaneously

Buffer Strategy:
  - If Week 6 explodes: Implement SIMPLE blocking
    * Entire rail blocked (no partial occupation)
    * Train waits at previous node
    * Add fine-grained blocking later
```

**Milestone 3:** Physics correct, multi-train works, NO collisions

---

## 📍 PHASE 4: IO System (Week 7 + partial Week 8)
**Goal:** Read inputs, write outputs

### Week 7: Input Parsing (3-4 days)
```cpp
Priority 9 - File Parsers:
□ RailNetworkParser.hpp/cpp (2 days)
  - parseFile(filename) → Graph*
  - Parse "Node <name>"
  - Parse "Rail <A> <B> <length> <speedLimit>"
  - Error handling:
    * Line numbers
    * Clear error messages
    * Format: "Error: [msg]\nLine: X\nContent: [line]"
  
⚠️ Validate BEFORE building graph:
  - Node names unique?
  - Rails connect existing nodes?
  - Positive values?
  - Handle empty lines, comments?

□ TrainConfigParser.hpp/cpp (2 days)
  - parseFile(filename) → vector<TrainConfig>
  - Parse 9 space-separated fields
  - Validate:
    * Stations exist in Graph
    * Path exists (call PathFinder)
    * All positive values
    * Time format correct (HHhMM)

✅ Test: Parse examples/network_simple.txt, trains_simple.txt
⚠️ Reality check: Parsing bugs take 1+ day
  - Whitespace handling
  - Empty lines
  - EOF detection
  - String splitting edge cases
```

### Week 7-8: Output System (4-5 days)
```cpp
Priority 10 - Output Generation:
⚠️ WARNING: This is harder than it looks!

□ OutputWriter.hpp/cpp (1 day)
  - openFile(trainName_departureTime.result)
  - writeHeader(trainName, estimatedTime)
  - writeSnapshot(time, train, status, visualization)
  - closeFile()

□ Rail Visualization (2-3 days - TRICKY):
  Algorithm:
  1. Get current rail, train position (meters)
  2. cellCount = (int)rail->getLength()  // 1 cell per km
  3. trainCell = (int)(position / 1000.0)
  4. Build string: "[ ][ ][ ]...[x]...[ ][ ]"
  5. If other train on rail: add [O] at its position
  
  Edge cases to handle:
  - Rail < 1km → show at least 1 cell
  - Train at boundary (position == rail.length) → which cell?
  - Two trains same cell → show both [x][O] or [O][x]?
  - Rounding: (int) vs floor() vs ceil()?

□ Status Mapping (30 min):
  - IdleState → "Speed up" (match PDF, even if wrong)
  - AcceleratingState → "Speed up"
  - CruisingState → "Maintain"
  - BrakingState → "Braking"
  - StoppedState → "Stopped"
  - WaitingState → "Waiting"

□ Snapshot Timing (1 day):
  - Every 5 minutes: if (time % 300 == 0)
  - Distance remaining calculation
  - Last line: distance=0.00km, Stopped

✅ Test: Output file matches PDF format line-by-line
⚠️ Reality check: Visualization bugs take 2-3 days
⚠️ Off-by-one errors are EXTREMELY common
⚠️ String formatting in C++ is annoying

Buffer Strategy (IMPORTANT):
  If output is exploding time:
  - Implement SIMPLE output first (no visualization)
  - Just text: time, status, distance
  - Add ASCII visualization later if time permits
```

**Milestone 4:** Parse files → run simulation → generate output

---

## 📍 PHASE 5: Events + CLI (Week 8 finish)
**Goal:** Mandatory complete

### Week 8 (remaining time): Events + Polish
```cpp
Priority 11 - Observer Pattern (2-3 days):
⚠️ Implement MINIMAL event system only

□ IObserver.hpp
  - virtual void onNotify(Event*) = 0

□ EventManager.hpp/cpp (Singleton)
  - vector<IObserver*> observers
  - attach(IObserver*)
  - notify(Event*)

□ Event.hpp (abstract)
  - Time startTime, endTime
  - virtual apply() = 0
  - virtual revert() = 0 (optional)

□ Implement 2 event types (NOT 3):
  □ StationDelayEvent - add time to stop_duration
  □ TrackMaintenanceEvent - reduce rail speedLimit
  

□ EventFactory.cpp
  - createRandomEvent() → Event*
  - Use SeededRNG

□ Train implements IObserver:
  - onNotify(Event* e)
    if (e->affects(this)) → adjust behavior

□ SeededRNG.hpp/cpp (1 day)
  - Wrapper for <random>
  - Constructor takes seed

Priority 13 - Final Testing (1-2 days):
□ Test Scenario 1: Simple journey
□ Test Scenario 2: Collision avoidance
□ Test Scenario 3: Event impact (if events done)
□ Memory leak check: valgrind
□ Verify all outputs match spec

✅ All MR-001 through MR-010 done
✅ 3+ patterns implemented (Factory, State, Strategy minimum)
✅ Observer if events done
✅ Singleton (SimulationManager)
✅ UML diagrams match code

```

**Milestone 5 (EVALUATION READY):** Mandatory 100% functional

---

## 📍 PHASE 6: Bonus Features (if time remains)
**Priority order by impact/ease:**

### Bonus 1: Deterministic Mode (EASY - 2 days)
```cpp
□ --seed=N flag parsing
□ Pass seed to SeededRNG
□ Log seed at startup
□ Verify: same seed = identical output

✅ High value, low effort
```

### Bonus 2: A* Pathfinding (EASY - 2-3 days)
```cpp
□ AStarStrategy.cpp
  - Heuristic: straight-line / avg_speed
□ Add to Strategy pattern
□ --pathfinding=astar flag

✅ Easy if Strategy pattern done right
```

### Bonus 3: SFML Visualization (MEDIUM - 1 week)
```cpp
□ IRenderer.hpp interface
□ SFMLRenderer.cpp
  - Draw nodes, rails, trains
  - Color by state
  - Camera pan/zoom
  - Pause/resume
□ --render flag

✅ High impact for demo, takes time
```

### Bonus 4: Hot Reload (MEDIUM - 4-5 days)
```cpp
□ FileWatcher.hpp
□ Re-parse on file change
□ Command Pattern for changes
□ --hot-reload flag

⚠️ Complex, lower priority
```

### Bonus 5: Monte Carlo (MEDIUM-HIGH - 3-4 days)
```cpp
□ MonteCarloRunner.cpp
□ StatsCollector.cpp
□ Run N times, output CSV
□ --monte-carlo=N flag

✅ Good for analysis, 
```

### Bonus 6: Replay System (HARD - 1 week)
```cpp
□ CommandManager records actions
□ Save to JSON
□ Replay player
□ --record/--replay flags

⚠️ Complex, only if time abundant
```

---

## Modeling Assumptions

### Rail Directionality
Rails are modeled as single-track physical segments. Trains may travel in both directions, but only one train can occupy a rail at a time.

### Node Semantics
All nodes are represented by a single Node class. An optional NodeType enumeration differentiates stations (City) from junctions.

### Train Identification
Train IDs are sequential deterministic integers assigned in input order to ensure reproducibility.



## 🎯 REALISTIC TIMELINE

```
Week 1:   Core entities (Node, Rail)
Week 2:   Train entity
Week 3:   Factory + State (3 states only)
Week 4:   Dijkstra + Strategy wrapper
Week 5:   Physics (single train, ALL states)
Week 6:   Simulation Manager + Collision
Week 7:   Input parsing
Week 8:   Output + Events (minimal) + CLI + Testing

= MANDATORY DONE (with buffer)

Remaining time: Bonus by priority
```

---

## ✅ DEFINITION OF READY FOR EVALUATION

**Minimum Viable Product:**
- [ ] Trains move with physics
- [ ] Multiple trains don't collide
- [ ] Parses input files
- [ ] Generates output files
- [ ] 3 design patterns working
- [ ] 2 UML diagrams provided
- [ ] No memory leaks
- [ ] Compiles without warnings

**Nice to Have:**
- [ ] Event system
- [ ] Pretty ASCII visualization
- [ ] All 6 states implemented
- [ ] Full error handling

---

## 🎓 EVALUATION DAY PREP

**1 Week Before:**
- [ ] All core features working
- [ ] Practice demo (5 min presentation)
- [ ] Review UML diagrams
- [ ] Prepare pattern explanations
- [ ] List known limitations

**Defense:**
- [ ] Demo live (simple scenario)
- [ ] Show physics calculations
- [ ] Explain state transitions
- [ ] Walk through collision avoidance
- [ ] Show output file

**Difficult Questions:**
- "Why Singleton?" → Explain global coordination need
- "Why State Pattern?" → Avoid giant if/else, cleaner
- "Limitations?" → Be honest (e.g. "no train reversal")

---

## Domain Modeling Decisions

### Rail Directionality
Rails are modeled as single-track bidirectional segments.  
Only one train can occupy a rail at a time to simplify collision detection and ensure safety.

### Node Semantics
A single Node class is used.  
Node roles (City vs Junction) are represented using a NodeType enumeration instead of subclassing.

### Train Identification
Train IDs are sequential deterministic integers assigned in input order to guarantee reproducibility.


**Reality: Most projects take 10-12 weeks for perfection.**
**Goal: Have evaluation-ready product in 8 weeks.**
**Strategy: Prioritize core, simplify when needed, add polish if time.**

railway-simulation/
├── CMakeLists.txt
├── README.md
├── DESIGN.md                 # Architecture documentation
docs/
├── uml/
│   ├── Sequence_Diagram.md
│   ├── State_Machine.md
├── 00_Requirements_Analysis_Document.md
├── 01_Design_Patterns_Mapping.md
├── 02_UML_Diagrams.md
├── 03_Physics_Design_Document.md
├── 04_Architecture_Decision_Record.md
├── 05_Implementation_Roadmap_REALISTIC.md
└── visualGoal.jpg
├── include/
│   ├── core/
│   │   ├── Graph.hpp
│   │   ├── Node.hpp
│   │   ├── Rail.hpp
│   │   └── Train.hpp
│   ├── simulation/
│   │   ├── SimulationManager.hpp  # Singleton
│   │   ├── PhysicsSystem.hpp
│   │   └── CollisionAvoidance.hpp
│   ├── patterns/
│   │   ├── factories/
│   │   │   ├── TrainFactory.hpp
│   │   │   └── EventFactory.hpp
│   │   ├── strategies/
│   │   │   ├── IPathfindingStrategy.hpp
│   │   │   ├── DijkstraStrategy.hpp
│   │   │   └── AStarStrategy.hpp
│   │   ├── states/
│   │   │   ├── ITrainState.hpp
│   │   │   ├── IdleState.hpp
│   │   │   ├── AcceleratingState.hpp
│   │   │   ├── CruisingState.hpp
│   │   │   ├── BrakingState.hpp
│   │   │   ├── StoppedState.hpp
│   │   │   └── WaitingState.hpp
│   │   ├── observers/
│   │   │   ├── IObserver.hpp
│   │   │   ├── ISubject.hpp
│   │   │   └── EventManager.hpp
│   │   └── commands/
│   │       ├── ICommand.hpp
│   │       ├── CommandManager.hpp
│   │       └── [specific commands]
│   ├── io/
│   │   ├── FileParser.hpp
│   │   ├── RailNetworkParser.hpp
│   │   ├── TrainConfigParser.hpp
│   │   └── OutputWriter.hpp
│   ├── rendering/
│   │   ├── IRenderer.hpp
│   │   ├── SFMLRenderer.hpp
│   │   ├── NodeRenderer.hpp
│   │   ├── RailRenderer.hpp
│   │   └── TrainRenderer.hpp
│   ├── analysis/
│   │   ├── MonteCarloRunner.hpp
│   │   └── StatsCollector.hpp
│   └── utils/
│       ├── SeededRNG.hpp
│       ├── FileWatcher.hpp
│       └── Logger.hpp
├── src/
│   └── [corresponding .cpp files]
├── tests/
│   ├── unit/
│   │   ├── test_graph.cpp
│   │   ├── test_train.cpp
│   │   ├── test_physics.cpp
│   │   └── ...
│   └── integration/
│       └── test_simulation.cpp
├── examples/
│   ├── network_simple.txt
│   ├── trains_simple.txt
│   ├── network_complex.txt
│   └── trains_complex.txt
└── tools/
    ├── plot_stats.py        # Monte Carlo visualization
    └── replay_analyzer.py   # Replay file analysis