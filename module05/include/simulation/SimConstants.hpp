#ifndef SIMCONSTANTS_HPP
#define SIMCONSTANTS_HPP

// Shared simulation timing and speed constants.
// Split out of SimulationManager.hpp so that sub-services can include
// these values without creating a circular dependency.
namespace SimConfig
{
    constexpr double BASE_TIMESTEP_SECONDS  = 1.0;
    constexpr double SECONDS_PER_MINUTE     = 60.0;
    constexpr double SECONDS_PER_HOUR       = 3600.0;
    constexpr double SECONDS_PER_DAY        = 86400.0;

    constexpr int MINUTES_PER_DAY       = 1440;
    constexpr int MINUTES_PER_HALF_DAY  = 720;

    constexpr double MIN_SPEED     = 0.1;
    constexpr double MAX_SPEED     = 100.0;
    constexpr double DEFAULT_SPEED = 10.0;
}

#endif