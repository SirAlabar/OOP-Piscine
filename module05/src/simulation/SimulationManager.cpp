#include "simulation/SimulationManager.hpp"
#include "simulation/MovementSystem.hpp"
#include "simulation/SimulationContext.hpp"
#include "simulation/CollisionAvoidance.hpp"
#include "simulation/PhysicsSystem.hpp"
#include "core/Train.hpp"
#include "core/Graph.hpp"
#include "core/Rail.hpp"
#include "patterns/states/AcceleratingState.hpp"
#include "io/OutputWriter.hpp"
#include <iostream>

// Constructor
SimulationManager::SimulationManager()
	: _network(nullptr),
	  _collisionSystem(new CollisionAvoidance()),
	  _context(nullptr),
	  _currentTime(0.0),
	  _timestep(1.0),
	  _running(false),
	  _lastSnapshotMinute(-1)
{
}

// Destructor
SimulationManager::~SimulationManager()
{
	cleanupOutputWriters();
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
	_context = new SimulationContext(_network, _collisionSystem, &_trains);
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
	_lastSnapshotMinute = -1;

	if (!_network || !_context)
	{
		return;
	}

	// Create output writers and calculate estimated times
	for (Train* train : _trains)
	{
		if (!train)
		{
			continue;
		}

		try
		{
			OutputWriter* writer = new OutputWriter(train);
			writer->open();

			// Calculate estimated travel time
			double estimatedMinutes = 0.0;
			const auto& path = train->getPath();
			
			for (Rail* rail : path)
			{
				if (rail)
				{
					double segmentTimeHours = rail->getLength() / rail->getSpeedLimit();
					estimatedMinutes += segmentTimeHours * 60.0;
				}
			}

			writer->writeHeader(estimatedMinutes);
			_outputWriters[train] = writer;

			std::cout << "  Created output: " << train->getName() << "_" 
			          << train->getDepartureTime().toString() << ".result" 
			          << " (estimated: " << static_cast<int>(estimatedMinutes) << " min)" 
			          << std::endl;
		}
		catch (const std::exception& e)
		{
			std::cerr << "Failed to create output for train " << train->getName() 
			          << ": " << e.what() << std::endl;
		}
	}

	checkDepartures();
	_collisionSystem->refreshRailOccupancy(_trains, _network);
	_context->refreshAllRiskData();
	handleStateTransitions();
	_context->refreshAllRiskData();

	// Write initial snapshots for all trains at departure
	writeSnapshots();
}

void SimulationManager::stop()
{
	_running = false;

	// Write final snapshots for all trains
	for (auto& pair : _outputWriters)
	{
		Train* train = pair.first;
		OutputWriter* writer = pair.second;

		if (train && writer)
		{
			writer->writeSnapshot(_currentTime);
		}
	}

	cleanupOutputWriters();
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
	_context->refreshAllRiskData();
	updateTrainStates(_timestep);
	checkFinishedTrains();

	_currentTime += _timestep;

	// Write snapshots every 2 minutes
	int currentMinute = static_cast<int>(_currentTime / 60.0);
	if (currentMinute % 1 == 0 && currentMinute != _lastSnapshotMinute)
	{
		writeSnapshots();
		_lastSnapshotMinute = currentMinute;
	}
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
			if (train && !train->isFinished())
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
	cleanupOutputWriters();
	_trains.clear();
	_currentTime = 0.0;
	_running = false;
	_lastSnapshotMinute = -1;

	if (_context)
	{
		delete _context;
		_context = nullptr;
	}

	if (_network)
	{
		_context = new SimulationContext(_network, _collisionSystem, &_trains);
	}
}

// Helper methods
void SimulationManager::updateTrainStates(double dt)
{
    for (Train* train : _trains)
    {
        if (!train)
		{
            continue;
		}

        if (train->isFinished())
		{
            continue;
		}

        train->update(dt);
        MovementSystem::resolveProgress(train, _context);
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

		if (train->isFinished())
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

void SimulationManager::checkFinishedTrains()
{
    for (Train* train : _trains)
    {
        if (!train || train->isFinished())
		{
            continue;
		}

        if (train->getCurrentRail() == nullptr)
        {
            std::cout << "[MOVEMENT] "
                      << train->getName()
                      << " COMPLETED JOURNEY\n";

            train->markFinished();
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

		if (train->isFinished())
        {
            continue;
        }
		
		ITrainState* newState = train->getCurrentState()->checkTransition(train, _context);
		
		if (newState)
		{
			train->setState(newState);
		}
	}
}

void SimulationManager::writeSnapshots()
{
	for (auto& pair : _outputWriters)
	{
		Train* train = pair.first;
		OutputWriter* writer = pair.second;

		if (train && writer)
		{
			
			// Only write if train has departed
			if (train->getCurrentState()->getName() != "Idle")
			{
				writer->writeSnapshot(_currentTime);
			}
		}
	}
}

void SimulationManager::cleanupOutputWriters()
{
	for (auto& pair : _outputWriters)
	{
		if (pair.second)
		{
			pair.second->close();
			delete pair.second;
		}
	}
	_outputWriters.clear();
}