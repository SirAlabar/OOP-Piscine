#include "utils/SeededRNG.hpp"

SeededRNG::SeededRNG(unsigned int seed)
	: _generator(seed), _seed(seed)
{
}

int SeededRNG::getInt(int min, int max)
{
	std::uniform_int_distribution<int> dist(min, max);
	return dist(_generator);
}

double SeededRNG::getDouble(double min, double max)
{
	std::uniform_real_distribution<double> dist(min, max);
	return dist(_generator);
}

bool SeededRNG::getBool(double probability)
{
	std::uniform_real_distribution<double> dist(0.0, 1.0);
	return dist(_generator) < probability;
}

unsigned int SeededRNG::getSeed() const
{
	return _seed;
}

void SeededRNG::reseed(unsigned int seed)
{
	_seed = seed;
	_generator.seed(seed);
}