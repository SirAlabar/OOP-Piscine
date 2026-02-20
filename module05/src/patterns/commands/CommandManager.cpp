#include "patterns/commands/CommandManager.hpp"
#include "patterns/commands/ICommand.hpp"
#include "patterns/commands/TrainDepartureCommand.hpp"
#include "patterns/commands/TrainStateChangeCommand.hpp"
#include "patterns/commands/TrainAdvanceRailCommand.hpp"
#include "patterns/commands/SimEventCommand.hpp"
#include "utils/StringUtils.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

// =============================================================================
// Construction
// =============================================================================

CommandManager::CommandManager()
    : _recording(false),
      _replaying(false)
{
}

CommandManager::~CommandManager()
{
    for (ICommand* cmd : _commands)
    {
        delete cmd;
    }
}

// =============================================================================
// Recording
// =============================================================================

void CommandManager::startRecording()
{
    _recording = true;
    _replaying = false;
}

bool CommandManager::isRecording() const
{
    return _recording;
}

void CommandManager::record(ICommand* cmd)
{
    if (!cmd)
    {
        return;
    }

    cmd->execute();
    _commands.push_back(cmd);
}

// =============================================================================
// Replay
// =============================================================================

void CommandManager::startReplay()
{
    _recording = false;
    _replaying = true;
}

bool CommandManager::isReplaying() const
{
    return _replaying;
}

std::vector<ICommand*> CommandManager::getCommandsForTime(
    double startTime,
    double endTime) const
{
    std::vector<ICommand*> result;

    for (ICommand* cmd : _commands)
    {
        if (!cmd)
        {
            continue;
        }

        double t = cmd->getTimestamp();

        if (t >= startTime && t < endTime)
        {
            result.push_back(cmd);
        }
    }

    return result;
}

// =============================================================================
// Query
// =============================================================================

std::size_t CommandManager::commandCount() const
{
    return _commands.size();
}

// =============================================================================
// Persistence — save
// =============================================================================

bool CommandManager::saveToFile(
    const std::string&      path,
    const RecordingMetadata& meta) const
{
    std::ofstream f(path);
    if (!f.is_open())
    {
        return false;
    }

    f << "{\n";
    f << "\"network_file\":\"" << StringUtils::escapeJson(meta.networkFile) << "\",\n";
    f << "\"train_file\":\""   << StringUtils::escapeJson(meta.trainFile)   << "\",\n";
    f << "\"seed\":"           << meta.seed                                  << ",\n";
    f << "\"stop_time\":"      << meta.stopTime                              << ",\n";
    f << "\"commands\":[\n";

    for (std::size_t i = 0; i < _commands.size(); ++i)
    {
        if (!_commands[i])
        {
            continue;
        }

        f << _commands[i]->serialize();

        if (i + 1 < _commands.size())
        {
            f << ",";
        }

        f << "\n";
    }

    f << "]\n";
    f << "}\n";

    return f.good();
}

// =============================================================================
// Persistence — load
// =============================================================================

bool CommandManager::loadFromFile(
    const std::string& path,
    RecordingMetadata& outMeta)
{
    std::ifstream f(path);
    if (!f.is_open())
    {
        return false;
    }

    // Read entire file into a string.
    std::string content(
        (std::istreambuf_iterator<char>(f)),
        std::istreambuf_iterator<char>());

    // Extract metadata.
    outMeta.networkFile = StringUtils::unescapeJson(extractString(content, "network_file"));
    outMeta.trainFile   = StringUtils::unescapeJson(extractString(content, "train_file"));

    long long seed      = extractInt(content, "seed");
    outMeta.seed        = (seed >= 0) ? static_cast<unsigned int>(seed) : 0u;

    outMeta.stopTime    = extractDouble(content, "stop_time");

    // Find the commands array.
    std::size_t arrayStart = content.find("\"commands\":[");
    if (arrayStart == std::string::npos)
    {
        return false;
    }

    arrayStart = content.find('[', arrayStart);
    if (arrayStart == std::string::npos)
    {
        return false;
    }

    // Walk through JSON objects in the array.
    std::size_t pos = arrayStart + 1;

    while (pos < content.size())
    {
        // Skip whitespace and commas between objects.
        while (pos < content.size()
               && (content[pos] == ' ' || content[pos] == '\n'
                   || content[pos] == '\r' || content[pos] == ','
                   || content[pos] == '\t'))
        {
            ++pos;
        }

        if (pos >= content.size() || content[pos] == ']')
        {
            break;
        }

        if (content[pos] != '{')
        {
            break;
        }

        // Find matching closing brace.
        int    depth    = 0;
        std::size_t end = pos;

        while (end < content.size())
        {
            if (content[end] == '{')
            {
                ++depth;
            }
            else if (content[end] == '}')
            {
                --depth;

                if (depth == 0)
                {
                    ++end;
                    break;
                }
            }

            ++end;
        }

        std::string json = content.substr(pos, end - pos);
        ICommand*   cmd  = deserializeCommand(json);

        if (cmd)
        {
            _commands.push_back(cmd);
        }

        pos = end;
    }

    return true;
}

