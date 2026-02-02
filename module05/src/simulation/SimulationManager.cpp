#include "simulation/SimulationManager.hpp"
#include "simulation/SimulationContext.hpp"
#include "simulation/CollisionAvoidance.hpp"
#include "simulation/PhysicsSystem.hpp"
#include "core/Train.hpp"
#include "core/Graph.hpp"
#include "patterns/states/AcceleratingState.hpp"

// Constructor
SimulationManager::SimulationManager()
	: _network(nullptr),
	  _collisionSystem(new CollisionAvoidance()),
	  _context(nullptr),
	  _currentTime(0.0),
	  _timestep(1.0),
	  _running(false)
{
}

// Destructor
SimulationManager::~SimulationManager()
{
	delete _collisionSystem;
	delete _context;
}

// Initialization
void SimulationManager::setNetwork(Graph* network)
{
	_network = network;
	
	if (_context)
	{
		delete _context;
	}
	_context = new SimulationContext(
    _network,
    _collisionSystem,
    &_trains);
}

void SimulationManager::addTrain(Train* train)
{
	if (train)
	{
		_trains.push_back(train);
	}
}

void SimulationManager::setTimestep(double timestep)
{
	if (timestep > 0.0)
	{
		_timestep = timestep;
	}
}

// Simulation control
void SimulationManager::start()
{
	_running = true;
}

void SimulationManager::stop()
{
	_running = false;
}

void SimulationManager::step()
{
	if (!_network || !_context)
	{
		return;
	}

	checkDepartures();
	_collisionSystem->refreshRailOccupancy(_trains, _network);
	_context->refreshAllRiskData();
	handleStateTransitions();
	updateTrainStates(_timestep);
	_currentTime += _timestep;
}


void SimulationManager::run(double maxTime)
{
	start();
	
	while (_running && _currentTime < maxTime)
	{
		step();
		
		// Check if all trains finished
		bool allFinished = true;
		for (Train* train : _trains)
		{
			if (train && train->getCurrentRail() != nullptr)
			{
				allFinished = false;
				break;
			}
		}
		
		if (allFinished)
		{
			stop();
		}
	}
}

// Getters
double SimulationManager::getCurrentTime() const
{
	return _currentTime;
}

Time SimulationManager::getCurrentTimeFormatted() const
{
	int totalMinutes = static_cast<int>(_currentTime / 60.0);
	int hours = totalMinutes / 60;
	int minutes = totalMinutes % 60;
	return Time(hours, minutes);
}

const SimulationManager::TrainList& SimulationManager::getTrains() const
{
	return _trains;
}

const Graph* SimulationManager::getNetwork() const
{
	return _network;
}

bool SimulationManager::isRunning() const
{
	return _running;
}

// Cleanup
void SimulationManager::reset()
{
	_trains.clear();
	_currentTime = 0.0;
	_running = false;

	if (_context)
	{
		delete _context;
		_context = nullptr;
	}

	if (_network)
	{
		_context = new SimulationContext(
                    _network,
                    _collisionSystem,
                    &_trains);

	}
}


// Helper methods
void SimulationManager::updateTrainStates(double dt)
{
	for (Train* train : _trains)
	{
		if (train)
		{
			train->update(dt);
		}
	}
}

void SimulationManager::checkDepartures()
{
	Time currentTimeFormatted = getCurrentTimeFormatted();
	
	for (Train* train : _trains)
	{
		if (!train || !train->getCurrentState())
		{
			continue;
		}
		
		if (train->getCurrentState()->getName() == "Idle")
		{
			if (currentTimeFormatted >= train->getDepartureTime())
			{
				static AcceleratingState accelState;
				train->setState(&accelState);
			}
		}
	}
}

void SimulationManager::handleStateTransitions()
{
	for (Train* train : _trains)
	{
		if (!train || !train->getCurrentState())
		{
			continue;
		}
		
		// Ask state if it wants to transition
		ITrainState* newState = train->getCurrentState()->checkTransition(train, _context);
		
		if (newState)
		{
			train->setState(newState);
		}
	}
}