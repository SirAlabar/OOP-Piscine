#include "io/CLI.hpp"
#include <iostream>

CLI::CLI(int argc, char* argv[]) : _argc(argc), _argv(argv)
{
}

bool CLI::shouldShowHelp() const
{
	return _argc == 2 && std::string(_argv[1]) == "--help";
}

bool CLI::hasValidArguments() const
{
	return _argc == 3;
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
	
	std::cout << "========================================\n\n";
}