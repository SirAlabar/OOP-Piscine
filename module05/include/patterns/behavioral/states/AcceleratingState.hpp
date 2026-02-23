#ifndef ACCELERATINGSTATE_HPP
#define ACCELERATINGSTATE_HPP

#include "patterns/behavioral/states/ITrainState.hpp"

class AcceleratingState : public ITrainState
{
public:
	AcceleratingState() = default;
	~AcceleratingState() override = default;
	
	void update(Train* train, double dt) override;
	std::string getName() const override;
	ITrainState* checkTransition(Train* train, SimulationContext* ctx) override;
};

#endif