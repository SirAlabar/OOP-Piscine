#ifndef TRAINTRANSITIONSTATE_HPP
#define TRAINTRANSITIONSTATE_HPP

class Train;
class SimulationContext;
class ITrainState;

// Shared transition logic reused by multiple train states.
// Each function returns the target state if a transition is required,
// or nullptr to remain in the current state.
namespace TrainTransitionState
{
    // Evaluates leader-train interaction for a train that is moving
    // (Accelerating or Cruising).  Priority arbitration is delegated to the
    // TrafficController; if access is denied and the gap is unsafe, braking is
    // triggered.
    // Must be called AFTER the emergency-zone check.
    ITrainState* checkLeaderInteraction(Train* train, SimulationContext* ctx);

    // Evaluates whether a stopped or waiting train may resume by requesting
    // rail access through the TrafficController.
    // Returns accelerating state on GRANT, waiting state on DENY (for Stopped),
    // or nullptr to stay in current state (for Waiting).
    ITrainState* checkRailAccessForResume(Train* train, SimulationContext* ctx);
}

#endif