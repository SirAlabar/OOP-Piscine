#ifndef PHYSICSCONSTANTS_HPP
#define PHYSICSCONSTANTS_HPP

// Central repository for all physics constants and unit-conversion factors.
// Every physics calculation must reference these values — never hardcode
// 9.8, 3.6, 1000.0, etc. directly in source files.
namespace PhysicsConstants
{
    // -------------------------------------------------------------------------
    // Physical constants
    // -------------------------------------------------------------------------
    constexpr double GRAVITY = 9.8;  // m/s² — standard gravitational acceleration

    // -------------------------------------------------------------------------
    // Unit conversion factors
    // -------------------------------------------------------------------------

    // Mass: metric tons → kilograms
    constexpr double TONS_TO_KG = 1000.0;

    // Force: kilonewtons → newtons
    constexpr double KN_TO_N = 1000.0;

    // Distance: kilometres → metres
    constexpr double KM_TO_M = 1000.0;

    // Distance: metres → kilometres
    constexpr double M_TO_KM = 1.0 / KM_TO_M;

    // Speed: km/h → m/s  (divide by this factor)
    constexpr double KMH_TO_MS_DIVISOR = 3.6;

    // Speed: m/s → km/h  (multiply by this factor)
    constexpr double MS_TO_KMH_FACTOR = 3.6;
}

#endif