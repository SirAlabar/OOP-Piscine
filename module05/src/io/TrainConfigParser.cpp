#include "io/TrainConfigParser.hpp"
#include "utils/Time.hpp"
#include <sstream>
#include <stdexcept>

TrainConfigParser::TrainConfigParser(const std::string& filepath)
	: FileParser(filepath)
{
}

std::vector<TrainConfig> TrainConfigParser::parse()
{
	std::vector<TrainConfig> configs;
	std::vector<std::string> lines = readLines();
	int lineNumber = 0;

	for (const auto& line : lines)
	{
		lineNumber++;
		try
		{
			TrainConfig config = parseLine(line);
			configs.push_back(config);
		}
		catch (const std::exception& e)
		{
			throw std::runtime_error(
				"Error at line " + std::to_string(lineNumber) + 
				": " + e.what() + "\nContent: " + line
			);
		}
	}

	return configs;
}

TrainConfig TrainConfigParser::parseLine(const std::string& line)
{
	std::istringstream iss(line);
	TrainConfig config;

	std::string departureTimeStr;
	std::string stopDurationStr;

	iss >> config.name 
	    >> config.mass 
	    >> config.frictionCoef 
	    >> config.maxAccelForce 
	    >> config.maxBrakeForce
	    >> config.departureStation 
	    >> config.arrivalStation 
	    >> departureTimeStr 
	    >> stopDurationStr;

	if (iss.fail())
	{
		throw std::runtime_error(
			"Invalid train format. Expected 9 fields: "
			"<name> <mass> <friction> <accel> <brake> <departure> <arrival> <time> <duration>"
		);
	}

	if (config.name.empty())
	{
		throw std::runtime_error("Train name cannot be empty");
	}

	if (config.mass <= 0.0)
	{
		throw std::runtime_error("Train mass must be positive");
	}

	if (config.frictionCoef < 0.0)
	{
		throw std::runtime_error("Friction coefficient must be non-negative");
	}

	if (config.maxAccelForce <= 0.0)
	{
		throw std::runtime_error("Maximum acceleration force must be positive");
	}

	if (config.maxBrakeForce <= 0.0)
	{
		throw std::runtime_error("Maximum brake force must be positive");
	}

	if (config.departureStation.empty() || config.arrivalStation.empty())
	{
		throw std::runtime_error("Departure and arrival stations cannot be empty");
	}

	config.departureTime = Time(departureTimeStr);
	if (!config.departureTime.isValid())
	{
		throw std::runtime_error(
			"Invalid departure time format '" + departureTimeStr + 
			"'. Expected HHhMM (e.g., 14h10)"
		);
	}

	config.stopDuration = Time(stopDurationStr);
	if (!config.stopDuration.isValid())
	{
		throw std::runtime_error(
			"Invalid stop duration format '" + stopDurationStr + 
			"'. Expected HHhMM (e.g., 00h10)"
		);
	}

	return config;
}