#ifndef SEEDEDRNG_HPP
#define SEEDEDRNG_HPP

#include "utils/IRng.hpp"
#include <random>

// Seeded Random Number Generator for reproducible simulations.
// Same seed = identical event sequence.
class SeededRNG : public IRng
{
private:
    std::mt19937 _generator;
    unsigned int _seed;

public:
    explicit SeededRNG(unsigned int seed = 42);

    int          getInt(int min, int max)          override;
    double       getDouble(double min, double max)  override;
    bool         getBool(double probability)        override;
    unsigned int getSeed()                    const override;

    // Reset generator to a new seed (used by MonteCarloRunner between runs).
    void reseed(unsigned int seed);
};

#endif