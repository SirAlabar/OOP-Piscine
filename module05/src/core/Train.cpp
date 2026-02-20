#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "utils/Time.hpp"
#include "patterns/states/ITrainState.hpp"
#include <iostream>
#include <algorithm>

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

void Train::setDepartureTime(const Time& time)
{
	_departureTime = time;
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

const PathSegment* Train::getCurrentPathSegment() const
{
	if (_currentRailIndex < _path.size())
	{
		return &_path[_currentRailIndex];
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

void Train::reverseJourney()
{
	// Swap departure and arrival stations
	std::string temp = _departureStation;
	_departureStation = _arrivalStation;
	_arrivalStation = temp;
	
	// Reverse the path vector (Aâ†’Bâ†’C becomes Câ†’Bâ†’A)
	std::reverse(_path.begin(), _path.end());
	
	// For each segment, swap from/to nodes to maintain correct direction
	for (PathSegment& segment : _path)
	{
		Node* tempNode = segment.from;
		segment.from = segment.to;
		segment.to = tempNode;
	}
	
	// Reset journey state
	_currentRailIndex = 0;
	_position = 0.0;
	_velocity = 0.0;
	_finished = false;
	
	// Departure time stays same (will be checked against next day's time)
	// Stop duration remains unchanged
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
        return;
    }
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

// Get current node (start of current rail)
Node* Train::getCurrentNode() const
{
	if (_path.empty() || _currentRailIndex >= _path.size())
	{
		return nullptr;
	}

	return _path[_currentRailIndex].from;
}

// Get next node (end of current rail)
Node* Train::getNextNode() const
{
	if (_path.empty() || _currentRailIndex >= _path.size())
	{
		return nullptr;
	}

	return _path[_currentRailIndex].to;
}