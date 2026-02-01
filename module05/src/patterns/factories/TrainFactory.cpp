#include "patterns/factories/TrainFactory.hpp"
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
	if (!network)
	{
		return false;
	}

	if (config.name.empty())
	{
		return false;
	}

	if (config.mass <= 0.0)
	{
		return false;
	}

	if (config.frictionCoef < 0.0)
	{
		return false;
	}

	if (config.maxAccelForce <= 0.0)
	{
		return false;
	}

	if (config.maxBrakeForce <= 0.0)
	{
		return false;
	}

	if (!network->hasNode(config.departureStation))
	{
		return false;
	}

	if (!network->hasNode(config.arrivalStation))
	{
		return false;
	}

	if (!config.departureTime.isValid())
	{
		return false;
	}

	if (!config.stopDuration.isValid())
	{
		return false;
	}

	return true;
}