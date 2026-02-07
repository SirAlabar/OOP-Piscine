#ifndef SEEDEDRNG_HPP
#define SEEDEDRNG_HPP

#include <random>

// Seeded Random Number Generator for reproducible simulations
// Same seed = identical event sequence
class SeededRNG
{
private:
	std::mt19937 _generator;
	unsigned int _seed;

public:
	explicit SeededRNG(unsigned int seed = 42);

	// Get random integer in range [min, max]
	int getInt(int min, int max);

	// Get random double in range [min, max]
	double getDouble(double min, double max);

	// Get random boolean with given probability (0.0 - 1.0)
	bool getBool(double probability);

	// Get the seed used
	unsigned int getSeed() const;

	// Reset generator with new seed
	void reseed(unsigned int seed);
};

#endif