// =============================================================================
// Deserialization — one command object from JSON
// =============================================================================

ICommand* CommandManager::deserializeCommand(const std::string& json)
{
    std::string type = extractString(json, "type");
    double      t    = extractDouble(json, "t");

    if (type == "DEPARTURE")
    {
        std::string train = extractString(json, "train");
        return new TrainDepartureCommand(t, train);
    }

    if (type == "STATE_CHANGE")
    {
        std::string train = extractString(json, "train");
        std::string from  = extractString(json, "from");
        std::string to    = extractString(json, "to");
        return new TrainStateChangeCommand(t, train, from, to);
    }

    if (type == "ADVANCE_RAIL")
    {
        std::string train     = extractString(json, "train");
        long long   railIndex = extractInt(json, "rail_index");
        if (railIndex < 0)
        {
            return nullptr;
        }
        return new TrainAdvanceRailCommand(
            t, train, static_cast<std::size_t>(railIndex));
    }

    if (type == "EVENT")
    {
        std::string eventType = StringUtils::unescapeJson(extractString(json, "event_type"));
        std::string desc      = StringUtils::unescapeJson(extractString(json, "desc"));
        return new SimEventCommand(t, eventType, desc);
    }

    // "RELOAD" and unknown types are intentionally skipped.
    return nullptr;
}

// =============================================================================
// Minimal JSON helpers
// =============================================================================

// Extract the value of a string field: "key":"value"
std::string CommandManager::extractString(
    const std::string& json,
    const std::string& key)
{
    std::string needle = "\"" + key + "\":\"";
    std::size_t pos    = json.find(needle);

    if (pos == std::string::npos)
    {
        return "";
    }

    pos += needle.size();

    std::string result;

    while (pos < json.size() && json[pos] != '"')
    {
        if (json[pos] == '\\' && pos + 1 < json.size())
        {
            result += json[pos];
            result += json[pos + 1];
            pos += 2;
        }
        else
        {
            result += json[pos];
            ++pos;
        }
    }

    return result;
}

// Extract the value of a numeric (floating-point) field: "key":value
double CommandManager::extractDouble(
    const std::string& json,
    const std::string& key)
{
    std::string needle = "\"" + key + "\":";
    std::size_t pos    = json.find(needle);

    if (pos == std::string::npos)
    {
        return 0.0;
    }

    pos += needle.size();

    // Skip whitespace.
    while (pos < json.size() && json[pos] == ' ')
    {
        ++pos;
    }

    try
    {
        return std::stod(json.substr(pos));
    }
    catch (...)
    {
        return 0.0;
    }
}

// Extract the value of an integer field: "key":value
long long CommandManager::extractInt(
    const std::string& json,
    const std::string& key)
{
    std::string needle = "\"" + key + "\":";
    std::size_t pos    = json.find(needle);

    if (pos == std::string::npos)
    {
        return -1;
    }

    pos += needle.size();

    while (pos < json.size() && json[pos] == ' ')
    {
        ++pos;
    }

    try
    {
        return std::stoll(json.substr(pos));
    }
    catch (...)
    {
        return -1;
    }
}