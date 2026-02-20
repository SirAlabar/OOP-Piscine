#include "patterns/commands/TrainAdvanceRailCommand.hpp"
#include "simulation/SimulationManager.hpp"
#include "core/Train.hpp"
#include <sstream>
#include <iomanip>

TrainAdvanceRailCommand::TrainAdvanceRailCommand(double             timestamp,
                                                 const std::string& trainName,
                                                 std::size_t        railIndex)
    : _timestamp(timestamp),
      _trainName(trainName),
      _railIndex(railIndex)
{
}

void TrainAdvanceRailCommand::execute()
{
    // No-op: advancement was already applied when recording.
}

std::string TrainAdvanceRailCommand::serialize() const
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(6);
    ss << "{\"t\":" << _timestamp
       << ",\"type\":\"ADVANCE_RAIL\""
       << ",\"train\":\"" << _trainName << "\""
       << ",\"rail_index\":" << _railIndex << "}";
    return ss.str();
}

std::string TrainAdvanceRailCommand::getType() const
{
    return "ADVANCE_RAIL";
}

double TrainAdvanceRailCommand::getTimestamp() const
{
    return _timestamp;
}

void TrainAdvanceRailCommand::applyReplay(SimulationManager* sim)
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

    // Advance the train until it reaches the recorded rail index.
    // This handles cases where multiple advances fire at the same timestamp.
    while (train->getCurrentRailIndex() < _railIndex)
    {
        train->advanceToNextRail();
        train->setPosition(0.0);
    }
}