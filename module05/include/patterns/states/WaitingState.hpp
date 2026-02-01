#ifndef WAITINGSTATE_HPP
#define WAITINGSTATE_HPP

#include "patterns/states/ITrainState.hpp"

class WaitingState : public ITrainState
{
public:
	WaitingState() = default;
	~WaitingState() override = default;
	
	void update(Train* train, double dt) override;
	std::string getName() const override;
	ITrainState* checkTransition(Train* train, SimulationContext* ctx) override;
};

#endif