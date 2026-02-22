#ifndef IRNG_HPP
#define IRNG_HPP

// Narrow interface for random number generation.
class IRng
{
public:
    virtual ~IRng() = default;

    // Returns a random integer in the closed range [min, max].
    virtual int getInt(int min, int max) = 0;

    // Returns a random double in the closed range [min, max].
    virtual double getDouble(double min, double max) = 0;

    // Returns true with the given probability in [0.0, 1.0].
    virtual bool getBool(double probability) = 0;

    // Returns the seed used to initialise this generator.
    virtual unsigned int getSeed() const = 0;
};

#endif