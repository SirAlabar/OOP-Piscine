#include "simulation/PhysicsSystem.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include <cmath>

// Unit conversions
double PhysicsSystem::tonsToKg(double tons)
{
	return tons * 1000.0;
}

double PhysicsSystem::kNtoN(double kN)
{
	return kN * 1000.0;
}

double PhysicsSystem::kmhToMs(double kmh)
{
	return kmh / 3.6;
}

double PhysicsSystem::kmToM(double km)
{
	return km * 1000.0;
}

double PhysicsSystem::msToKmh(double ms)
{
	return ms * 3.6;
}

double PhysicsSystem::mToKm(double meters)
{
	return meters / 1000.0;
}

// Force calculations
double PhysicsSystem::calculateFriction(const Train* train)
{
	if (!train)
	{
		return 0.0;
	}
	
	double massKg = tonsToKg(train->getMass());
	double frictionForce = train->getFrictionCoef() * massKg * GRAVITY;
	
	return frictionForce;  // Newtons
}

double PhysicsSystem::calculateNetForce(const Train* train, double appliedForce)
{
	if (!train)
	{
		return 0.0;
	}
	
	double frictionForce = calculateFriction(train);
	double netForce = appliedForce - frictionForce;
	
	return netForce;  // Newtons
}

double PhysicsSystem::calculateBrakingDeceleration(const Train* train)
{
	if (!train)
	{
		return 0.0;
	}
	
	double massKg = tonsToKg(train->getMass());
	double brakeForceN = kNtoN(train->getMaxBrakeForce());
	double frictionForce = calculateFriction(train);
	
	// Both brake and friction oppose motion
	double totalForce = brakeForceN + frictionForce;
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
	
	// d = v² / (2 * a)
	double distance = (velocity * velocity) / (2.0 * deceleration);
	
	return distance;  // meters
}

// Motion updates
void PhysicsSystem::updateVelocity(Train* train, double netForce, double dt)
{
	if (!train)
	{
		return;
	}
	
	double massKg = tonsToKg(train->getMass());
	
	// a = F / m
	double acceleration = netForce / massKg;
	
	// v_new = v_old + a * dt
	double newVelocity = train->getVelocity() + acceleration * dt;
	
	// Cannot go backward
	if (newVelocity < 0.0)
	{
		newVelocity = 0.0;
	}
	
	train->setVelocity(newVelocity);
}

void PhysicsSystem::updatePosition(Train* train, double dt)
{
	if (!train)
	{
		return;
	}
	
	// position_new = position_old + v * dt
	double newPosition = train->getPosition() + train->getVelocity() * dt;
	
	train->setPosition(newPosition);
}