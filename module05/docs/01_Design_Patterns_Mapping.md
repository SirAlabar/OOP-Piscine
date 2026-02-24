# DESIGN PATTERNS MAPPING

---

## QUICK REFERENCE

| Pattern     | What It Does                          | Where We Use It              |
|-------------|---------------------------------------|------------------------------|
| **Factory** | Creates objects without specifying exact class | TrainFactory, EventFactory |
| **Observer**| One object notifies many others       | EventDispatcher -> Trains/Rails |
| **Strategy**| Swap algorithms at runtime            | Pathfinding (Dijkstra/A*)    |
| **State**   | Object behavior changes with state    | Train states (Accelerating/Braking/etc) |
| **Command** | Actions as objects (undo/redo/replay) | User commands, hot-reload    |
| **Builder** | Step-by-step object construction      | SimulationBuilder            |

---

## 1. FACTORY PATTERN

**Problem:** Need to create Trains, Events, Nodes from file input with validation.

**Solution:** Factory class handles creation + validation.

```
TrainFactory::create(TrainConfig) -> Train*
  [+] Validates mass > 0
  [+] Checks stations exist in network
  [+] Verifies path exists
  [+] Assigns unique ID
  [+] Returns ready-to-use Train

EventFactory::createRandom() -> Event*
  [+] Picks random event type
  [+] Picks random location
  [+] Sets duration/effects
```

**Classes:**
- `TrainFactory` - creates trains
- `EventFactory` - creates random events
- `NodeFactory` - creates City/RailNode

**Why This Helps:**
- Parser doesn't need to know about Train internals
- Easy to add new train types (FreightTrain, PassengerTrain)
- Validation centralized in one place

**SOLID:** Open/Closed (add new types without changing factory code)

---

## 2. OBSERVER PATTERN

**Problem:** Events affect multiple trains/rails. How do they all get notified?

**Solution:** EventDispatcher keeps list of observers, notifies them all.

```
Event happens -> EventDispatcher.notify(event) -> All observers get onNotify(event)

Observers:
- Train : adjusts speed if TrackMaintenance affects current rail
- Rail  : reduces speed limit if TrackMaintenance event
- Node  : sets signal failure flag
```

**How It Works:**
1. Train registers: `EventDispatcher.attach(train)`
2. Event occurs: `EventDispatcher.notify(event)`
3. Each observer checks: "Does this affect me?"
4. If yes, apply effect

**Classes:**
- `IObserver` interface - has `onNotify(event)` method
- `EventDispatcher` - keeps list, calls notify
- `Train/Rail/Node` - implement IObserver

**Why This Helps:**
- EventDispatcher doesn't need to know about Trains
- Easy to add new observers (TrafficLight, WeatherStation)
- Decoupled: events don't know who's listening

**SOLID:** Dependency Inversion (depends on interface, not concrete classes)

---

## 3. STRATEGY PATTERN

**Problem:** Need different pathfinding algorithms (Dijkstra, A*).

**Solution:** PathFinder uses interchangeable strategy.

```
PathFinder
  +-- uses IPathfindingStrategy
      |-- DijkstraStrategy (mandatory)
      +-- AStarStrategy (bonus)

pathFinder.setStrategy(new DijkstraStrategy());
path = pathFinder.findPath(start, end);

// Later: switch algorithm
pathFinder.setStrategy(new AStarStrategy());
```

**Classes:**
- `IPathfindingStrategy` interface - has `findPath()`
- `DijkstraStrategy` - implements Dijkstra
- `AStarStrategy` - implements A* (bonus)
- `PathFinder` - uses strategy

**Why This Helps:**
- Swap algorithms without changing Train code
- Easy to add new algorithms (BidirectionalSearch)
- Can benchmark different approaches

**SOLID:** Open/Closed (add algorithms without modifying PathFinder)

---

## 4. STATE PATTERN

**Problem:** Train behavior changes drastically based on state. Avoid giant if/else.

**Solution:** Each state is a class with its own `update()` logic.

```
[ Idle ] --departure--> [ Accelerating ] --reached speed--> [ Cruising ]
                               |                                  |
                          rail occupied                      need brake
                               |                                  |
                               v                                  v
                          [ Waiting ]                        [ Braking ]
                                                                  |
                                                                 v=0
                                                                  v
                                                            [ Stopped ]

Each state handles its own physics/logic:
- AcceleratingState : apply max force
- CruisingState     : maintain speed limit
- BrakingState      : apply brake force
- WaitingState      : gentle stop if blocked
- StoppedState      : wait for duration
```

**Classes:**
- `ITrainState` interface - has `update(train, dt)`
- `IdleState, AcceleratingState, CruisingState, BrakingState, StoppedState, WaitingState`
- `Train` - has current_state, delegates update

