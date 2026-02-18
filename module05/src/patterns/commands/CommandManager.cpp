#include "patterns/commands/CommandManager.hpp"
#include "patterns/commands/ICommand.hpp"
#include "patterns/commands/TrainStateChangeCommand.hpp"
#include "patterns/commands/TrainDepartureCommand.hpp"
#include "patterns/commands/TrainAdvanceRailCommand.hpp"
#include "patterns/commands/SimEventCommand.hpp"
#include "patterns/commands/ReloadCommand.hpp"
#include "utils/StringUtils.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

CommandManager::CommandManager()
    : _mode(Mode::IDLE),
      _replayIndex(0)
{
}

CommandManager::~CommandManager()
{
    for (ICommand* cmd : _commands)
    {
        delete cmd;
    }
}

// Record
void CommandManager::startRecording()
{
    _mode        = Mode::RECORD;
    _replayIndex = 0;

    for (ICommand* cmd : _commands)
    {
        delete cmd;
    }
    _commands.clear();
}

void CommandManager::record(ICommand* cmd)
{
    if (_mode != Mode::RECORD || !cmd)
    {
        delete cmd;
        return;
    }

    _commands.push_back(cmd);
}

void CommandManager::saveToFile(const std::string& path, const RecordingMetadata& meta) const
{
    std::ofstream f(path);
    if (!f.is_open())
    {
        throw std::runtime_error("CommandManager: cannot open file for writing: " + path);
    }

    f << "{\n";
    f << "  \"version\": 1,\n";
    f << "  \"network_file\": \"" << StringUtils::escapeJson(meta.networkFile) << "\",\n";
    f << "  \"train_file\": \""   << StringUtils::escapeJson(meta.trainFile)   << "\",\n";
    f << "  \"seed\": "           << meta.seed                                 << ",\n";
    f << "  \"commands\": [\n";

    for (std::size_t i = 0; i < _commands.size(); ++i)
    {
        f << "    " << _commands[i]->serialize();
        if (i + 1 < _commands.size())
        { 
            f << ","; 
        }
        f << "\n";
    }

    f << "  ]\n";
    f << "}\n";
}

// Replay
bool CommandManager::loadFromFile(const std::string& path, RecordingMetadata& outMeta)
{
    std::ifstream f(path);
    if (!f.is_open())
    {
        return false;
    }

    for (ICommand* cmd : _commands)
    {
        delete cmd;
    }
    _commands.clear();
    _replayIndex = 0;
    outMeta      = RecordingMetadata{};

    std::string line;
    bool        inCommands = false;

    while (std::getline(f, line))
    {
        std::string t = StringUtils::trim(line);

        if (t.empty() || t == "{" || t == "}" || t == "]" || t == "]," )
        {
            continue;
        }

        if (t.find("\"commands\"") != std::string::npos)
        {
            inCommands = true;
            continue;
        }

        if (!inCommands)
        {
            // Parse top-level metadata key: value lines
            KVMap kv = parseJsonObject("{" + t + "}");
            for (const auto& pair : kv)
            {
                if      (pair.first == "network_file")
                {
                    outMeta.networkFile = StringUtils::unescapeJson(pair.second);
                }
                else if (pair.first == "train_file")
                {
                    outMeta.trainFile   = StringUtils::unescapeJson(pair.second);
                }
                else if (pair.first == "seed")
                {
                    std::istringstream ss(pair.second);
                    ss >> outMeta.seed;
                }
            }
        }
        else
        {
            if (!t.empty() && t.back() == ',')
            {
                t.pop_back();
            }
            if (t.front() != '{' || t.back() != '}')
            {
                continue;
            }

            KVMap     kv  = parseJsonObject(t);
            ICommand* cmd = createCommandFromKV(kv);
            if (cmd)
            { 
                _commands.push_back(cmd);
            }
        }
    }

    _mode = Mode::IDLE;
    return true;
}

void CommandManager::startReplay()
{
    _mode        = Mode::REPLAY;
    _replayIndex = 0;
}

