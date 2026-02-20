#include "patterns/factories/TrainFactory.hpp"
#include "patterns/factories/TrainValidator.hpp"
#include "core/Train.hpp"
#include "core/Graph.hpp"
#include "utils/Time.hpp"

Train* TrainFactory::create(const TrainConfig& config, const Graph* network)
{
    if (!validate(config, network))
    {
        return nullptr;
    }

    return new Train(
        config.name,
        config.mass,
        config.frictionCoef,
        config.maxAccelForce,
        config.maxBrakeForce,
        config.departureStation,
        config.arrivalStation,
        config.departureTime,
        config.stopDuration
    );
}

bool TrainFactory::validate(const TrainConfig& config, const Graph* network)
{
    return TrainValidator::validate(config, network).valid;
}