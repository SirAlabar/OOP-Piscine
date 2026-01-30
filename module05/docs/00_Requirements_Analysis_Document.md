# 📋 REQUIREMENTS ANALYSIS DOCUMENT
*Railway Simulation Engine - Module 05*

---

## 1. MANDATORY REQUIREMENTS

### MR-001: Railway Network Definition System
**Description:** System must allow defining a railway network from input file containing nodes (cities/rail points) and rail segments connecting them.

**Input Format:**
```
Node <name>
Rail <nodeA> <nodeB> <length_km> <speed_limit_kmh>
```

**Acceptance Criteria:**
- [ ] Parse Node declarations with name validation (no duplicates)
- [ ] Parse Rail declarations linking two existing nodes
- [ ] Store length as double (kilometers)
- [ ] Store speed_limit as double (km/h)
- [ ] Build graph structure with adjacency relationships
- [ ] Validate file syntax and output clear error messages on failure
- [ ] Reject invalid data (negative lengths, non-existent nodes, duplicate rails)

**Test Cases:**
- ✅ Valid network file with 10+ nodes, 15+ rails
- ❌ Network with duplicate node names → Error: "Node 'CityA' already exists"
- ❌ Rail referencing non-existent node → Error: "Node 'InvalidCity' not found"
- ❌ Negative rail length → Error: "Rail length must be positive"

**Dependencies:** None (foundation)

**Design Pattern:** Factory Pattern for Node/Rail creation

---

### MR-002: Train Modeling System
**Description:** Model trains with physical characteristics and journey parameters.

**Input Format:**
```
<name> <mass_tons> <friction_coef> <max_accel_kN> <max_brake_kN> <departure_station> <arrival_station> <departure_time> <station_stop_duration>
```

**Acceptance Criteria:**
- [ ] Parse train parameters from file
- [ ] Store physical properties: mass, friction coefficient, max acceleration force, max braking force
- [ ] Store journey properties: departure/arrival stations, departure time, stop duration
- [ ] Validate all parameters (positive values, valid stations)
- [ ] Generate unique ID for each train
- [ ] Support multiple trains from same file

**Train Properties:**
```cpp
class Train {
    string name;
    int id;  // unique, auto-generated
    double mass;           // metric tons
    double friction_coef;  // dimensionless (0.001-0.01 typical for trains)
    double max_accel_force; // kilonewtons
    double max_brake_force; // kilonewtons
    string departure_station;
    string arrival_station;
    Time departure_time;    // format: HHhMM
    Time stop_duration;     // format: HHhMM (at each intermediate station)
};
```

**Test Cases:**
- ✅ Train with valid parameters
- ❌ Negative mass → Error: "Train mass must be positive"
- ❌ Invalid station → Error: "Departure station 'InvalidCity' not in network"
- ❌ Invalid time format → Error: "Expected time format HHhMM, got '25h00'"

**Dependencies:** MR-001 (needs network to validate stations)

**Design Pattern:** Factory Pattern for Train creation, State Pattern for train behavior

---

### MR-003: Pathfinding System
**Description:** Determine optimal route between departure and arrival stations.

**Acceptance Criteria:**
- [ ] Implement graph pathfinding algorithm (Dijkstra minimum)
- [ ] Edge cost = estimated travel time based on distance and speed limit
- [ ] Find path from departure to arrival station
- [ ] Handle cases where no path exists
- [ ] Cache computed paths to avoid redundant calculations
- [ ] Return ordered list of nodes/rails to traverse

**Cost Function:**
```
travel_time_per_rail = rail.length / rail.speed_limit
total_cost = sum(travel_time for all rails in path)
```

**Test Cases:**
- ✅ Simple path: A → B (direct connection)
- ✅ Complex path: A → C (via B, D)
- ✅ Multiple possible routes: choose shortest time
- ❌ No path exists: A → Z (disconnected) → Error: "No route found from A to Z"

**Dependencies:** MR-001 (needs graph structure)

**Design Pattern:** Strategy Pattern (IPathfindingStrategy → DijkstraStrategy, later AStarStrategy)

---

