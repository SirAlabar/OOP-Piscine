#ifndef EMERGENCYSTATE_HPP
#define EMERGENCYSTATE_HPP

#include "patterns/states/ITrainState.hpp"

class EmergencyState : public ITrainState
{
public:
	EmergencyState() = default;
	~EmergencyState() override = default;
	
	void update(Train* train, double dt) override;
	ITrainState* checkTransition(Train* train, const SimulationContext* ctx) override;
	std::string getName() const override;
};

#endif