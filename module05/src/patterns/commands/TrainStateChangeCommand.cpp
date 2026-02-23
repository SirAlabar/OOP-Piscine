#include "patterns/commands/TrainStateChangeCommand.hpp"
#include "utils/StringUtils.hpp"
#include "simulation/IReplayTarget.hpp"
#include "simulation/SimulationContext.hpp"
#include "patterns/states/StateRegistry.hpp"
#include "core/Train.hpp"

TrainStateChangeCommand::TrainStateChangeCommand(double             timestamp,
                                                 const std::string& trainName,
                                                 const std::string& fromState,
                                                 const std::string& toState)
    : _timestamp(timestamp),
      _trainName(trainName),
      _fromState(fromState),
      _toState(toState)
{
}

void TrainStateChangeCommand::execute()
{
    // No-op: the transition was already applied by SimulationManager when recording.
}

std::string TrainStateChangeCommand::serialize() const
{
    return StringUtils::serializeHeader(_timestamp)
        + ",\"type\":\"STATE_CHANGE\""
        + ",\"train\":\"" + _trainName + "\""
        + ",\"from\":\""  + _fromState + "\""
        + ",\"to\":\""    + _toState   + "\"}";
}

std::string TrainStateChangeCommand::getType() const
{
    return "STATE_CHANGE";
}

double TrainStateChangeCommand::getTimestamp() const
{
    return _timestamp;
}

void TrainStateChangeCommand::applyReplay(IReplayTarget* target)
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

    SimulationContext* context = target->getContext();

    if (!context)
    {
        return;
    }

    StateRegistry& states = context->states();

    ITrainState* targetState = states.fromName(_toState);

    if (targetState)
    {
        train->setState(targetState);
    }
}