#ifndef IDLESTATE_HPP
#define IDLESTATE_HPP

#include "patterns/states/ITrainState.hpp"

class IdleState : public ITrainState
{
public:
	IdleState() = default;
	~IdleState() override = default;
	
	void update(Train* train, double dt) override;
	std::string getName() const override;
	ITrainState* checkTransition(Train* train, SimulationContext* ctx) override;
};

#endif