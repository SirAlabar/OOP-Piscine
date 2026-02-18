#include "patterns/commands/ReloadCommand.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>

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

void ReloadCommand::undo()
{
    // Write the saved pre-reload contents to temp files, then invoke the rebuild callback.
    static const std::string TMP_NET   = "/tmp/railway_undo_network.txt";
    static const std::string TMP_TRAIN = "/tmp/railway_undo_trains.txt";

    if (writeTempFile(TMP_NET,   _oldNetContent).empty())
    {
        return;
    }
    if (writeTempFile(TMP_TRAIN, _oldTrainContent).empty())
    {
        return;
    }
    if (_rebuildFn)
    {
        _rebuildFn(TMP_NET, TMP_TRAIN);
    }
}

std::string ReloadCommand::serialize() const
{
    // Only record the file paths (not full contents) in the JSON log.
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(6);
    ss << "{\"t\":" << _timestamp
       << ",\"type\":\"RELOAD\""
       << ",\"net_file\":\""   << _newNetFile   << "\""
       << ",\"train_file\":\"" << _newTrainFile  << "\"}";
    return ss.str();
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