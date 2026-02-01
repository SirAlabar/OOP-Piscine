#ifndef TRAINFACTORY_HPP
#define TRAINFACTORY_HPP

#include <string>
#include "utils/Time.hpp"

class Train;
class Graph;

struct TrainConfig
{
	std::string name;
	double      mass;
	double      frictionCoef;
	double      maxAccelForce;
	double      maxBrakeForce;
	std::string departureStation;
	std::string arrivalStation;
	Time        departureTime;
	Time        stopDuration;
};

class TrainFactory
{
public:
	static Train* create(const TrainConfig& config, const Graph* network);

private:
	static bool validate(const TrainConfig& config, const Graph* network);
};

#endif