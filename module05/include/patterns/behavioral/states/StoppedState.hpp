#ifndef STOPPEDSTATE_HPP
#define STOPPEDSTATE_HPP

#include "patterns/behavioral/states/ITrainState.hpp"

class StoppedState : public ITrainState
{
public:
	StoppedState() = default;
	~StoppedState() override = default;
	
	void update(Train* train, double dt) override;
	std::string getName() const override;
	ITrainState* checkTransition(Train* train, SimulationContext* ctx) override;
};

#endif