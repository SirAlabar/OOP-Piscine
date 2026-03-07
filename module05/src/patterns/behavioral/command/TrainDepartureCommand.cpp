#include "patterns/behavioral/command/TrainDepartureCommand.hpp"
#include "utils/StringUtils.hpp"
#include "simulation/interfaces/IReplayTarget.hpp"
#include "simulation/core/SimulationContext.hpp"
#include "patterns/behavioral/states/StateRegistry.hpp"
#include "core/Train.hpp"

TrainDepartureCommand::TrainDepartureCommand(double             timestamp,
                                             const std::string& trainName)
    : _timestamp(timestamp),
      _trainName(trainName)
{
}

void TrainDepartureCommand::execute()
{
    // No-op: departure was already applied when recording.
}

std::string TrainDepartureCommand::serialize() const
{
    return StringUtils::serializeHeader(_timestamp)
        + ",\"type\":\"DEPARTURE\""
        + ",\"train\":\"" + _trainName + "\"}";
}

std::string TrainDepartureCommand::getType() const
{
    return "DEPARTURE";
}

double TrainDepartureCommand::getTimestamp() const
{
    return _timestamp;
}

void TrainDepartureCommand::applyReplay(IReplayTarget* target)
{
    if (!target)
    {
        return;
    }

    Train* train = target->findTrain(_trainName);
    if (!train)
    {
        return;
    }

    SimulationContext* ctx = target->getContext();
    if (!ctx)
    {
        return;
    }

    train->setState(ctx->states().accelerating());
}