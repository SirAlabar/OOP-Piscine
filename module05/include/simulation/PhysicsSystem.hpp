#ifndef PHYSICSSYSTEM_HPP
#define PHYSICSSYSTEM_HPP

#include "simulation/PhysicsConstants.hpp"

class Train;

class PhysicsSystem
{
public:
    PhysicsSystem()  = default;
    ~PhysicsSystem() = default;

    // Unit conversions — delegate to PhysicsConstants for a single source of truth.
    static double tonsToKg(double tons);
    static double kNtoN(double kN);
    static double kmhToMs(double kmh);
    static double kmToM(double km);
    static double msToKmh(double ms);
    static double mToKm(double meters);

    // Force calculations
    static double calculateFriction(const Train* train);
    static double calculateNetForce(const Train* train, double appliedForce);
    static double calculateBrakingDeceleration(const Train* train);
    static double calculateBrakingDistance(const Train* train);

    // Motion updates
    static void updateVelocity(Train* train, double netForce, double dt);
    static void updatePosition(Train* train, double dt);
};

#endif