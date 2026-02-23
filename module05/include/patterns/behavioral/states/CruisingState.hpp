#ifndef CRUISINGSTATE_HPP
#define CRUISINGSTATE_HPP

#include "patterns/behavioral/states/ITrainState.hpp"

class CruisingState : public ITrainState
{
public:
	CruisingState() = default;
	~CruisingState() override = default;
	
	void update(Train* train, double dt) override;
	std::string getName() const override;
	ITrainState* checkTransition(Train* train, SimulationContext* ctx) override;
};

#endif