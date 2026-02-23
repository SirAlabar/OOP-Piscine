#include "patterns/behavioral/states/StateRegistry.hpp"
#include "patterns/behavioral/states/IdleState.hpp"
#include "patterns/behavioral/states/AcceleratingState.hpp"
#include "patterns/behavioral/states/CruisingState.hpp"
#include "patterns/behavioral/states/WaitingState.hpp"
#include "patterns/behavioral/states/BrakingState.hpp"
#include "patterns/behavioral/states/StoppedState.hpp"
#include "patterns/behavioral/states/EmergencyState.hpp"

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

ITrainState* StateRegistry::fromName(const std::string& name)
{
    if (name == _idle.getName())
    {
        return &_idle;
    }

    if (name == _accelerating.getName())
    {
        return &_accelerating;
    }

    if (name == _cruising.getName())
    {
        return &_cruising;
    }

    if (name == _waiting.getName())
    {
        return &_waiting;
    }

    if (name == _braking.getName())
    {
        return &_braking;
    }

    if (name == _stopped.getName())
    {
        return &_stopped;
    }

    if (name == _emergency.getName())
    {
        return &_emergency;
    }

    return nullptr;
}