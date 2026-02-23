#include "patterns/commands/ReloadCommand.hpp"
#include "utils/StringUtils.hpp"
#include <fstream>

ReloadCommand::ReloadCommand(double      timestamp,
                             std::string oldNetContent,
                             std::string oldTrainContent,
                             std::string newNetFile,
                             std::string newTrainFile,
                             RebuildFn   rebuildFn)
    : _timestamp(timestamp),
      _oldNetContent(std::move(oldNetContent)),
      _oldTrainContent(std::move(oldTrainContent)),
      _newNetFile(std::move(newNetFile)),
      _newTrainFile(std::move(newTrainFile)),
      _rebuildFn(std::move(rebuildFn))
{
}

void ReloadCommand::execute()
{
    // No-op: Application already performed the rebuild before recording this command.
}

std::string ReloadCommand::serialize() const
{
    // Only record the file paths (not full contents) in the JSON log.
    return StringUtils::serializeHeader(_timestamp)
        + ",\"type\":\"RELOAD\""
        + ",\"net_file\":\""   + _newNetFile   + "\""
        + ",\"train_file\":\"" + _newTrainFile  + "\"}";
}

std::string ReloadCommand::getType() const
{
    return "RELOAD";
}

double ReloadCommand::getTimestamp() const
{
    return _timestamp;
}

void ReloadCommand::applyReplay(SimulationManager* /*sim*/)
{
    // File-reload cannot be replayed deterministically (paths may no longer be valid).
}

std::string ReloadCommand::writeTempFile(const std::string& path, const std::string& content)
{
    std::ofstream f(path);
    if (!f.is_open())
    {
        return "";
    }
    f << content;
    return path;
}