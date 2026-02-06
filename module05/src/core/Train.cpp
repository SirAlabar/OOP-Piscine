#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "utils/Time.hpp"
#include "patterns/states/ITrainState.hpp" 
#include <iostream>

// Initialize static ID counter
int Train::_nextID = 1;

// Default constructor
Train::Train()
	: _name(""), _id(_nextID++), _finished(false), _mass(0.0), _frictionCoef(0.0),
	  _maxAccelForce(0.0), _maxBrakeForce(0.0),
	  _velocity(0.0), _position(0.0),
	  _departureStation(""), _arrivalStation(""),
	  _departureTime(), _stopDuration(),
	  _currentRailIndex(0), _currentState(nullptr)
{
}

// Parameterized constructor
Train::Train(const std::string& name, double mass, double frictionCoef,
             double maxAccelForce, double maxBrakeForce,
             const std::string& departureStation, const std::string& arrivalStation,
             const Time& departureTime, const Time& stopDuration)
	: _name(name), _id(_nextID++), _finished(false),
	  _mass(mass), _frictionCoef(frictionCoef),
	  _maxAccelForce(maxAccelForce), _maxBrakeForce(maxBrakeForce),
	  _velocity(0.0), _position(0.0),
	  _departureStation(departureStation),
	  _arrivalStation(arrivalStation),
	  _departureTime(departureTime),
	  _stopDuration(stopDuration),
	  _currentRailIndex(0), _currentState(nullptr)
{
}

// Copy constructor
Train::Train(const Train& other)
	: _name(other._name), _id(other._id),
	  _finished(other._finished),
	  _mass(other._mass), _frictionCoef(other._frictionCoef),
	  _maxAccelForce(other._maxAccelForce),
	  _maxBrakeForce(other._maxBrakeForce),
	  _velocity(other._velocity),
	  _position(other._position),
	  _departureStation(other._departureStation),
	  _arrivalStation(other._arrivalStation),
	  _departureTime(other._departureTime),
	  _stopDuration(other._stopDuration),
	  _path(other._path),
	  _currentRailIndex(other._currentRailIndex),
	  _currentState(other._currentState)
{
}

// Assignment operator
Train& Train::operator=(const Train& other)
{
	if (this != &other)
	{
		_name = other._name;
		_id = other._id;
		_finished = other._finished;
		_mass = other._mass;
		_frictionCoef = other._frictionCoef;
		_maxAccelForce = other._maxAccelForce;
		_maxBrakeForce = other._maxBrakeForce;
		_velocity = other._velocity;
		_position = other._position;
		_departureStation = other._departureStation;
		_arrivalStation = other._arrivalStation;
		_departureTime = other._departureTime;
		_stopDuration = other._stopDuration;
		_path = other._path;
		_currentRailIndex = other._currentRailIndex;
		_currentState = other._currentState;
	}
	return *this;
}

// Identity getters
std::string Train::getName() const
{
	return _name;
}

int Train::getID() const
{
	return _id;
}

// Physical property getters
double Train::getMass() const
{
	return _mass;
}

double Train::getFrictionCoef() const
{
	return _frictionCoef;
}

double Train::getMaxAccelForce() const
{
	return _maxAccelForce;
}

double Train::getMaxBrakeForce() const
{
	return _maxBrakeForce;
}

// Motion state getters/setters
double Train::getVelocity() const
{
	return _velocity;
}

double Train::getPosition() const
{
	return _position;
}

void Train::setVelocity(double velocity)
{
	_velocity = velocity;
}

void Train::setPosition(double position)
{
	_position = position;
}

// Journey getters
std::string Train::getDepartureStation() const
{
	return _departureStation;
}

std::string Train::getArrivalStation() const
{
	return _arrivalStation;
}

Time Train::getDepartureTime() const
{
	return _departureTime;
}

Time Train::getStopDuration() const
{
	return _stopDuration;
}

// Path management
const Train::Path& Train::getPath() const
{
	return _path;
}

void Train::setPath(const Path& path)
{
	_path = path;
	_currentRailIndex = 0;
}

Rail* Train::getCurrentRail() const
{
	if (_currentRailIndex < _path.size())
    {
		return _path[_currentRailIndex].rail;
    }
	return nullptr;
}

size_t Train::getCurrentRailIndex() const
{
	return _currentRailIndex;
}

void Train::advanceToNextRail()
{
	if (_path.empty())
	{
        return;
	}

	if (_currentRailIndex < _path.size())
	{
		_currentRailIndex++;
	}
}

// State management
ITrainState* Train::getCurrentState() const
{
	return _currentState;
}

void Train::setState(ITrainState* state)
{
	if (_finished)
	{
        return;
	}

	    std::cout << "[STATE] Train " << _name
              << " changing state from "
              << (_currentState ? _currentState->getName() : "NULL")
              << " to "
              << (state ? state->getName() : "NULL")
              << " | finished=" << (_finished ? "YES" : "NO")
              << std::endl;

	_currentState = state;
}

// Validation
bool Train::isValid() const
{
	return !_name.empty()
	    && _mass > 0.0
	    && _frictionCoef >= 0.0
	    && _maxAccelForce > 0.0
	    && _maxBrakeForce > 0.0
	    && !_departureStation.empty()
	    && !_arrivalStation.empty()
	    && _departureTime.isValid()
	    && _stopDuration.isValid();
}

// Static ID management
void Train::resetIDCounter()
{
	_nextID = 1;
}

int Train::getNextID()
{
	return _nextID;
}

bool Train::isFinished() const
{
    return _finished;
}

void Train::markFinished()
{
    if (_finished)
    {
        std::cout << "[FINISHED] Train " << _name 
                  << " already finished, markFinished() called again"
                  << std::endl;
        return;
    }

    std::cout << "[FINISHED] Train " << _name 
              << " is now FINISHED"
              << std::endl;

    _finished = true;
}


// Delegate update to current state
void Train::update(double dt)
{
	if (_currentState)
	{
		_currentState->update(this, dt);
	}
}