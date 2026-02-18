#include "io/CLI.hpp"
#include <iostream>
#include <sstream>
#include <vector>

CLI::CLI(int argc, char* argv[]) : _argc(argc), _argv(argv)
{
    parseFlags();
}

bool CLI::shouldShowHelp() const
{
    return _argc == 2 && std::string(_argv[1]) == "--help";
}

bool CLI::hasValidArguments() const
{
    return _argc >= 3;
}

std::string CLI::getNetworkFile() const
{
    return (_argc >= 2) ? std::string(_argv[1]) : "";
}

std::string CLI::getTrainFile() const
{
    return (_argc >= 3) ? std::string(_argv[2]) : "";
}

void CLI::printUsage(const std::string& programName) const
{
    std::cout << "Usage: " << programName << " <network_file> <train_file>" << std::endl;
    std::cout << "       " << programName << " --help" << std::endl;
}

void CLI::printHelp() const
{
    std::cout << "\n========================================\n";
    std::cout << "  RAILWAY SIMULATION - FILE FORMAT GUIDE\n";
    std::cout << "========================================\n\n";

    std::cout << "USAGE:\n";
    std::cout << "  ./railway_sim <network_file> <train_file>\n\n";

    std::cout << "----------------------------------------\n";
    std::cout << "NETWORK FILE FORMAT:\n";
    std::cout << "----------------------------------------\n";
    std::cout << "Node declaration:\n";
    std::cout << "  Node <n>\n\n";
    std::cout << "  Example:\n";
    std::cout << "    Node CityA\n";
    std::cout << "    Node CityB\n";
    std::cout << "    Node RailNodeA\n\n";

    std::cout << "Rail declaration:\n";
    std::cout << "  Rail <nodeA> <nodeB> <length_km> <speed_limit_kmh>\n\n";
    std::cout << "  Example:\n";
    std::cout << "    Rail CityA RailNodeA 15.0 250.0\n";
    std::cout << "    Rail RailNodeA CityB 20.0 200.0\n\n";

    std::cout << "Rules:\n";
    std::cout << "  - Node names must be unique\n";
    std::cout << "  - Rails must connect two existing nodes\n";
    std::cout << "  - Length and speed limit must be positive\n\n";

    std::cout << "----------------------------------------\n";
    std::cout << "TRAIN FILE FORMAT:\n";
    std::cout << "----------------------------------------\n";
    std::cout << "Train declaration (9 fields, space-separated):\n";
    std::cout << "  <n> <mass_tons> <friction_coef> <max_accel_kN> <max_brake_kN> ";
    std::cout << "<departure_station> <arrival_station> <departure_time> <stop_duration>\n\n";
    std::cout << "  Example:\n";
    std::cout << "    TrainAB 80 0.05 356.0 500.0 CityA CityB 14h10 00h10\n\n";

    std::cout << "----------------------------------------\n";
    std::cout << "OPTIONAL FLAGS:\n";
    std::cout << "----------------------------------------\n";
    std::cout << "  --seed=N              Set random seed for deterministic events\n";
    std::cout << "  --pathfinding=astar   Use A* pathfinding (default: dijkstra)\n";
    std::cout << "  --render              Enable SFML visualization\n";
    std::cout << "  --hot-reload          Watch input files for changes (requires --render)\n";
    std::cout << "  --round-trip          Trains reverse at destination (indefinite)\n";
    std::cout << "  --monte-carlo=N       Run N simulations and output statistics\n";
    std::cout << "  --record              Record simulation commands to output/replay.json\n";
    std::cout << "  --replay=file         Replay a previously recorded session\n\n";

    std::cout << "Examples:\n";
    std::cout << "  ./railway_sim network.txt trains.txt --seed=42 --record --render\n";
    std::cout << "  ./railway_sim network.txt trains.txt --replay=output/replay.json --render\n\n";

    std::cout << "========================================\n\n";
}

void CLI::parseFlags()
{
    for (int i = 3; i < _argc; ++i)
    {
        std::string arg = _argv[i];
        std::string key, value;

        if (parseFlag(arg, key, value))
        {
            _flags[key] = value;
        }
        else
        {
            std::cerr << "Warning: Invalid flag format: " << arg << std::endl;
        }
    }
}