### MR-004: Physics Simulation Engine
**Description:** Simulate realistic train movement based on forces, mass, friction, and speed limits.

**Physics Model (Detailed):**

```
FORCES:
1. Applied Force (Fa):
   - When accelerating: Fa = max_accel_force (up to speed limit)
   - When cruising: Fa = friction_force (to maintain speed)
   - When braking: Fa = -max_brake_force

2. Friction Force (Ff):
   Ff = friction_coef × mass × g
   where g = 9.8 m/s² (gravity constant)
   
3. Net Force:
   Fnet = Fa - Ff (in direction of motion)

KINEMATICS:
1. Acceleration:
   a = Fnet / mass

2. Velocity Update (fixed timestep):
   v(t + Δt) = v(t) + a × Δt
   
   Constraint: v ≤ rail_speed_limit

3. Position Update:
   position(t + Δt) = position(t) + v(t) × Δt
   
   Position is distance along current rail segment (0 to rail.length)

4. Braking Distance Calculation:
   When approaching rail end or station:
   brake_decel = (max_brake_force + friction_force) / mass
   stopping_distance = v² / (2 × brake_decel)
```

**Unit Conversions:**
```
mass: tons → kg: ×1000
force: kN → N: ×1000
speed: km/h → m/s: ÷3.6
distance: km → m: ×1000
```

**Acceptance Criteria:**
- [ ] Convert all units to SI (m, kg, s, N)
- [ ] Calculate net force correctly
- [ ] Update velocity with acceleration (capped by speed limit)
- [ ] Update position with velocity
- [ ] Detect when train reaches end of rail segment
- [ ] Calculate braking distance accurately
- [ ] Initiate braking when needed to stop at station or before occupied rail

**State Transitions:**
```
Idle → Accelerating (at departure time)
Accelerating → Cruising (reached speed limit)
Cruising → Braking (approaching station or obstacle)
Braking → Stopped (velocity = 0 at station)
Stopped → Accelerating (after stop duration elapsed)
Any → Waiting (rail ahead occupied)
Waiting → Accelerating (rail cleared)
```

