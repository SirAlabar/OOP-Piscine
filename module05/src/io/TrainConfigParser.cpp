#include "io/TrainConfigParser.hpp"
#include "utils/StringUtils.hpp"
#include "utils/Time.hpp"
#include <sstream>
#include <stdexcept>
#include <set>

TrainConfigParser::TrainConfigParser(const std::string& filepath)
    : FileParser(filepath)
{
}

std::vector<TrainConfig> TrainConfigParser::parse()
{
    std::vector<TrainConfig> configs;
    std::vector<std::string> lines = readLines();

    for (const auto& line : lines)
    {
        _lineNumber++;
        try
        {
            TrainConfig config = parseLine(line);
            configs.push_back(config);
        }
        catch (const std::exception& e)
        {
            throwLineError(e.what(), line);
        }
    }

    validateUniqueNames(configs);

    return configs;
}

void TrainConfigParser::validateUniqueNames(const std::vector<TrainConfig>& configs) const
{
    std::set<std::string> seenNames;

    for (const auto& config : configs)
    {
        if (!seenNames.insert(config.name).second)
        {
            throw std::runtime_error(
                "Duplicate train name detected: '" + config.name + "'");
        }
    }
}

TrainConfig TrainConfigParser::parseLine(const std::string& line)
{
    auto tokens = StringUtils::splitTokens(line);

    if (tokens.size() != 9)
    {
        throw std::runtime_error(
            "Invalid train format. Expected 9 fields: "
            "<n> <mass> <friction> <accel> <brake> <departure> <arrival> <time> <duration>"
        );
    }

    TrainConfig config;

    config.name = tokens[0];

    try
    {
        config.mass          = std::stod(tokens[1]);
        config.frictionCoef  = std::stod(tokens[2]);
        config.maxAccelForce = std::stod(tokens[3]);
        config.maxBrakeForce = std::stod(tokens[4]);
    }
    catch (...)
    {
        throw std::runtime_error("Invalid numeric value in train configuration");
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

    config.departureStation = tokens[5];
    config.arrivalStation   = tokens[6];

    if (config.departureStation.empty() || config.arrivalStation.empty())
    {
        throw std::runtime_error("Departure and arrival stations cannot be empty");
    }

    if (config.departureStation == config.arrivalStation)
    {
        throw std::runtime_error("Departure and arrival stations must be different");
    }

    config.departureTime = Time(tokens[7]);
    if (!config.departureTime.isValid())
    {
        throw std::runtime_error(
            "Invalid departure time format '" + tokens[7] +
            "'. Expected HHhMM (e.g., 14h10)");
    }

    config.stopDuration = Time(tokens[8]);
    if (!config.stopDuration.isValid())
    {
        throw std::runtime_error(
            "Invalid stop duration format '" + tokens[8] +
            "'. Expected HHhMM (e.g., 00h10)");
    }

    return config;
}