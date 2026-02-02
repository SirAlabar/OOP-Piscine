```mermaid
stateDiagram-v2
    [*] --> Idle : Train created
    
    Idle --> Accelerating : departure_time reached
    
    Accelerating --> Cruising : v >= speed_limit
    
    %% Reação a tráfego
    Accelerating --> Braking : leader stopped or too close
    Cruising --> Braking : leader stopped or too close
    
    %% Fim de trilho também exige frenagem
    Cruising --> Braking : approaching station or rail end
    
    %% Fluxo de frenagem normal
    Braking --> Stopped : v = 0 and gap safe
    
    %% Situação crítica durante frenagem
    Braking --> Emergency : gap unsafe while braking
    
    %% Frenagem de emergência
    Emergency --> Stopped : v = 0 after emergency stop
    
    %% Parado fisicamente
    Stopped --> Waiting : leader still stopped
    Stopped --> Accelerating : no blocking train
    
    %% Estado lógico de espera
    Waiting --> Accelerating : leader moves or rail cleared
    
    %% Chegada final
    Stopped --> [*] : at final destination
