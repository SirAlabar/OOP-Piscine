#include "patterns/states/TrainTransitionState.hpp"
#include "patterns/states/StateRegistry.hpp"
#include "patterns/mediator/TrafficController.hpp"
#include "simulation/SimulationContext.hpp"
#include "simulation/RiskData.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"

namespace TrainTransitionState
{

ITrainState* checkLeaderInteraction(Train* train, SimulationContext* ctx)
{
    if (!train || !ctx)
    {
        return nullptr;
    }

    const RiskData& risk = ctx->getRisk(train);

    if (!risk.hasLeader())
    {
        return nullptr;
    }

    TrafficController* controller = ctx->getTrafficController();
    Rail*              currentRail = train->getCurrentRail();

    if (controller && currentRail)
    {
        if (controller->requestRailAccess(train, currentRail) == TrafficController::GRANT)
        {
            // Higher priority than the leader — continue.
            return nullptr;
        }
    }

    // No controller (fallback) or access denied: yield to leader.
    if (risk.leader->getVelocity() < 0.1 || risk.gap < risk.safeDistance)
    {
        return ctx->states().braking();
    }

    return nullptr;
}

ITrainState* checkRailAccessForResume(Train* train, SimulationContext* ctx)
{
    if (!train || !ctx)
    {
        return nullptr;
    }

    Rail* currentRail = train->getCurrentRail();
    if (!currentRail)
    {
        return nullptr;
    }

    TrafficController* controller = ctx->getTrafficController();
    if (!controller)
    {
        return nullptr;
    }

    if (controller->requestRailAccess(train, currentRail) == TrafficController::GRANT)
    {
        return ctx->states().accelerating();
    }

    return nullptr;
}

} // namespace TrainTransitionState