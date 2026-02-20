#include "patterns/commands/TrainDepartureCommand.hpp"
#include "simulation/SimulationManager.hpp"
#include "simulation/SimulationContext.hpp"
#include "patterns/states/StateRegistry.hpp"
#include "core/Train.hpp"
#include <sstream>
#include <iomanip>

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
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(6);
    ss << "{\"t\":"     << _timestamp
       << ",\"type\":\"DEPARTURE\""
       << ",\"train\":\"" << _trainName << "\"}";
    return ss.str();
}

std::string TrainDepartureCommand::getType() const
{
    return "DEPARTURE";
}

double TrainDepartureCommand::getTimestamp() const
{
    return _timestamp;
}

void TrainDepartureCommand::applyReplay(SimulationManager* sim)
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

    SimulationContext* ctx = sim->getContext();
    if (!ctx)
    {
        return;
    }

    train->setState(ctx->states().accelerating());
}