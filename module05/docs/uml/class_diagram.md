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

    %% ===== RELATIONSHIPS  =====
    
    %% Core structure - COMPOSITION (Graph owns Nodes/Rails)
    Graph "1" *-- "*" Node : owns
    Graph "1" *-- "*" Rail : owns
    Rail "1" --> "2" Node : connects
    
    %% Train uses State (STATE PATTERN)
    Train "1" --> "1" ITrainState : currentState
    ITrainState <|.. IdleState
    ITrainState <|.. AcceleratingState
    ITrainState <|.. CruisingState
    ITrainState <|.. BrakingState
    ITrainState <|.. StoppedState
    ITrainState <|.. WaitingState
    
    %% PathFinder uses Strategy (STRATEGY PATTERN) - CORRECT DIRECTION
    PathFinder "1" --> "1" IPathfindingStrategy : uses
    IPathfindingStrategy <|.. DijkstraStrategy
    IPathfindingStrategy <|.. AStarStrategy
    PathFinder --> Graph : queries
    
    %% Event notification (OBSERVER PATTERN)
    EventManager "1" --> "*" IObserver : notifies
    EventManager "1" --> "*" Event : manages
    Train ..|> IObserver : implements
    Rail ..|> IObserver : implements
    
    %% Factories create objects (FACTORY PATTERN)
    TrainFactory ..> Train : creates
    EventFactory ..> Event : creates
    
    %% Singleton manages everything (SINGLETON PATTERN) - ORCHESTRATES
    SimulationManager "1" --> "*" Train : manages
    SimulationManager "1" --> "1" Graph : owns
    SimulationManager --> EventManager : uses
    SimulationManager --> PathFinder : uses