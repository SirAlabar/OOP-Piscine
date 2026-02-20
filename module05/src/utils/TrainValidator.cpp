#include "patterns/factories/TrainValidator.hpp"
#include "patterns/factories/TrainFactory.hpp"
#include "core/Graph.hpp"

ValidationResult TrainValidator::validate(const TrainConfig& config, const Graph* network)
{
    ValidationResult result;

    if (!network)
    {
        result.error = "Network is null";
        return result;
    }

    if (config.name.empty())
    {
        result.error = "Train name cannot be empty";
        return result;
    }

    if (config.mass <= 0.0)
    {
        result.error = "Train '" + config.name + "': mass must be positive";
        return result;
    }

    if (config.frictionCoef < 0.0)
    {
        result.error = "Train '" + config.name + "': friction coefficient must be non-negative";
        return result;
    }

    if (config.maxAccelForce <= 0.0)
    {
        result.error = "Train '" + config.name + "': max acceleration force must be positive";
        return result;
    }

    if (config.maxBrakeForce <= 0.0)
    {
        result.error = "Train '" + config.name + "': max brake force must be positive";
        return result;
    }

    if (!network->hasNode(config.departureStation))
    {
        result.error = "Train '" + config.name + "': departure station '" +
                       config.departureStation + "' not found in network";
        return result;
    }

    if (!network->hasNode(config.arrivalStation))
    {
        result.error = "Train '" + config.name + "': arrival station '" +
                       config.arrivalStation + "' not found in network";
        return result;
    }

    if (!config.departureTime.isValid())
    {
        result.error = "Train '" + config.name + "': invalid departure time";
        return result;
    }

    if (!config.stopDuration.isValid())
    {
        result.error = "Train '" + config.name + "': invalid stop duration";
        return result;
    }

    result.valid = true;
    return result;
}