**Why This Helps:**
- No giant switch statement in Train.update()
- Each state's logic is isolated
- Easy to add states (EmergencyState)

**SOLID:** Single Responsibility (each state = one class, one job)

---

## 5. COMMAND PATTERN

**Problem:** Need undo/redo, replay system, hot-reload.

**Solution:** Actions are objects that can be executed/undone/recorded.

```
Commands:
- TrainDepartureCommand
- TrainStateChangeCommand
- TrainAdvanceRailCommand
- SimEventCommand
- ReloadCommand

commandManager.execute(new TrainDepartureCommand(...));
commandManager.undo();
commandManager.redo();

For replay:
commandManager.record(time, command);
commandManager.saveToFile("replay.json");
```

**Classes:**
- `ICommand` interface - has `execute()`, `undo()`, `applyReplay()`
- Concrete commands for each action
- `CommandManager` - history stack, undo/redo, file save/load

**Why This Helps:**
- Actions can be saved for replay
- Hot-reload recorded as ReloadCommand
- Decoupled from SimulationManager

**SOLID:** Single Responsibility (each command = one action)

---

## 6. BUILDER PATTERN

**Problem:** SimulationManager requires a fully configured network, trains, writers,
and seed before it can run. Constructing all of this inline in Application
would be a god-function.

**Solution:** SimulationBuilder assembles the simulation step by step and returns
a ready-to-use SimulationBundle.

```
SimulationBuilder::build(netFile, trainFile) -> SimulationBundle
  [+] Parses network file  -> Graph
  [+] Parses train file    -> TrainConfigs
  [+] Validates each train against the network
  [+] Creates FileOutputWriters per train
  [+] Returns SimulationBundle { graph, trains, writers }
```

**Classes:**
- `SimulationBuilder` - orchestrates parsing, validation, and wiring
- `SimulationBundle`  - plain data struct: output of the build step

**Why This Helps:**
- Application never calls `new` directly for simulation objects
- SimulationManager receives a fully valid bundle -- no partial state
- Easy to test builder in isolation

**SOLID:** Single Responsibility (building is separate from running)

---

## 7. HOW PATTERNS WORK TOGETHER

```
STARTUP:
1. Factory creates Graph, Trains from files
2. SimulationBuilder wires graph, trains, writers into SimulationBundle
3. Trains register with EventDispatcher (Observer)

SIMULATION LOOP:
1. Each Train updates (State pattern)
2. Train finds path (Strategy pattern)
3. EventFactory creates random events (Factory)
4. EventDispatcher notifies observers (Observer)

USER ACTIONS:
1. User triggers action
2. CommandManager executes (Command)
3. Command recorded for replay
```

**Example: Train Journey**
```
1. TrainFactory creates train (Factory)
2. Train finds path using DijkstraStrategy (Strategy)
3. Train starts in IdleState (State)
4. At departure time -> AcceleratingState (State)
5. Event occurs -> EventDispatcher notifies train (Observer)
6. Train adjusts behavior based on event
```

---

## 8. SOLID PRINCIPLES SUMMARY

| Principle               | How We Apply It |
|-------------------------|----------------|
| **Single Responsibility** | Each state class handles one state. Each command does one action. |
| **Open/Closed**           | Add new states/strategies/commands without changing existing code. |
| **Liskov Substitution**   | All states work through ITrainState. All strategies through IPathfindingStrategy. |
| **Interface Segregation** | IObserver has 1 method. ICommand has focused methods. Lean interfaces. |
| **Dependency Inversion**  | Train depends on ITrainState, not AcceleratingState. PathFinder depends on IPathfindingStrategy, not Dijkstra. |

---

## CHECKLIST FOR EVALUATION

### Design Patterns (Need 3, We Have 6):
- [x] Factory Pattern (TrainFactory, EventFactory)
- [x] Observer Pattern (EventDispatcher)
- [x] Strategy Pattern (PathFinding)
- [x] State Pattern (Train states)
- [x] Command Pattern (CommandManager)
- [x] Builder Pattern (SimulationBuilder)

### SOLID Principles:
- [x] Single Responsibility - each class has one job
- [x] Open/Closed - extendable without modification
- [x] Liskov Substitution - polymorphism works
- [x] Interface Segregation - lean interfaces
- [x] Dependency Inversion - depend on abstractions

### Questions They'll Ask:
- "Why did you choose Factory?"   -> **To centralize creation and validation**
- "Why Observer over direct calls?" -> **Decoupling, easy to add observers**
- "Why State over if/else?"       -> **Cleaner code, easier to extend**
- "Why Strategy?"                 -> **Can swap algorithms, benchmark different approaches**
- "Show me SOLID in your code?"   -> **Point to specific examples above**