#include "patterns/states/StateRegistry.hpp"
#include "patterns/states/IdleState.hpp"
#include "patterns/states/AcceleratingState.hpp"
#include "patterns/states/CruisingState.hpp"
#include "patterns/states/WaitingState.hpp"
#include "patterns/states/BrakingState.hpp"
#include "patterns/states/StoppedState.hpp"
#include "patterns/states/EmergencyState.hpp"

StateRegistry::StateRegistry()
	: _idle(),
	  _accelerating(),
	  _cruising(),
	  _waiting(),
	  _braking(),
	  _stopped(),
	  _emergency()
{
}

ITrainState* StateRegistry::idle()
{
	return &_idle;
}

ITrainState* StateRegistry::accelerating()
{
	return &_accelerating;
}

ITrainState* StateRegistry::cruising()
{
	return &_cruising;
}

ITrainState* StateRegistry::waiting()
{
	return &_waiting;
}

ITrainState* StateRegistry::braking()
{
	return &_braking;
}

ITrainState* StateRegistry::stopped()
{
	return &_stopped;
}

ITrainState* StateRegistry::emergency()
{
	return &_emergency;
}