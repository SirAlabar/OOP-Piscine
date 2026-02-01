#ifndef BRAKINGSTATE_HPP
#define BRAKINGSTATE_HPP

#include "patterns/states/ITrainState.hpp"

class BrakingState : public ITrainState
{
public:
	BrakingState() = default;
	~BrakingState() override = default;
	
	void update(Train* train, double dt) override;
	std::string getName() const override;
	ITrainState* checkTransition(Train* train, SimulationContext* ctx) override;
};

#endif