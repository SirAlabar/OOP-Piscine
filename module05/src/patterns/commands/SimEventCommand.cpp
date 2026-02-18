#include "patterns/commands/SimEventCommand.hpp"
#include "utils/StringUtils.hpp"
#include <sstream>
#include <iomanip>

SimEventCommand::SimEventCommand(double             timestamp,
                                 const std::string& eventType,
                                 const std::string& description)
    : _timestamp(timestamp),
      _eventType(eventType),
      _description(description)
{
}

void SimEventCommand::execute()
{
    // No-op: event was already activated when recording.
}

void SimEventCommand::undo()
{
    // Event effects on rails/nodes are not reversible here.
}

std::string SimEventCommand::getType() const
{
    return "EVENT";
}

double SimEventCommand::getTimestamp() const
{
    return _timestamp;
}

std::string SimEventCommand::serialize() const
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(6);
    ss << "{\"t\":"          << _timestamp
       << ",\"type\":\"EVENT\""
       << ",\"event_type\":\"" << StringUtils::escapeJson(_eventType)   << "\""
       << ",\"desc\":\""       << StringUtils::escapeJson(_description)  << "\"}";
    return ss.str();
}

void SimEventCommand::applyReplay(SimulationManager* /*sim*/)
{
    // Event effects are not re-applied during replay to avoid double-effects.
}