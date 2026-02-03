#ifndef PHYSICSSYSTEM_HPP
#define PHYSICSSYSTEM_HPP

class Train;

class PhysicsSystem
{
public:
	PhysicsSystem() = default;
	~PhysicsSystem() = default;
	
	// Constants
	static constexpr double GRAVITY = 9.8;  // m/s²
	
	// Unit conversions
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