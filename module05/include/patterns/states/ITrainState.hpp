
#ifndef ITRAINSTATE_HPP
#define ITRAINSTATE_HPP

#include <string>

class Train;
class SimulationContext;

class ITrainState
{
public:
	virtual ~ITrainState() = default;
	
	// Existing: Update train physics/behavior
	virtual void update(Train* train, double dt) = 0;
	
	// Returns next state (or nullptr if no transition)
	virtual ITrainState* checkTransition(Train* train, SimulationContext* ctx) = 0;
	
	virtual std::string getName() const = 0;
};

#endif