#include "simulation/systems/PhysicsSystem.hpp"
#include "simulation/physics/PhysicsConstants.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include <cmath>

// -------------------------------------------------------------------------
// Unit conversions
// -------------------------------------------------------------------------

double PhysicsSystem::tonsToKg(double tons)
{
    return tons * PhysicsConstants::TONS_TO_KG;
}

double PhysicsSystem::kNtoN(double kN)
{
    return kN * PhysicsConstants::KN_TO_N;
}

double PhysicsSystem::kmhToMs(double kmh)
{
    return kmh / PhysicsConstants::KMH_TO_MS_DIVISOR;
}

double PhysicsSystem::kmToM(double km)
{
    return km * PhysicsConstants::KM_TO_M;
}

double PhysicsSystem::msToKmh(double ms)
{
    return ms * PhysicsConstants::MS_TO_KMH_FACTOR;
}

double PhysicsSystem::mToKm(double meters)
{
    return meters * PhysicsConstants::M_TO_KM;
}

// -------------------------------------------------------------------------
// Force calculations
// -------------------------------------------------------------------------

double PhysicsSystem::calculateFriction(const Train* train)
{
    if (!train)
    {
        return 0.0;
    }

    double massKg        = tonsToKg(train->getMass());
    double frictionForce = train->getFrictionCoef() * massKg * PhysicsConstants::GRAVITY;

    return frictionForce;  // Newtons
}

double PhysicsSystem::calculateNetForce(const Train* train, double appliedForce)
{
    if (!train)
    {
        return 0.0;
    }

    double frictionForce = calculateFriction(train);
    double netForce      = appliedForce - frictionForce;

    return netForce;  // Newtons
}

double PhysicsSystem::calculateBrakingDeceleration(const Train* train)
{
    if (!train)
    {
        return 0.0;
    }

    double massKg        = tonsToKg(train->getMass());
    double brakeForceN   = kNtoN(train->getMaxBrakeForce());
    double frictionForce = calculateFriction(train);

    // Both brake force and friction oppose motion.
    double totalForce   = brakeForceN + frictionForce;
    double deceleration = totalForce / massKg;

    return deceleration;  // m/s² (positive value)
}

double PhysicsSystem::calculateBrakingDistance(const Train* train)
{
    if (!train)
    {
        return 0.0;
    }

    double velocity = train->getVelocity();  // m/s

    if (velocity <= 0.0)
    {
        return 0.0;
    }

    double deceleration = calculateBrakingDeceleration(train);

    if (deceleration <= 0.0)
    {
        return 0.0;
    }

    // d = v² / (2 × a)
    double distance = (velocity * velocity) / (2.0 * deceleration);

    return distance;  // metres
}

// -------------------------------------------------------------------------
// Motion updates
// -------------------------------------------------------------------------

void PhysicsSystem::updateVelocity(Train* train, double netForce, double dt)
{
    if (!train || dt <= 0)
    {
        return;
    }

    double massKg = tonsToKg(train->getMass());

    // a = F / m
    double acceleration = netForce / massKg;

    // v_new = v_old + a × dt
    double newVelocity = train->getVelocity() + acceleration * dt;

    // Cannot reverse direction.
    if (newVelocity < 0.0)
    {
        newVelocity = 0.0;
    }

    train->setVelocity(newVelocity);
}

void PhysicsSystem::updatePosition(Train* train, double dt)
{
    if (!train || dt <= 0)
    {
        return;
    }

    // position_new = position_old + v × dt
    double newPosition = train->getPosition() + train->getVelocity() * dt;

    train->setPosition(newPosition);
}