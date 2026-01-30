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