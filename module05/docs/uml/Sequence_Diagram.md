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