**Test Cases:**
- ✅ Train accelerates from rest to speed limit
- ✅ Train maintains speed limit (doesn't exceed)
- ✅ Train brakes smoothly to stop at station
- ✅ Heavier train accelerates slower than lighter train (same force)
- ✅ Higher friction coefficient reduces acceleration

**Dependencies:** MR-002 (needs train properties), MR-003 (needs path)

**Design Pattern:** State Pattern for train behavior states

---

### MR-005: Multi-Train Simulation
**Description:** Simulate multiple trains operating simultaneously on the network.

**Simulation Loop Structure:**
```
timestep = 1 second (configurable)

while (simulation_running):
    current_time += timestep
    
    for each train:
        if train.departure_time == current_time:
            train.state = Accelerating
        
        train.update(timestep)
        physics.apply(train, timestep)
        
    check_collisions()
    update_events()
    
    if (current_time % output_interval == 0):
        write_output_snapshot()
```

**Acceptance Criteria:**
- [ ] Support simultaneous simulation of multiple trains
- [ ] Each train follows independent schedule
- [ ] Trains depart at specified times
- [ ] Fixed timestep loop (1 second by default)
- [ ] Physics updated each timestep for all trains
- [ ] Collision detection each timestep

**Test Cases:**
- ✅ 3 trains, same route, different departure times (should not collide)
- ✅ 2 trains, opposite directions, different rails (independent)
- ✅ 10+ trains in complex network

**Dependencies:** MR-001, MR-002, MR-004

**Design Pattern:** Singleton for SimulationManager (use carefully)

---

### MR-006: Collision Avoidance & Overtaking System
**Description:** Prevent train collisions and implement overtaking logic.

**Block-Based System:**
- Each rail segment is a "block"
- Only 1 train per block at a time
- Train cannot enter occupied block

**Overtaking Logic:**
```
IF:
  - Train A is ahead on rail
  - Train B is behind on same rail
  - Train B has higher current speed than Train A
  - Rail has "overtaking capacity" (attribute: can be added as bonus)
THEN:
  - Train B can overtake (temporarily occupy same block)
ELSE:
  - Train B enters WaitingState
  - Train B slows to match Train A's speed or stops
```

**Acceptance Criteria:**
- [ ] Track which train occupies each rail segment
- [ ] Before entering new rail, check if occupied
- [ ] If occupied, transition to WaitingState
- [ ] In WaitingState, train maintains safe following distance or stops
- [ ] When rail clears, train resumes Accelerating/Cruising
- [ ] (Basic) No overtaking initially; can be added as complexity
- [ ] (Advanced) Implement overtaking for faster trains

**Test Cases:**
- ✅ Train A on rail → Train B waits before entering
- ✅ Train A exits rail → Train B immediately proceeds
- ❌ Two trains should never be at same position

**Dependencies:** MR-005

---

### MR-007: Random Event System
**Description:** Generate random events affecting trains or network during simulation.

**Event Types Defined Below (see Section 4)**

**Acceptance Criteria:**
- [ ] Event generator creates events randomly during simulation
- [ ] Events can target cities (nodes) or rail segments
- [ ] Events have effects (delays, speed reductions, stops)
- [ ] Events have duration (start time, end time)
- [ ] Trains affected by events are notified (Observer Pattern)
- [ ] Event effects are reflected in train behavior

**Test Cases:**
- ✅ Station delay event → trains stop longer at that station
- ✅ Track maintenance → trains slow down on that rail
- ✅ Signal failure → trains stop at that node

**Dependencies:** MR-005

**Design Pattern:** Observer Pattern (EventManager notifies Trains/Rails), Factory Pattern (EventFactory creates events)

---

### MR-008: Travel Time Estimation & Output
**Description:** Estimate total travel time and generate detailed output file for each train journey.

**Output File Naming:**
```
<TrainName>_<DepartureTime>.result

Example: TrainAB1_14h10.result
```

**Output File Format:**
```
Train: <TrainName>
Final travel time: XXhXXm

[HHhMM] - [NodeA][NodeB] - [XX.XXkm] - [Status] - [Rail Visualization]
...
```

**Status Values:**
- `Speed up` (Accelerating)
- `Maintain` (Cruising)
- `Braking` (Braking)
- `Stopped` (Stopped at station)
- `Waiting` (Blocked by another train)

**Rail Visualization:**
- One cell `[ ]` per kilometer of rail
- Current train position marked with `[x]`
- Blocking train marked with `[O]`
- Example: `[x][ ][ ][ ][ ][ ][ ]` (train at start of 7km rail)
- Example: `[ ][ ][ ][x][ ][ ][ ]` (train at 3km into 7km rail)
- Example: `[ ][ ][ ][x][O][ ][ ]` (train at 3km, another train blocking at 4km)

**Output Interval:**
- Every 5 minutes of simulation time (configurable)
- Last line should be arrival (distance = 0.00km, status = Stopped)

**Travel Time Estimation (Initial):**
```
For each rail in path:
    estimated_time += rail.length / rail.speed_limit

Add: stop_duration × number_of_intermediate_stations

This is optimistic (assumes instant acceleration to speed limit)
```

**Acceptance Criteria:**
- [ ] Generate one output file per train
- [ ] File contains header with train name and estimated time
- [ ] File contains snapshot lines every 5 sim-minutes
- [ ] Each line has correct format
- [ ] Distance remaining decreases correctly
- [ ] Status reflects current train state
- [ ] Rail visualization accurate (1 cell per km, correct position)
- [ ] Final line shows arrival (distance = 0, Stopped)

**Test Cases:**
- ✅ Simple journey A → B: output shows progression
- ✅ Journey with intermediate stops: output shows "Stopped" status at stations
- ✅ Journey with blockage: output shows "Waiting" and `[O]` in visualization

**Dependencies:** MR-005, MR-006

---

### MR-009: Input Validation & Error Handling
**Description:** Robust validation of input files with clear error messages.

**Validation Rules:**

**Network File:**
- [ ] Node names must be unique
- [ ] Rail must connect two existing nodes
- [ ] Length must be positive
- [ ] Speed limit must be positive
- [ ] No duplicate rails (same pair of nodes)

**Train File:**
- [ ] Name must be non-empty
- [ ] Mass > 0
- [ ] Friction coefficient >= 0
- [ ] Max accel force > 0
- [ ] Max brake force > 0
- [ ] Departure/arrival stations must exist in network
- [ ] Departure time in format HHhMM
- [ ] Stop duration in format HHhMM
- [ ] Path must exist from departure to arrival

**Error Message Format:**
```
Error: <Clear description>
Line: <line number>
Content: <problematic line>

Example:
Error: Node 'CityZ' not found in network (referenced in Rail definition)
Line: 15
Content: Rail CityA CityZ 25.0 200.0
```

**Acceptance Criteria:**
- [ ] All invalid inputs detected before simulation starts
- [ ] Clear, actionable error messages
- [ ] Line numbers provided when possible
- [ ] Program exits gracefully on invalid input (no crash, no undefined behavior)

**Test Cases:**
- ❌ Each invalid case should produce appropriate error message

**Dependencies:** All input-related requirements

---

### MR-010: CLI Interface
**Description:** Command-line interface for running simulation and displaying help.

**Usage:**
```bash
./railway_sim <network_file> <train_file> [options]

Options:
  --help              Show help message including file format specs
  --seed=<N>          Set random seed for deterministic simulation
  --render            Enable SFML visualization
  --output-dir=<dir>  Output directory for result files (default: current dir)
  --timestep=<sec>    Simulation timestep in seconds (default: 1)
```

**Help Output (`--help`):**
```
Railway Simulation Engine

Usage: ./railway_sim <network_file> <train_file> [options]

Network File Format:
  Node <name>
  Rail <nodeA> <nodeB> <length_km> <speed_limit_kmh>
  
  Example:
    Node CityA
    Node CityB
    Rail CityA CityB 50.0 250.0

Train File Format:
  <name> <mass_tons> <friction_coef> <max_accel_kN> <max_brake_kN> <departure> <arrival> <depart_time> <stop_duration>
  
  Example:
    TrainAB 80 0.005 356.0 500.0 CityA CityB 14h10 00h10

Options:
  --help              Show this help message
  --seed=<N>          Set random seed (for deterministic mode)
  --render            Enable graphical visualization
  ...
```

**Acceptance Criteria:**
- [ ] Correct argument count validation
- [ ] `--help` displays comprehensive help
- [ ] Options parsed correctly
- [ ] Invalid options produce error and suggest `--help`

**Test Cases:**
- ❌ No arguments → usage message
- ❌ Only 1 argument → usage message
- ✅ `./railway_sim network.txt trains.txt` → runs simulation
- ✅ `./railway_sim --help` → displays help

**Dependencies:** None (foundation)

---

## 2. BONUS FEATURES

### BF-001: SFML Real-Time Visualization
**Description:** Graphical rendering of simulation with interactive camera.

**Implementation Notes:**
- Use SFML library (graphics module)
- Separate rendering thread from simulation (or fixed 60 FPS rendering, 1 FPS simulation update)
- Model-View separation (SimulationManager doesn't know about renderer)

**Features:**
- [ ] Draw nodes as circles (cities larger, rail nodes smaller)
- [ ] Draw rails as lines between nodes
- [ ] Color-code rails by speed limit (green=fast, yellow=medium, red=slow)
- [ ] Draw trains as moving shapes along rails
- [ ] Color-code trains by state (blue=cruising, green=accelerating, red=braking, gray=stopped, yellow=waiting)
- [ ] Camera pan with WASD or arrow keys
- [ ] Camera zoom with mouse wheel
- [ ] Pause/resume with SPACE
- [ ] Display simulation time on screen
- [ ] FPS counter

**CLI Flag:**
```bash
./railway_sim network.txt trains.txt --render
```

**Design Pattern:** Strategy Pattern (IRenderer interface), Dependency Inversion

---

### BF-002: Hot Reload System
**Description:** Reload input files during runtime without restarting simulation.

**Implementation Notes:**
- Use file system watcher or periodic polling
- On file change, re-parse and apply changes
- Validate changes don't break current simulation state

**Supported Changes:**
- [ ] Add new nodes (if not interfering with active trains)
- [ ] Add new rail segments
- [ ] Modify rail speed limits
- [ ] Add new trains (with future departure times)
- [ ] Trigger events via file modification

**Unsupported (Must Reject):**
- Deleting nodes/rails currently occupied
- Modifying active train properties mid-journey

**CLI Flag:**
```bash
./railway_sim network.txt trains.txt --hot-reload
```

**Design Pattern:** Command Pattern (changes applied as commands), Observer Pattern (file watcher notifies manager)

---

### BF-003: Deterministic Mode with Seeded RNG
**Description:** Reproducible simulations using seeded random number generator.

**Implementation Notes:**
- Wrap `<random>` library with SeededRNG class
- All randomness sources use this RNG (event generation, event timing, any stochastic choices)
- Fixed timestep already ensures determinism in physics

**Seed Sources:**
- CLI argument: `--seed=42`
- If not provided, use random seed and log it

**Acceptance Criteria:**
- [ ] Same seed + same inputs = identical output every time
- [ ] Seed logged to console/file at simulation start
- [ ] No global `rand()` or unseeded `random` usage

**CLI Flag:**
```bash
./railway_sim network.txt trains.txt --seed=12345
```

---

### BF-004: Replay System
**Description:** Record all simulation decisions and replay them exactly.

**Implementation Notes:**
- Record file contains: seed, input files, all commands/events with timestamps
- Replay player re-executes recorded actions

**Record Format (JSON):**
```json
{
  "seed": 42,
  "network_file": "network.txt",
  "train_file": "trains.txt",
  "events": [
    {"time": "00h05", "type": "train_departure", "train_id": 1},
    {"time": "00h10", "type": "event_occurred", "event": {...}},
    ...
  ]
}
```

**CLI Flags:**
```bash
./railway_sim network.txt trains.txt --record=replay.json
./railway_sim --replay=replay.json [--render]
```

**Design Pattern:** Command Pattern (events as commands), Memento Pattern (state snapshots)

---

### BF-005: Monte Carlo Simulation Mode
**Description:** Run simulation N times with different seeds, collect statistics.

**Implementation Notes:**
- Headless mode (no rendering)
- Iterate N times with different seeds
- Collect metrics each run
- Output aggregate statistics

**Metrics:**
```
Per Train:
- Travel time (min, max, avg, stddev)
- Number of blockages
- Time spent waiting
- Events encountered

Per Network:
- Rail utilization (% time occupied)
- Bottleneck identification
- Event frequency distribution
```

**Output Format:**
CSV file with columns: `run_id, train_name, travel_time, num_blockages, wait_time, ...`

**CLI Flag:**
```bash
./railway_sim network.txt trains.txt --monte-carlo=1000 --output=stats.csv
```

---

### BF-006: Enhanced Visualization & UX
**Description:** Visual polish and interactive features.

**Features:**
- [ ] Smooth train animation (interpolation between timesteps)
- [ ] Trail effect (show recent path)
- [ ] Event visualization (icons at affected locations)
- [ ] Click train → show info panel (name, speed, state, ETA)
- [ ] Click rail → show occupancy, traffic history
- [ ] Mini-map of full network
- [ ] Time acceleration controls (1x, 2x, 5x, 10x speed)
- [ ] Particle effects (optional)
- [ ] Sound effects (optional)

---

### BF-007: Advanced Pathfinding (A* Algorithm)
**Description:** Faster pathfinding using heuristic.

**Implementation Notes:**
- Add IPathfindingStrategy implementation: AStarStrategy
- Heuristic: straight-line distance / average speed
- CLI to select algorithm: `--pathfinding=astar` or `--pathfinding=dijkstra`

**Design Pattern:** Strategy Pattern (swap algorithms via config)

---

### BF-008: Console Command Interface
**Description:** Interactive console for runtime commands.

**Commands:**
```
> add_train <name> <mass> ... <departure> <arrival> <time> <duration>
> remove_train <id>
> modify_rail <nodeA> <nodeB> <new_speed_limit>
> trigger_event <type> <target> <params>
> pause
> resume
> step <num_steps>
> save_state <filename>
> load_state <filename>
```

**Design Pattern:** Command Pattern

---

## 3. PHYSICS MODEL SPECIFICATION

### 3.1 Force Calculations

**Given:**
- `m` = mass (tons → kg: multiply by 1000)
- `μ` = friction_coefficient (dimensionless, typically 0.001-0.01 for trains)
- `F_accel` = max_accel_force (kN → N: multiply by 1000)
- `F_brake` = max_brake_force (kN → N: multiply by 1000)
- `g` = 9.8 m/s² (gravitational acceleration)

**Friction Force:**
```
F_friction = μ × m × g  [in Newtons]
```

**Net Force (Accelerating):**
```
F_net = F_accel - F_friction
```

**Net Force (Braking):**
```
F_net = -(F_brake + F_friction)
```
*(Both brake force and friction oppose motion)*

**Net Force (Cruising at speed limit):**
```
F_net = 0  (or apply just enough to counter friction)
```

### 3.2 Motion Equations

**Acceleration:**
```
a = F_net / m  [m/s²]
```

**Velocity Update (Euler integration):**
```
v_new = v_old + a × Δt

if v_new > v_limit:
    v_new = v_limit  (enforce speed limit)

if v_new < 0:
    v_new = 0  (can't go backward)
```

**Position Update:**
```
position_new = position_old + v_old × Δt  [meters along rail]
```

### 3.3 Braking Distance

**Braking Deceleration:**
```
a_brake = (F_brake + F_friction) / m
```

**Stopping Distance (from current velocity v):**
```
d_stop = v² / (2 × a_brake)
```

**Decision Logic:**
```
distance_to_target = rail.length - current_position + distance_to_final_destination

if distance_to_target <= d_stop:
    Enter BrakingState
```

### 3.4 State Transition Logic

```
State: Idle
  → at departure_time: Enter AcceleratingState

State: Accelerating
  → if v >= speed_limit: Enter CruisingState
  → if need to brake: Enter BrakingState
  → if rail ahead occupied: Enter WaitingState

State: Cruising
  → if need to brake: Enter BrakingState
  → if rail ahead occupied: Enter WaitingState

State: Braking
  → if v == 0 at station: Enter StoppedState
  → if v == 0 before destination: Error (should never happen with correct braking distance)

State: Stopped
  → after stop_duration elapsed: Enter AcceleratingState (if more rails in path)
  → if at final destination: Simulation complete for this train

State: Waiting
  → if rail ahead clears: Enter AcceleratingState
```

### 3.5 Example Calculation

**Given Train:**
- mass = 80 tons = 80,000 kg
- friction_coef = 0.005
- max_accel = 356 kN = 356,000 N
- max_brake = 500 kN = 500,000 N
- current speed = 0 m/s (at rest)
- rail speed limit = 250 km/h = 69.44 m/s

**Step 1: Calculate friction force**
```
F_friction = 0.005 × 80,000 × 9.8 = 3,920 N
```

**Step 2: Calculate net acceleration force**
```
F_net = 356,000 - 3,920 = 352,080 N
```

**Step 3: Calculate acceleration**
```
a = 352,080 / 80,000 = 4.40 m/s²
```

**Step 4: Update velocity (Δt = 1 second)**
```
v_new = 0 + 4.40 × 1 = 4.40 m/s
```

**Step 5: Update position**
```
position_new = 0 + 0 × 1 = 0 m  (still at start since v_old was 0)
```

**Next timestep:**
```
v_new = 4.40 + 4.40 × 1 = 8.80 m/s
position_new = 0 + 4.40 × 1 = 4.40 m
```

**Continue until v reaches 69.44 m/s (speed limit)**

**Time to reach speed limit:**
```
t = v_limit / a = 69.44 / 4.40 = 15.8 seconds
```

**Braking distance from max speed:**
```
a_brake = (500,000 + 3,920) / 80,000 = 6.30 m/s²
d_stop = 69.44² / (2 × 6.30) = 383 meters = 0.383 km
```

**Conclusion:** This train needs to start braking 383 meters before its destination to stop smoothly.

---

## 4. EVENT SYSTEM SPECIFICATION

### 4.1 Event Types

#### Event Type 1: Station Delay
**Description:** Train must wait additional time at station.

**Properties:**
- `station_name`: which station
- `additional_delay`: time in minutes (HHhMM format)
- `start_time`: when delay begins
- `end_time`: when delay ends (or null for indefinite)

**Effect:**
- When train stops at affected station, add `additional_delay` to its `stop_duration`

**Example:**
```
StationDelayEvent {
  station: "CityB",
  additional_delay: "00h15",
  start_time: "10h00",
  end_time: "12h00"
}
```

**Trigger Conditions:**
- Random (probability-based)
- Or manual trigger (console command, hot reload)

---

#### Event Type 2: Track Maintenance
**Description:** Rail segment has reduced speed limit.

**Properties:**
- `rail_id`: which rail (or nodeA, nodeB pair)
- `speed_reduction_factor`: 0.0-1.0 (e.g., 0.5 = half speed)
- `start_time`
- `end_time`

**Effect:**
- Affected rail's effective speed limit = `original_limit × speed_reduction_factor`
- Trains on that rail cannot exceed reduced limit

**Example:**
```
TrackMaintenanceEvent {
  rail: (CityA, RailNodeA),
  speed_reduction: 0.6,  // 60% of original speed
  start_time: "08h00",
  end_time: "10h00"
}
```

---

#### Event Type 3: Signal Failure
**Description:** Node (station or junction) requires trains to stop completely.

**Properties:**
- `node_name`
- `stop_duration`: forced stop time (e.g., "00h05")
- `start_time`
- `end_time`

**Effect:**
- Any train reaching this node must enter StoppedState for `stop_duration`
- Even if it's not a station stop on their route

**Example:**
```
SignalFailureEvent {
  node: "RailNodeC",
  stop_duration: "00h05",
  start_time: "14h30",
  end_time: "14h45"
}
```

---

#### Event Type 4: Weather Condition
**Description:** Affects multiple rails in a region (all rails within certain distance of a center point).

**Properties:**
- `center_node`: epicenter of weather
- `radius_km`: how far effect spreads
- `speed_reduction_factor`
- `friction_increase`: increase friction coefficient (e.g., +0.02 for rain)
- `start_time`
- `end_time`

**Effect:**
- All rails within radius have reduced speed limits
- Trains on affected rails have increased friction (harder to accelerate/brake)

**Example:**
```
WeatherEvent {
  type: "Heavy Rain",
  center: "CityB",
  radius: 50.0,  // 50 km
  speed_reduction: 0.7,
  friction_increase: 0.02,
  start_time: "16h00",
  duration: "02h00"
}
```

---

### 4.2 Event Generation Logic

**Random Event Generator:**
```
class EventGenerator {
    SeededRNG rng;
    vector<EventProbability> event_probabilities;
    
    void update(current_time) {
        for each event_type in event_probabilities:
            if rng.random() < event_type.probability_per_hour:
                Event* event = EventFactory::create(event_type);
                event->start_time = current_time;
                event->end_time = current_time + random_duration();
                EventManager::getInstance().scheduleEvent(event);
    }
};
```

**Event Probabilities (Example Config):**
```
StationDelay: 0.1 per hour (10% chance per hour)
TrackMaintenance: 0.05 per hour
SignalFailure: 0.02 per hour
Weather: 0.01 per hour
```

**Observer Notification:**
```
When event scheduled:
  EventManager.notify(ALL_OBSERVERS, event)

Observers (Trains, Rails, Nodes):
  onNotify(event) {
    if event.affects(this):
      apply_event_effects()
  }
```

---

### 4.3 Event Impact on Simulation

**Example Scenario:**

```
Time: 10h00 - TrackMaintenanceEvent starts on rail (CityA, RailNodeA)
  → Rail speed_limit: 250 km/h → 150 km/h (0.6 factor)
  
Time: 10h05 - Train1 enters this rail at 200 km/h
  → Train1 must decelerate to 150 km/h (speed limit enforced)
  → Train1 enters BrakingState briefly, then CruisingState at 150 km/h
  
Time: 12h00 - TrackMaintenanceEvent ends
  → Rail speed_limit restored to 250 km/h
  → Train1 (if still on rail) can accelerate back to 250 km/h
```

---

## 5. ACCEPTANCE TESTING CHECKLIST

### Mandatory Requirements Complete:
- [ ] MR-001: Network parsing works with valid/invalid inputs
- [ ] MR-002: Train parsing works with valid/invalid inputs
- [ ] MR-003: Pathfinding finds correct routes
- [ ] MR-004: Physics calculations verified with hand calculations
- [ ] MR-005: Multiple trains simulated simultaneously
- [ ] MR-006: Collision avoidance prevents crashes
- [ ] MR-007: Random events generated and applied
- [ ] MR-008: Output files match required format
- [ ] MR-009: All error cases handled gracefully
- [ ] MR-010: CLI interface complete with `--help`

### Design Patterns Implemented:
- [ ] Factory Pattern (Train, Event creation)
- [ ] Observer Pattern (Event notifications)
- [ ] Strategy Pattern (Pathfinding)
- [ ] State Pattern (Train behavior)
- [ ] Command Pattern (Simulation control)
- [ ] Singleton Pattern (SimulationManager)

### UML Diagrams Created:
- [ ] Class Diagram (all major classes)
- [ ] Sequence Diagram (train journey flow)
- [ ] (Optional) State Machine Diagram (train states)
- [ ] (Optional) Component Diagram (system architecture)

### SOLID Principles Applied:
- [ ] Single Responsibility: Each class has one job
- [ ] Open/Closed: Extendable without modification
- [ ] Liskov Substitution: Polymorphism works correctly
- [ ] Interface Segregation: Lean interfaces
- [ ] Dependency Inversion: Depend on abstractions

---

## 6. TESTING EXAMPLES

### Test Scenario 1: Simple Journey
**Network:**
```
Node CityA
Node CityB
Rail CityA CityB 50.0 200.0
```

**Train:**
```
TestTrain 100 0.005 400.0 500.0 CityA CityB 10h00 00h05
```

**Expected Output:**
- Train departs at 10h00
- Accelerates to 200 km/h
- Cruises 50 km
- Brakes and stops at CityB
- Output file shows progression every 5 minutes

---

### Test Scenario 2: Collision Avoidance
**Network:**
```
Node A
Node B
Node C
Rail A B 30.0 150.0
Rail B C 30.0 150.0
```

**Trains:**
```
Train1 80 0.005 300.0 500.0 A C 10h00 00h10
Train2 60 0.005 400.0 500.0 A C 10h05 00h10
```

**Expected Behavior:**
- Train1 departs first, occupies A→B rail
- Train2 departs 5 min later
- Train2 reaches end of A but cannot enter A→B (occupied)
- Train2 enters WaitingState
- Train1 clears A→B rail
- Train2 resumes journey

**Output Verification:**
- Train2's output shows "Waiting" status
- Train2's rail visualization shows `[O]` where Train1 blocks

---

### Test Scenario 3: Event Impact
**Network:** (same as above)

**Event:**
```
TrackMaintenanceEvent on rail A→B, speed reduction 0.5, time 10h00-11h00
```

**Train:**
```
Train1 80 0.005 300.0 500.0 A C 10h10 00h10
```

**Expected Behavior:**
- Train1 departs at 10h10 (event already active)
- Rail A→B speed limit: 150 km/h → 75 km/h
- Train1 limited to 75 km/h on this rail
- At 11h00, event ends, speed limit restored
- If train still on A→B, it can accelerate to 150 km/h

---

## Modeling Assumptions

- Rails are modeled as single-track bidirectional segments with mutual exclusion.
- All nodes use a single Node class; NodeType distinguishes stations and junctions.
- Train IDs are sequential deterministic integers assigned in input order.

## NEXT STEPS

With this requirements document complete, you can now proceed to:

1. **0.2 Design Patterns Mapping** - Map each requirement to specific classes/patterns
2. **0.3 UML Diagrams** - Visualize the architecture
3. **0.4 Physics Design** - Create detailed calculation reference
4. **0.5 Architecture Decisions** - Document all key design choices