std::vector<ICommand*> CommandManager::getCommandsForTime(double tFrom, double tTo)
{
    std::vector<ICommand*> result;

    if (_mode != Mode::REPLAY)
    {
        return result;
    }

    while (_replayIndex < _commands.size())
    {
        double t = _commands[_replayIndex]->getTimestamp();

        if (t >= tFrom && t < tTo)
        {
            result.push_back(_commands[_replayIndex]);
            ++_replayIndex;
        }
        else if (t >= tTo)
        {
            break;
        }
        else
        {
            ++_replayIndex;
        }
    }

    return result;
}

// Undo
bool CommandManager::undoLast()
{
    if (_commands.empty())
    {
        return false;
    }
    _commands.back()->undo();
    return true;
}

CommandManager::Mode CommandManager::getMode() const
{
    return _mode;
}

bool CommandManager::isRecording() const
{
    return _mode == Mode::RECORD;
}

bool CommandManager::isReplaying() const
{
    return _mode == Mode::REPLAY;
}

std::size_t CommandManager::commandCount() const
{
    return _commands.size();
}


// JSON helpers  (use StringUtils for string ops; only parsing logic lives here)
CommandManager::KVMap CommandManager::parseJsonObject(const std::string& line)
{
    KVMap result;

    std::size_t bOpen  = line.find('{');
    std::size_t bClose = line.rfind('}');

    if (bOpen == std::string::npos || bClose == std::string::npos || bOpen >= bClose)
    {
        return result;
    }

    std::string inner = line.substr(bOpen + 1, bClose - bOpen - 1);

    // Split by comma, respecting quoted strings
    std::vector<std::string> tokens;
    {
        bool        inQuote = false;
        std::size_t start   = 0;

        for (std::size_t i = 0; i < inner.size(); ++i)
        {
            if (inner[i] == '"' && (i == 0 || inner[i - 1] != '\\'))
            {
                inQuote = !inQuote;
            }
            else if (inner[i] == ',' && !inQuote)
            {
                tokens.push_back(StringUtils::trim(inner.substr(start, i - start)));
                start = i + 1;
            }
        }

        if (start < inner.size())
        {
            tokens.push_back(StringUtils::trim(inner.substr(start)));
        }
    }

    for (const std::string& token : tokens)
    {
        std::size_t colon = token.find(':');
        if (colon == std::string::npos)
        {
            continue;
        }

        std::string rawKey   = StringUtils::trim(token.substr(0, colon));
        std::string rawValue = StringUtils::trim(token.substr(colon + 1));

        // Unquote key
        if (rawKey.size() >= 2 && rawKey.front() == '"' && rawKey.back() == '"')
        {
            rawKey = rawKey.substr(1, rawKey.size() - 2);
        }

        // Unquote value (strings only)
        if (rawValue.size() >= 2 && rawValue.front() == '"' && rawValue.back() == '"')
        {
            rawValue = rawValue.substr(1, rawValue.size() - 2);
        }

        result.push_back({rawKey, rawValue});
    }

    return result;
}

ICommand* CommandManager::createCommandFromKV(const KVMap& kv)
{
    auto get = [&](const std::string& key) -> std::string
    {
        for (const auto& pair : kv)
        {
            if (pair.first == key)
            {
                return pair.second;
            }
        }
        return "";
    };

    std::string type = get("type");
    double      t    = 0.0;
    {
        std::istringstream ss(get("t"));
        ss >> t;
    }

    if (type == "STATE_CHANGE")
    {
        return new TrainStateChangeCommand(t, get("train"), get("from"), get("to"));
    }

    if (type == "DEPARTURE")
    {
        return new TrainDepartureCommand(t, get("train"));
    }

    if (type == "ADVANCE_RAIL")
    {
        std::size_t idx = 0;
        {
            std::istringstream ss(get("rail_index"));
            ss >> idx;
        }
        return new TrainAdvanceRailCommand(t, get("train"), idx);
    }

    if (type == "EVENT")
    {
        return new SimEventCommand(t,
            StringUtils::unescapeJson(get("event_type")),
            StringUtils::unescapeJson(get("desc")));
    }

    if (type == "RELOAD")
    {
        return new ReloadCommand(t, "", "", get("net_file"), get("train_file"), nullptr);
    }

    return nullptr;
}