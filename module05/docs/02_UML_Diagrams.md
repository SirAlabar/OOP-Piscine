# 🎨 UML DIAGRAMS
*3 Essential Diagrams - Clean & Complete*

---

## 1. CLASS DIAGRAM - COMPLETE ARCHITECTURE

**Shows:** All 6 patterns + core entities in one organized view

```mermaid
classDiagram
    %% ===== CORE ENTITIES =====
    class Graph {
        +addNode(Node*)
        +addRail(Rail*)
        +getNode(name) Node*
    }
    
    class Node {
        -string name
        +getName() string
    }
    
    class Rail {
        -double length
        -double speedLimit
        -Train* occupiedBy
        +getLength() double
        +isOccupied() bool
    }

    %% ===== TRAIN + STATE PATTERN =====
    class Train {
        -string name 
        -int id
        -double mass
        -double velocity
        -ITrainState* state
        +update(dt)
        +setState(state)
        +onNotify(event)
    }
    
    class ITrainState {
        <<interface>>
        +update(Train*, dt)*
        +getName()*
    }
    
    class IdleState
    class AcceleratingState  
    class CruisingState
    class BrakingState
    class StoppedState
    class WaitingState

    %% ===== STRATEGY PATTERN =====
    class PathFinder {
        +findPath(start, end)
    }
    
    class IPathfindingStrategy {
        <<interface>>
        +findPath(graph, start, end)*
    }
    
    class DijkstraStrategy
    class AStarStrategy

    %% ===== OBSERVER PATTERN =====
    class EventManager {
        +attach(observer)
        +notify(event)
    }
    
    class IObserver {
        <<interface>>
        +onNotify(event)*
    }
    
    class Event {
        <<abstract>>
        +apply()*
        +revert()*
    }

    %% ===== FACTORY PATTERN =====
    class TrainFactory {
        +createTrain(config) Train*
    }
    
    class EventFactory {
        +createRandomEvent() Event*
    }

    %% ===== SINGLETON PATTERN =====
    class SimulationManager {
        -static instance
        -SimulationManager()
        +getInstance()$
        +update(dt)
    }

    %% ===== RELATIONSHIPS =====
    
    %% Core structure
    Graph --> Node : contains
    Graph --> Rail : contains
    Rail --> Node : connects 2
    
    %% Train uses State (STATE PATTERN)
    Train --> ITrainState : currentState
    ITrainState <|.. IdleState
    ITrainState <|.. AcceleratingState
    ITrainState <|.. CruisingState
    ITrainState <|.. BrakingState
    ITrainState <|.. StoppedState
    ITrainState <|.. WaitingState
    
    %% PathFinder uses Strategy (STRATEGY PATTERN)
    PathFinder --> IPathfindingStrategy : uses
    IPathfindingStrategy <|.. DijkstraStrategy
    IPathfindingStrategy <|.. AStarStrategy
    Graph --> PathFinder : uses
    
    %% Event notification (OBSERVER PATTERN)
    EventManager --> IObserver : notifies
    EventManager --> Event : manages
    Train ..|> IObserver : implements
    Rail ..|> IObserver : implements
    
    %% Factories create objects (FACTORY PATTERN)
    TrainFactory ..> Train : creates
    EventFactory ..> Event : creates
    
    %% Singleton manages everything (SINGLETON PATTERN)
    SimulationManager --> Train : manages *
    SimulationManager --> Graph : owns
    SimulationManager --> EventManager : uses
```

**Key Points:**
- **6 Patterns Visible:** State, Strategy, Observer, Factory, Singleton (+ Command in bonus)
- **Core Entities:** Graph, Node, Rail, Train
- **SOLID Principles:** All dependencies point to interfaces (ITrainState, IPathfindingStrategy, IObserver)

### Domain Modeling Notes

- Rails are represented as bidirectional edges with exclusive occupancy.
- Node roles (City vs Junction) are represented via NodeType enum.
- Train IDs are deterministic and sequential for reproducibility.

---

## 2. SEQUENCE DIAGRAM - TRAIN JOURNEY (Runtime Behavior)

**Shows:** How objects interact over time - from creation to simulation

```mermaid
sequenceDiagram
    participant Parser
    participant Graph
    participant TrainFactory
    participant Train
    participant EventManager
    participant PathFinder
    participant SimManager as SimulationManager
    participant Physics as PhysicsSystem
    participant Collision as CollisionSystem

    Note over Parser,Graph: INITIALIZATION PHASE
    
    Parser->>Graph: addNode(CityA)
    Parser->>Graph: addNode(CityB)
    Parser->>Graph: addRail(CityA, CityB, 50km, 250kmh)
    
    Parser->>TrainFactory: createTrain(config)
    TrainFactory->>Train: new Train()
    Train->>Train: setState(IdleState)
    TrainFactory-->>Parser: train*

    Parser->>EventManager: attach(train)
    
    Parser->>PathFinder: findPath(start, end)
    PathFinder->>Graph: getNode(), getRails()
    PathFinder->>PathFinder: strategy->findPath()
    PathFinder-->>Parser: path
    
    Parser->>Train: setPath(path)
    Parser->>SimManager: addTrain(train)

    Note over SimManager,Collision: SIMULATION LOOP
    
    loop Every Timestep (dt = 1s)
        SimManager->>Train: update(dt)
        Train->>Train: currentState->update(this, dt)
        
        alt Departure Time
            Train->>Train: setState(AcceleratingState)
        end
        
        SimManager->>Physics: update(trains, dt)
        Physics->>Train: applyForces(), updateVelocity(), updatePosition()
        
        SimManager->>Collision: check(trains, rails)
        Collision->>Train: isNextRailOccupied()?
        
        alt Rail Occupied
            Train->>Train: setState(WaitingState)
        end
        
        alt Event Occurs
            EventManager->>Train: onNotify(event)
            Train->>Train: adjust behavior
        end
        
        alt Speed Limit Reached
            Train->>Train: setState(CruisingState)
        end
        
        alt Need Brake
            Train->>Train: setState(BrakingState)
        end
        
        alt At Station
            Train->>Train: setState(StoppedState)
        end
    end
```

