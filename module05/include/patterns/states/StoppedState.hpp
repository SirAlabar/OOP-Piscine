#ifndef STOPPEDSTATE_HPP
#define STOPPEDSTATE_HPP

#include "patterns/states/ITrainState.hpp"

class StoppedState : public ITrainState
{
private:
	double _timeRemaining;  // seconds left in stop

public:
	StoppedState(double stopDuration);
	~StoppedState() override = default;
	
	void update(Train* train, double dt) override;
	std::string getName() const override;
	double getTimeRemaining() const;
    ITrainState* checkTransition(Train* train, SimulationContext* ctx) override; 
};

#endif