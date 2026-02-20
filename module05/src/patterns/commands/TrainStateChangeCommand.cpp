#include "patterns/commands/TrainStateChangeCommand.hpp"
#include "simulation/SimulationManager.hpp"
#include "simulation/SimulationContext.hpp"
#include "patterns/states/StateRegistry.hpp"
#include "core/Train.hpp"
#include <sstream>
#include <iomanip>

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
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(6);
    ss << "{\"t\":"     << _timestamp
       << ",\"type\":\"STATE_CHANGE\""
       << ",\"train\":\"" << _trainName << "\""
       << ",\"from\":\""  << _fromState << "\""
       << ",\"to\":\""    << _toState   << "\"}";
    return ss.str();
}

std::string TrainStateChangeCommand::getType() const
{
    return "STATE_CHANGE";
}

double TrainStateChangeCommand::getTimestamp() const
{
    return _timestamp;
}

void TrainStateChangeCommand::applyReplay(SimulationManager* sim)
{
    if (!sim)
    {
        return;
    }

    Train* train = sim->findTrain(_trainName);

    if (!train)
    {
        return;
    }

    SimulationContext* context = sim->getContext();

    if (!context)
    {
        return;
    }

    StateRegistry& states = context->states();

    ITrainState* targetState = nullptr;

    if (_toState == "Idle")
    {
        targetState = states.idle();
    }
    else if (_toState == "Accelerating")
    {
        targetState = states.accelerating();
    }
    else if (_toState == "Cruising")
    {
        targetState = states.cruising();
    }
    else if (_toState == "Waiting")
    {
        targetState = states.waiting();
    }
    else if (_toState == "Braking")
    {
        targetState = states.braking();
    }
    else if (_toState == "Stopped")
    {
        targetState = states.stopped();
    }
    else if (_toState == "Emergency")
    {
        targetState = states.emergency();
    }

    if (targetState)
    {
        train->setState(targetState);
    }
}