**Key Flow:**
1. **Creation:** Factory creates train with validation
2. **Registration:** Train registers as event observer
3. **Planning:** PathFinder calculates route (Strategy pattern)
4. **Simulation:** Loop updates train, states transition automatically

---

## 3. STATE MACHINE DIAGRAM - TRAIN BEHAVIOR

**Shows:** All possible train states and transitions

```mermaid
stateDiagram-v2
    [*] --> Idle : Train created
    
    Idle --> Accelerating : departure_time reached
    
    Accelerating --> Cruising : v >= speed_limit
    Accelerating --> Braking : need to stop
    Accelerating --> Waiting : rail occupied
    
    Cruising --> Braking : approaching station
    Cruising --> Waiting : rail occupied
    
    Braking --> Stopped : v = 0 at station
    
    Stopped --> Accelerating : after stop_duration
    Stopped --> [*] : at final destination
    
    Waiting --> Accelerating : rail cleared
```

**State Descriptions:**

| State | What It Does | Physics Applied |
|-------|--------------|-----------------|
| **Idle** | Waiting at departure station | v = 0 |
| **Accelerating** | Speeding up to limit | Apply max_accel_force |
| **Cruising** | Maintaining speed limit | Balance friction |
| **Braking** | Slowing down to stop | Apply max_brake_force |
| **Stopped** | At station, passengers boarding | v = 0, wait stop_duration |
| **Waiting** | Blocked by another train | Gentle brake, wait |

**Why State Pattern:**
- ✅ No giant `if/else` in Train.update()
- ✅ Each state = separate class with own logic
- ✅ Easy to add new states (EmergencyState, MaintenanceState)

---

## EVALUATION GUIDE

### What Each Diagram Proves:

**Diagram 1 (Class):**
- ✅ "I understand OOP structure"
- ✅ "I use 6 design patterns"
- ✅ "I follow SOLID principles (interfaces everywhere)"

**Diagram 2 (Sequence):**
- ✅ "I understand runtime behavior"
- ✅ "My objects interact correctly"
- ✅ "Patterns work together in practice"

**Diagram 3 (State Machine):**
- ✅ "I model complex behavior properly"
- ✅ "State pattern solves real problem"
- ✅ "No spaghetti code in my train logic"

### Defense Questions & Answers:

**Q: "Show me the Factory pattern"**
- Point to TrainFactory → Train in Diagram 1
- "Creates trains with validation, assigns unique ID"

**Q: "Show me the State pattern"**
- Point to ITrainState hierarchy in Diagram 1
- Point to full state machine in Diagram 3
- "6 states, each handles own update logic"

**Q: "Show me the Observer pattern"**
- Point to EventManager → IObserver in Diagram 1
- Point to event notification in Diagram 2
- "EventManager notifies all trains/rails of events"

**Q: "Show me the Strategy pattern"**
- Point to PathFinder → IPathfindingStrategy in Diagram 1
- "Can swap Dijkstra/A* without changing code"

**Q: "Show me the Singleton pattern"**
- Point to SimulationManager in Diagram 1
- "One instance manages entire simulation"

**Q: "How do these patterns work together?"**
- Walk through Diagram 2 sequence
- "Factory creates → Observer registers → Strategy finds path → State controls behavior"

**Q: "Explain this relationship [points to arrow]"**
- Solid arrow = "has-a" (composition/aggregation)
- Dashed arrow = "creates" or "implements"
- Triangle = inheritance/interface implementation

**Q: "Why interface instead of concrete class?"**
- "Dependency Inversion - depend on abstractions"
- "Open/Closed - can add new implementations"
- "Easy to test with mocks"

---

## CHECKLIST

### Required for Evaluation:
- [x] **2+ UML diagrams** (we have 3)
- [x] **Class diagram** showing structure
- [x] **Behavioral diagram** showing runtime (sequence or state machine)

### Design Patterns Visible:
- [x] Factory Pattern (TrainFactory, EventFactory)
- [x] Observer Pattern (EventManager, IObserver)
- [x] Strategy Pattern (IPathfindingStrategy)
- [x] State Pattern (ITrainState + 6 states)
- [x] Singleton Pattern (SimulationManager)
- [x] Command Pattern (mentioned for bonus)

### SOLID Principles Visible:
- [x] **Single Responsibility** - Each state/strategy is separate class
- [x] **Open/Closed** - Interfaces allow extension without modification
- [x] **Liskov Substitution** - All states/strategies interchangeable
- [x] **Interface Segregation** - Lean interfaces (IObserver, ITrainState)
- [x] **Dependency Inversion** - Train depends on ITrainState, not concrete states

**Ready for defense!** 🎯
