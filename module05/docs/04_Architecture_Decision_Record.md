# 📋 ARCHITECTURE DECISION RECORD
*Railway Simulation Engine - Design Rationale*

---

## ADR-001: Layer Separation

**Context:** Simulation, IO, and rendering have different responsibilities.

**Decision:** Separate into 4 layers:
- **Engine:** Core simulation (physics, trains, graph)
- **IO:** File parsing and output generation
- **Rendering:** SFML visualization (bonus)
- **App:** CLI interface and orchestration

**Rationale:**
- Each layer has single responsibility
- Easy to test independently
- Can run headless (no rendering)
- Swap rendering library easily

**Trade-offs:**
- ✅ Clean interfaces, testable, modular
- ❌ More directories, more abstractions

**Consequences:**
- Rendering can't modify simulation state
- IO validates before passing to Engine
- Clear dependency flow: App → Engine/IO/Rendering

---

## ADR-002: State Pattern for Train Behavior

**Context:** Train behavior changes drastically: idle → accelerating → cruising → braking → stopped → waiting.

**Decision:** Use State Pattern with 6 state classes.

**Rationale:**
- Each state handles own physics logic
- No giant if/else in Train.update()
- Easy to add new states (EmergencyState)
- Clear state transitions

**Trade-offs:**
- ✅ Clean code, easy extension, isolated logic
- ❌ More classes (6 vs 1), virtual function overhead

**Consequences:**
- Train.update() just calls currentState->update(this, dt)
- Each state tested independently
- Adding states doesn't change Train class

---

## ADR-003: Strategy Pattern for Pathfinding

**Context:** Need Dijkstra (mandatory) and A* (bonus). May add more algorithms.

**Decision:** Use Strategy Pattern with IPathfindingStrategy interface.

**Rationale:**
- Swap algorithms at runtime: `--pathfinding=dijkstra` or `--pathfinding=astar`
- Easy to add new algorithms (BidirectionalSearch)
- Can benchmark different algorithms

**Trade-offs:**
- ✅ Swappable, extensible, testable
- ❌ Interface overhead, one extra layer

**Consequences:**
- PathFinder uses interface, not concrete algorithm
- CLI supports algorithm selection
- Easy A/B performance testing

---

## ADR-004: Deterministic Simulation

**Context:** Need reproducible results for debugging, testing, and evaluation.

**Decision:**
- Fixed timestep (Δt = 1 second)
- Seeded RNG: `--seed=42`
- All randomness from SeededRNG class

**Rationale:**
- Same seed + inputs = identical outputs always
- Bugs reproducible
- Regression tests with golden outputs
- Evaluators can verify behavior

**Trade-offs:**
- ✅ Perfect reproducibility, easy debugging
- ❌ Less continuous than variable timestep

**Consequences:**
- Replay system just stores seed + inputs
- Tests compare against expected outputs
- Output files include seed used

---

## ADR-005: Technology Choices

| Technology | Why |
|-----------|-----|
| **C++17** | Modern features (auto, smart pointers, structured bindings) while maintaining performance |
| **SFML** | Lightweight 2D rendering, easy setup |
| **Mermaid** | UML in markdown, GitHub renders it |
| **CMake** | Cross-platform builds, industry standard |

**Rationale:**
- C++17: Modern C++ features (std::optional, std::variant, filesystem) with zero-cost abstractions
- SFML: Simpler than OpenGL, perfect for 2D network view
- Mermaid: Version-controlled diagrams, no external tools
- CMake: Works everywhere, IDE support

---

## ADR-006: File Structure

```
railway-simulation/
├── docs/           # All design documents
├── include/
│   ├── core/       # Graph, Node, Rail, Train
│   ├── simulation/ # SimulationManager, Physics, Collision
│   ├── patterns/   # factories, strategies, states, observers, commands
│   ├── io/         # Parsers, output writers
│   ├── rendering/  # SFML visualization
│   └── utils/      # SeededRNG, Time, Logger
├── src/            # Implementation files
├── tests/
│   ├── unit/       # Individual class tests
│   └── integration/# Full simulation tests
└── examples/       # Sample network/train files
```

**Rationale:**
- Organized by concern (core, simulation, patterns)
- Patterns folder shows design pattern usage clearly
- Tests mirror source structure
- Easy navigation

---

## ADR-007: Testing Strategy

**Four testing layers:**

1. **Unit Tests:** Individual classes (Graph, PathFinding, Physics)
   - GoogleTest framework
   - Fast, isolated tests

2. **Regression Tests:** Full simulations with golden outputs
   - Fixed seed ensures same results
   - Detect behavioral changes

3. **Scenario Tests:** Complex multi-train scenarios
   - Validates collision avoidance
   - Tests event handling

4. **Performance Tests:** Timing and resource usage
   - Compare pathfinding algorithms
   - Measure simulation speed

**Rationale:**
- Confidence in correctness
- Safe refactoring
- Bugs caught early
- Professional practice demonstration

---



## ADR-008: Railway Domain Modeling Assumptions

**Context:** The project specification contains ambiguities regarding rail directionality, node semantics, and train identification. These decisions affect core data structures, pathfinding, and simulation determinism.

**Decision:**
- Rails are modeled as **single-track bidirectional segments** with mutual exclusion occupancy (only one train per rail at a time).
- A **single Node class** is used. Node roles (City vs Junction) are represented by an optional `NodeType` enumeration.
- Train identifiers are **sequential deterministic integers** assigned in input order.

**Rationale:**
- Simplifies graph representation and collision detection logic.
- Avoids unnecessary inheritance complexity in node hierarchy.
- Ensures deterministic simulation runs and reproducible results.

**Trade-offs:**
- Multi-track rail modeling is not supported.
- Node specialization requires future extension if richer station logic is needed.
- Train IDs are predictable but not globally unique beyond a single run.

**Consequences:**
- Pathfinding treats rails as bidirectional edges.
- CollisionAvoidance enforces exclusive rail occupancy.
- Simulation outputs remain identical across runs given identical input files.

---

## SUMMARY

These decisions create a **clean, testable, extensible** architecture:

✅ **Clarity:** Separated layers, explicit patterns  
✅ **Quality:** Comprehensive testing, deterministic  
✅ **Maintainability:** Modular structure, isolated responsibilities  
✅ **Extensibility:** Open/Closed principle throughout  
✅ **Professionalism:** Industry tools and practices  

The architecture balances academic requirements (patterns, UML, OOP) with real engineering (testing, performance, maintainability).