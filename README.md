# Object-Oriented Programming Piscine

Complete implementation of the OOP Piscine with all bonus exercises.

## 📋 Table of Contents

- [Overview](#overview)
- [Project Structure](#project-structure)
- [Modules](#modules)
  - [Module 00 - Encapsulation](#module-00---encapsulation)
  - [Module 01 - Relationship](#module-01---relationship)
  - [Module 02 - UML](#module-02---uml)
  - [Module 03 - SOLID](#module-03---solid)
  - [Module 04 - Design Patterns](#module-04---design-patterns)
  - [Module 05 - Train Yourself](#module-05---train-yourself)
- [Compilation](#compilation)
- [Requirements](#requirements)
- [Evaluation](#evaluation)

---

## 🎯 Overview

This project is a comprehensive Object-Oriented Programming course divided into 6 modules. Each module introduces fundamental OOP concepts through practical exercises, building progressively toward a complex railway simulation system.

**Key Learning Objectives:**
- Master encapsulation principles
- Understand object relationships (Composition, Aggregation, Inheritance, Association)
- Learn UML diagram creation and interpretation
- Apply SOLID principles to real-world problems
- Implement Gang of Four design patterns
- Build a complete simulation system combining all learned concepts

---

## 📁 Project Structure
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
├── diagrams/      # UML diagrams
├── include/       # Header files
├── src/           # Source files
└── inputs/        # Simulation input files

# Object-Oriented Programming Piscine

Complete implementation of the OOP Piscine with all bonus exercises.

## 📋 Table of Contents

- [Overview](#overview)
- [Project Structure](#project-structure)
- [Modules](#modules)
  - [Module 00 - Encapsulation](#module-00---encapsulation)
  - [Module 01 - Relationship](#module-01---relationship)
  - [Module 02 - UML](#module-02---uml)
  - [Module 03 - SOLID](#module-03---solid)
  - [Module 04 - Design Patterns](#module-04---design-patterns)
  - [Module 05 - Train Yourself](#module-05---train-yourself)
- [Compilation](#compilation)
- [Requirements](#requirements)
- [Evaluation](#evaluation)

---

## 🎯 Overview

This project is a comprehensive Object-Oriented Programming course divided into 6 modules. Each module introduces fundamental OOP concepts through practical exercises, building progressively toward a complex railway simulation system.

**Key Learning Objectives:**
- Master encapsulation principles
- Understand object relationships (Composition, Aggregation, Inheritance, Association)
- Learn UML diagram creation and interpretation
- Apply SOLID principles to real-world problems
- Implement Gang of Four design patterns
- Build a complete simulation system combining all learned concepts

---

## 📁 Project Structure
```
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
    ├── diagrams/      # UML diagrams
    ├── include/       # Header files
    ├── src/           # Source files
    └── inputs/        # Simulation input files
```

---

## 📚 Modules

### Module 00 - Encapsulation

**Concept:** Understanding public vs private access control

#### Exercise 00: Divide and Conquer
- Fix a broken banking system with no encapsulation
- Implement proper access control for `Bank` and `Account` classes
- Ensure bank operations follow business rules

**Requirements:**
- Bank receives 5% of all deposits
- Account IDs must be unique
- Attributes must be private
- Implement getters/setters where appropriate

**Bonuses:**
- ✓ Account as nested class inside Bank
- ✓ `operator[]` for account access without loops
- ✓ Exception handling with try/catch

#### Exercise 01: I Don't Know What I'm Doing!
- Create a `Graph` class with `Vector2` points
- Output ASCII art visualization of plotted points

**Bonuses:**
- ✓ PNG image generation
- ✓ Line drawing between points
- ✓ Read points from input file

---

### Module 01 - Relationship

**Concept:** Understanding object relationships in OOP

#### Exercise 00: Putting It Into Practice

**Part 1 - Composition:** `Worker` contains `Position` and `Statistic`
- Objects cannot exist independently
- Lifecycle tied to parent object

**Part 2 - Aggregation:** `Worker` can hold/release `Shovel`
- Objects can exist independently
- Shovel survives Worker destruction

**Part 3 - Inheritance:** `Tool` → `Shovel`, `Hammer`
- Abstract base class with virtual methods
- Multiple tools for workers

**Part 4 - Association:** `Workshop` registers `Workers`
- Loose coupling between objects
- Workers can join/leave freely

**Bonuses:**
- ✓ Template method `Worker::GetTool<T>()`
- ✓ Workshop requires specific tool types
- ✓ Auto-release worker when tool is lost

---

### Module 02 - UML

**Concept:** Visual software architecture representation

#### Exercise 00: Car Composition
- Create complete class diagram for car system
- Include: Engine, Transmission, Wheels, Brakes, Steering, Pedals, etc.
- Show all relationships (composition, aggregation, inheritance)

**Bonuses:**
- ✓ Fix ascending arrows (all relationships descending/horizontal)
- ✓ Add public/private visibility markers
- ✓ Proper block organization
- ✓ Cardinality notation (1..*, 0..1)
- ✓ Sequence diagrams:
  - Steering operation
  - Braking operation
  - Acceleration operation

---

### Module 03 - SOLID

**Concept:** Five principles for maintainable OOP code

#### Exercise 00: [S]ingle Responsibility Principle
- Decompose monolithic `Car` class
- Each class should have one reason to change
- Separate: Engine, Transmission, Brakes, Steering

#### Exercise 01: [O]pen/Closed Principle
- `Command` class with discount strategies
- Open for extension, closed for modification
- Implement: `TuesdayDiscountCommand`, `PackageReductionDiscountCommand`

#### Exercise 02: [L]iskov Substitution Principle
- `Shape` hierarchy: Rectangle, Circle, Triangle
- Derived classes must be substitutable for base class
- Consistent `area()` and `perimeter()` behavior

#### Exercise 03: [I]nterface Segregation Principle
- `EmployeeManager` with multiple employee types
- Clients shouldn't depend on unused interfaces
- Implement: `HourlyEmployee`, `SalariedEmployee`, `TempWorker`, `ContractEmployee`, `Apprentice`

#### Exercise 04: [D]ependency Inversion Principle
- `ILogger` interface with multiple implementations
- High-level modules shouldn't depend on low-level modules
- Implement: `FileLogger`, `StreamLogger`, `HeaderDecorator`, `DateHeaderDecorator`

---

### Module 04 - Design Patterns

**Concept:** Gang of Four reusable solutions

#### Exercise 00: Preparation
Create base classes: `Person`, `Student`, `Professor`, `Course`, `Room`

#### Exercise 01: Singleton
- `CourseList` and `StudentList` as singletons
- Ensure only one instance exists

#### Exercise 02: Factory & Command
- `Secretary` creates forms (Factory)
- `Headmaster` signs and executes forms (Command)
- Forms: CourseFinished, NeedMoreClassRoom, NeedCourseCreation, SubscriptionToCourse

#### Exercise 03: Mediator
- `Headmaster` coordinates all interactions
- Professors, Students, and Secretary communicate through Headmaster
- Centralized control of school operations

#### Exercise 04: Observer
- `Bell` notifies Professors and Students
- Event-driven architecture
- Break and class resumption events

#### Exercise 05: Facade
- `School` class simplifies complex subsystems
- Single interface for: recruitment, day routine, graduation, etc.

---

### Module 05 - Train Yourself

**Concept:** Apply all learned concepts in a real-world simulation

#### Railway Network Simulation

**Mandatory Features:**
- Parse railway network (nodes + rails)
- Parse train specifications
- Simulate multiple trains simultaneously
- Pathfinding algorithm (Dijkstra/A*)
- Overtaking system to prevent collisions
- Generate detailed travel logs per train

**Technical Requirements:**
- ✓ 3+ Design Patterns implemented
- ✓ 2+ UML Diagrams (class + sequence)
- ✓ SOLID principles applied
- ✓ Proper encapsulation throughout

**Input Files:**

1. **network.txt**: Rail network definition
```
Node CityA
Node CityB
Rail CityA CityB 50.0 250.0
```

2. **trains.txt**: Train specifications
```
TrainAB 80 0.05 356.0 30.0 CityA CityB 14h10 00h10
```

**Output Format:**
```
Train : TrainAB1
Final travel time : 02h15m

[00h00] - [CityA][NodeA] - [53.00km] - [Speed up] - [x][ ][ ][ ]
[00h05] - [CityA][NodeA] - [52.50km] - [Maintain] - [x][ ][ ][ ]
...
```

**Bonuses:**
- ✓ Real-time graphical interface
- ✓ Runtime file reloading
- ✓ Enhanced visualization
- ✓ Statistical analysis (multiple runs)
- ✓ Random event generation
- ✓ Dynamic route optimization

---

## 🔨 Compilation

### General Rules
```bash
# Compiler
c++ -Wall -Wextra -Werror -std=c++98

# Required Makefile rules
make          # Build
make clean    # Remove objects
make fclean   # Remove objects + executable
make re       # Rebuild all
```

### Module-Specific Build
```bash
# Module 00 - Banking
cd module00/ex00
make
./bank

# Module 05 - Railway Simulation
cd module05
make
./railway_sim inputs/network.txt inputs/trains.txt
```

## 🚀 Getting Started
```bash
# Clone repository
git clone <your-repo>
cd opp-piscine

# Start with Module 00
cd module00/ex00
make
./bank

# Progress through modules sequentially
cd ../../module01/ex00
make
./worker
```