bool CLI::parseFlag(const std::string& arg, std::string& key, std::string& value)
{
    if (arg.size() < 3 || arg[0] != '-' || arg[1] != '-')
    {
        return false;
    }

    std::size_t pos = arg.find('=');

    if (pos != std::string::npos)
    {
        key   = arg.substr(2, pos - 2);
        value = arg.substr(pos + 1);
    }
    else
    {
        key   = arg.substr(2);
        value = "true";
    }

    return !key.empty();
}

bool CLI::hasSeed() const
{
    return _flags.find("seed") != _flags.end();
}

unsigned int CLI::getSeed() const
{
    if (!hasSeed()) { return 42; }
    std::istringstream ss(_flags.at("seed"));
    unsigned int seed = 0;
    ss >> seed;
    return seed;
}

std::string CLI::getPathfinding() const
{
    auto it = _flags.find("pathfinding");
    return (it != _flags.end()) ? it->second : "dijkstra";
}

bool CLI::hasRender()         const { return _flags.find("render")       != _flags.end(); }
bool CLI::hasHotReload()      const { return _flags.find("hot-reload")   != _flags.end(); }
bool CLI::hasRoundTrip()      const { return _flags.find("round-trip")   != _flags.end(); }
bool CLI::hasRecord()         const { return _flags.find("record")       != _flags.end(); }
bool CLI::hasReplay()         const { return _flags.find("replay")       != _flags.end(); }

std::string CLI::getReplayFile() const
{
    auto it = _flags.find("replay");
    return (it != _flags.end()) ? it->second : "";
}

bool CLI::hasMonteCarloRuns() const { return _flags.find("monte-carlo") != _flags.end(); }

unsigned int CLI::getMonteCarloRuns() const
{
    if (!hasMonteCarloRuns()) { return 0; }
    std::istringstream ss(_flags.at("monte-carlo"));
    unsigned int runs = 0;
    ss >> runs;
    return runs;
}

bool CLI::validateFlags(std::string& errorMsg) const
{
    const std::vector<std::string> validFlags = {
        "seed", "pathfinding", "render", "hot-reload",
        "monte-carlo", "round-trip", "record", "replay"
    };

    for (const auto& pair : _flags)
    {
        bool found = false;
        for (const auto& valid : validFlags)
        {
            if (pair.first == valid) { found = true; break; }
        }
        if (!found)
        {
            errorMsg = "Unknown flag: --" + pair.first;
            return false;
        }
    }

    if (_flags.find("pathfinding") != _flags.end())
    {
        const std::string& algo = _flags.at("pathfinding");
        if (algo != "dijkstra" && algo != "astar")
        {
            errorMsg = "Invalid pathfinding algorithm: '" + algo + "' (must be 'dijkstra' or 'astar')";
            return false;
        }
    }

    if (_flags.find("seed") != _flags.end())
    {
        const std::string& seedStr = _flags.at("seed");
        if (!seedStr.empty() && seedStr[0] == '-')
        {
            errorMsg = "Invalid seed value: '" + seedStr + "' (must be a positive integer)";
            return false;
        }
        std::istringstream ss(seedStr);
        unsigned int seed = 0;
        if (!(ss >> seed) || !ss.eof())
        {
            errorMsg = "Invalid seed value: '" + seedStr + "' (must be a positive integer)";
            return false;
        }
    }

    if (_flags.find("monte-carlo") != _flags.end())
    {
        const std::string& mcStr = _flags.at("monte-carlo");
        if (!mcStr.empty() && mcStr[0] == '-')
        {
            errorMsg = "Invalid monte-carlo value: '" + mcStr + "' (must be a positive integer)";
            return false;
        }
        std::istringstream ss(mcStr);
        unsigned int runs = 0;
        if (!(ss >> runs) || !ss.eof() || runs == 0)
        {
            errorMsg = "Invalid monte-carlo value: '" + mcStr + "' (must be a positive integer)";
            return false;
        }
    }

    // --replay requires a non-empty value
    if (_flags.find("replay") != _flags.end())
    {
        if (_flags.at("replay").empty() || _flags.at("replay") == "true")
        {
            errorMsg = "Flag --replay requires a file path (e.g. --replay=output/replay.json)";
            return false;
        }
    }

    // --record and --replay are mutually exclusive
    if (_flags.find("record") != _flags.end() && _flags.find("replay") != _flags.end())
    {
        errorMsg = "Flags --record and --replay cannot be used simultaneously";
        return false;
    }

    return true;
}