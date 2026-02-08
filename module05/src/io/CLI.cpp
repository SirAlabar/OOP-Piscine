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
	// Need at least 3 args: program_name network_file train_file
	// Additional args are optional flags
	return _argc >= 3;
}

std::string CLI::getNetworkFile() const
{
	if (_argc >= 2)
	{
		return std::string(_argv[1]);
	}
	return "";
}

std::string CLI::getTrainFile() const
{
	if (_argc >= 3)
	{
		return std::string(_argv[2]);
	}
	return "";
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
	std::cout << "The network file describes the railway graph structure.\n\n";
	
	std::cout << "Node declaration:\n";
	std::cout << "  Node <name>\n\n";
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
	std::cout << "The train file describes each train configuration.\n\n";
	
	std::cout << "Train declaration (9 fields, space-separated):\n";
	std::cout << "  <name> <mass_tons> <friction_coef> <max_accel_kN> <max_brake_kN> ";
	std::cout << "<departure_station> <arrival_station> <departure_time> <stop_duration>\n\n";
	
	std::cout << "  Example:\n";
	std::cout << "    TrainAB 80 0.05 356.0 500.0 CityA CityB 14h10 00h10\n";
	std::cout << "    TrainAC 60 0.05 412.0 400.0 CityA CityC 14h20 00h05\n\n";
	
	std::cout << "Field descriptions:\n";
	std::cout << "  1. name               - Train identifier\n";
	std::cout << "  2. mass_tons          - Train mass in metric tons\n";
	std::cout << "  3. friction_coef      - Coefficient of friction (e.g., 0.05)\n";
	std::cout << "  4. max_accel_kN       - Maximum acceleration force in kilonewtons\n";
	std::cout << "  5. max_brake_kN       - Maximum braking force in kilonewtons\n";
	std::cout << "  6. departure_station  - Starting station (must exist in network)\n";
	std::cout << "  7. arrival_station    - Destination station (must exist in network)\n";
	std::cout << "  8. departure_time     - Time format: HHhMM (e.g., 14h10)\n";
	std::cout << "  9. stop_duration      - Stop time at each station: HHhMM (e.g., 00h10)\n\n";
	
	std::cout << "Rules:\n";
	std::cout << "  - All numeric values must be positive\n";
	std::cout << "  - Departure and arrival stations must exist in the network\n";
	std::cout << "  - Time format must be HHhMM (hours: 00-23, minutes: 00-59)\n";
	std::cout << "  - A valid path must exist between departure and arrival stations\n\n";
	
	std::cout << "----------------------------------------\n";
	std::cout << "OPTIONAL FLAGS:\n";
	std::cout << "----------------------------------------\n";
	std::cout << "  --seed=N              Set random seed for deterministic events\n";
	std::cout << "  --pathfinding=astar   Use A* pathfinding (default: dijkstra)\n";
	std::cout << "  --render              Enable SFML visualization\n";
	std::cout << "  --hot-reload          Watch input files for changes\n";
	std::cout << "  --monte-carlo=N       Run N simulations and output statistics\n\n";
	
	std::cout << "Example:\n";
	std::cout << "  ./railway_sim network.txt trains.txt --seed=42 --pathfinding=astar\n\n";
	
	std::cout << "========================================\n\n";
}

void CLI::parseFlags()
{
	// Parse optional flags starting from index 3 (after network_file and train_file)
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
	// Check if starts with --
	if (arg.size() < 3 || arg[0] != '-' || arg[1] != '-')
	{
		return false;
	}
	
	// Find = separator
	size_t pos = arg.find('=');
	
	if (pos != std::string::npos)
	{
		// Format: --key=value
		key = arg.substr(2, pos - 2);
		value = arg.substr(pos + 1);
	}
	else
	{
		// Format: --key (boolean flag)
		key = arg.substr(2);
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
	if (!hasSeed())
	{
		return 42;  // Default seed
	}
	
	std::stringstream ss(_flags.at("seed"));
	unsigned int seed;
	ss >> seed;
	return seed;
}

std::string CLI::getPathfinding() const
{
	if (_flags.find("pathfinding") != _flags.end())
	{
		return _flags.at("pathfinding");
	}
	return "dijkstra";  // Default
}

bool CLI::hasRender() const
{
	return _flags.find("render") != _flags.end();
}

bool CLI::hasHotReload() const
{
	return _flags.find("hot-reload") != _flags.end();
}

bool CLI::hasMonteCarloRuns() const
{
	return _flags.find("monte-carlo") != _flags.end();
}

unsigned int CLI::getMonteCarloRuns() const
{
	if (!hasMonteCarloRuns())
	{
		return 0;
	}
	
	std::stringstream ss(_flags.at("monte-carlo"));
	unsigned int runs;
	ss >> runs;
	return runs;
}

bool CLI::validateFlags(std::string& errorMsg) const
{
	// Check for unknown flags
	const std::vector<std::string> validFlags = {
		"seed", "pathfinding", "render", "hot-reload", "monte-carlo"
	};
	
	for (const auto& pair : _flags)
	{
		const std::string& flag = pair.first;
		bool found = false;
		
		for (const auto& validFlag : validFlags)
		{
			if (flag == validFlag)
			{
				found = true;
				break;
			}
		}
		
		if (!found)
		{
			errorMsg = "Unknown flag: --" + flag;
			return false;
		}
	}
	
	// Validate pathfinding value
	if (_flags.find("pathfinding") != _flags.end())
	{
		std::string algo = _flags.at("pathfinding");
		if (algo != "dijkstra" && algo != "astar")
		{
			errorMsg = "Invalid pathfinding algorithm: '" + algo + "' (must be 'dijkstra' or 'astar')";
			return false;
		}
	}
	
	// Validate seed is numeric
	if (_flags.find("seed") != _flags.end())
	{
		std::string seedStr = _flags.at("seed");
		
		// Check for negative sign
		if (!seedStr.empty() && seedStr[0] == '-')
		{
			errorMsg = "Invalid seed value: '" + seedStr + "' (must be a positive integer)";
			return false;
		}
		
		std::stringstream ss(seedStr);
		unsigned int seed;
		if (!(ss >> seed) || !ss.eof())
		{
			errorMsg = "Invalid seed value: '" + seedStr + "' (must be a positive integer)";
			return false;
		}
	}
	
	// Validate monte-carlo is numeric and positive
	if (_flags.find("monte-carlo") != _flags.end())
	{
		std::string mcStr = _flags.at("monte-carlo");
		
		// Check for negative sign
		if (!mcStr.empty() && mcStr[0] == '-')
		{
			errorMsg = "Invalid monte-carlo value: '" + mcStr + "' (must be a positive integer)";
			return false;
		}
		
		std::stringstream ss(mcStr);
		unsigned int runs;
		if (!(ss >> runs) || !ss.eof() || runs == 0)
		{
			errorMsg = "Invalid monte-carlo value: '" + mcStr + "' (must be a positive integer)";
			return false;
		}
	}
	
	return true;
}