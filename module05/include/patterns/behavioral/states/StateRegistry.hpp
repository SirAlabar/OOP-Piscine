#ifndef STATEREGISTRY_HPP
#define STATEREGISTRY_HPP

#include <string>
#include "patterns/behavioral/states/IdleState.hpp"
#include "patterns/behavioral/states/AcceleratingState.hpp"
#include "patterns/behavioral/states/CruisingState.hpp"
#include "patterns/behavioral/states/WaitingState.hpp"
#include "patterns/behavioral/states/BrakingState.hpp"
#include "patterns/behavioral/states/StoppedState.hpp"
#include "patterns/behavioral/states/EmergencyState.hpp"

class StateRegistry
{
private:
	IdleState         _idle;
	AcceleratingState _accelerating;
	CruisingState     _cruising;
	WaitingState      _waiting;
	BrakingState      _braking;
	StoppedState      _stopped;
	EmergencyState    _emergency;

public:
	StateRegistry();
	~StateRegistry() = default;

	// Prevent copying (registry should be unique per simulation)
	StateRegistry(const StateRegistry&) = delete;
	StateRegistry& operator=(const StateRegistry&) = delete;

	// State accessors - return pointers to owned instances
	ITrainState* idle();
	ITrainState* accelerating();
	ITrainState* cruising();
	ITrainState* waiting();
	ITrainState* braking();
	ITrainState* stopped();
	ITrainState* emergency();

	// Returns the state whose getName() matches the given string, or nullptr.
	ITrainState* fromName(const std::string& name);
};

#endif