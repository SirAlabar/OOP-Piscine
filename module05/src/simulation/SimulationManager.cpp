#include "simulation/SimulationManager.hpp"
#include "simulation/MovementSystem.hpp"
#include "simulation/SimulationContext.hpp"
#include "simulation/CollisionAvoidance.hpp"
#include "simulation/PhysicsSystem.hpp"
#include "patterns/mediator/TrafficController.hpp"
#include "core/Train.hpp"
#include "core/Graph.hpp"
#include "core/Rail.hpp"
#include "core/Node.hpp"
#include "patterns/states/AcceleratingState.hpp"
#include "io/OutputWriter.hpp"
#include "patterns/observers/EventManager.hpp"
#include "patterns/factories/EventFactory.hpp"
#include "patterns/events/Event.hpp"
#include <iostream>

// Constructor
SimulationManager::SimulationManager()
	: _network(nullptr),
	  _collisionSystem(new CollisionAvoidance()),
	  _trafficController(nullptr),
	  _context(nullptr),
	  _eventFactory(nullptr),
	  _currentTime(0.0),
	  _timestep(1.0),
	  _running(false),
	  _eventSeed(42),
	  _lastSnapshotMinute(-1)
{
}

// Destructor
SimulationManager::~SimulationManager()
{
	cleanupOutputWriters();
	delete _collisionSystem;
	delete _trafficController;
	delete _context;
	delete _eventFactory;
	EventManager::destroy();
}

// Initialization
void SimulationManager::setNetwork(Graph* network)
{
	_network = network;
	
	if (_context)
	{
		delete _context;
	}
	
	if (_trafficController)
	{
		delete _trafficController;
	}
	
	if (_eventFactory)
	{
		delete _eventFactory;
	}
	
	// Create TrafficController (Mediator)
	_trafficController = new TrafficController(_network, _collisionSystem, &_trains);
	
	// Create SimulationContext with TrafficController
	_context = new SimulationContext(_network, _collisionSystem, &_trains, _trafficController);
	
	// Initialize EventFactory with seed
	_eventFactory = new EventFactory(_eventSeed, _network, &EventManager::getInstance());
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

void SimulationManager::setEventSeed(unsigned int seed)
{
	_eventSeed = seed;
	
	// Reinitialize EventFactory with new seed if network is already set
	if (_network && _eventFactory)
	{
		delete _eventFactory;
		_eventFactory = new EventFactory(_eventSeed, _network, &EventManager::getInstance());
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
	
	// Register all entities as observers for events
	registerObservers();

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
			
			for (const PathSegment& segment : path)
			{
				if (segment.rail)
				{
					double segmentTimeHours = segment.rail->getLength() / segment.rail->getSpeedLimit();
					estimatedMinutes += segmentTimeHours * 60.0;
				}
			}

			writer->writeHeader(estimatedMinutes);
			writer->writePathInfo();
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

	// Close all output files (final snapshots already written when trains finished)
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
	
	// Update events (generate new events + update active events)
	updateEvents();

	_currentTime += _timestep;

	// Write snapshots (handles state changes + periodic writes internally)
	writeSnapshots();
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
	_previousStates.clear();  // Clear state tracking
	_currentTime = 0.0;
	_running = false;
	_lastSnapshotMinute = -1;

	if (_context)
	{
		delete _context;
		_context = nullptr;
	}
	
	if (_trafficController)
	{
		delete _trafficController;
		_trafficController = nullptr;
	}

	if (_network)
	{
		_trafficController = new TrafficController(_network, _collisionSystem, &_trains);
		_context = new SimulationContext(_network, _collisionSystem, &_trains, _trafficController);
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

        // Decrement stop duration if train is stopped at a station
        if (train->getCurrentState() && 
            train->getCurrentState()->getName() == "Stopped")
        {
            bool expired = _context->decrementStopDuration(train, dt);
            if (expired)
            {
                _context->clearStopDuration(train);
            }
        }

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
				// Train wants to depart - check first rail in path
				const auto& path = train->getPath();
				if (path.empty() || !path[0].rail)
				{
					continue;
				}

				// Request access through TrafficController (Mediator pattern)
				if (_trafficController)
				{
					TrafficController::AccessDecision decision = 
						_trafficController->requestRailAccess(train, path[0].rail);
					
					if (decision == TrafficController::GRANT)
					{
						// Permission granted - depart!
						static AcceleratingState accelState;
						train->setState(&accelState);
					}
					// If DENY, stay in Idle and try again next step
				}
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
	// Write snapshots every 2 minutes
	int currentMinute = static_cast<int>(_currentTime / 60.0);
	bool periodicWrite = (currentMinute % 2 == 0 && currentMinute != _lastSnapshotMinute);
	
	if (periodicWrite)
	{
		_lastSnapshotMinute = currentMinute;
	}
	
	for (auto& pair : _outputWriters)
	{
		Train* train = pair.first;
		OutputWriter* writer = pair.second;

		if (train && writer)
		{
			// Skip trains that haven't departed yet
			if (train->getCurrentState()->getName() == "Idle")
			{
				continue;
			}
			
			// Get current state name
			std::string currentStateName = train->getCurrentState()->getName();
			
			// Check if state changed
			bool stateChanged = false;
			auto prevStateIt = _previousStates.find(train);
			if (prevStateIt != _previousStates.end())
			{
				stateChanged = (prevStateIt->second != currentStateName);
			}
			else
			{
				// First time tracking this train - initialize
				_previousStates[train] = currentStateName;
				stateChanged = true;  // Write first snapshot after departure
			}
			
			// Write snapshot if: state changed OR periodic interval
			if (stateChanged || periodicWrite)
			{
				writer->writeSnapshot(_currentTime);
				
				// Update previous state
				_previousStates[train] = currentStateName;
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

void SimulationManager::registerObservers()
{
	EventManager& eventManager = EventManager::getInstance();
	
	// Register all trains as observers
	for (Train* train : _trains)
	{
		if (train)
		{
			eventManager.attach(train);
		}
	}
	
	// Register all nodes as observers
	if (_network)
	{
		const auto& nodes = _network->getNodes();
		for (Node* node : nodes)
		{
			if (node)
			{
				eventManager.attach(node);
			}
		}
		
		// Register all rails as observers
		const auto& rails = _network->getRails();
		for (Rail* rail : rails)
		{
			if (rail)
			{
				eventManager.attach(rail);
			}
		}
	}
}

void SimulationManager::updateEvents()
{
	if (!_eventFactory)
	{
		return;
	}
	
	EventManager& eventManager = EventManager::getInstance();
	
	// Store active events before update
	std::vector<Event*> previousActive = eventManager.getActiveEvents();
	
	// Update existing events (activate/deactivate based on time)
	Time currentTimeFormatted = getCurrentTimeFormatted();
	eventManager.update(currentTimeFormatted);
	
	// Get current active events after update
	std::vector<Event*> currentActive = eventManager.getActiveEvents();
	
	// Find newly activated events (in current but not in previous)
	for (Event* event : currentActive)
	{
		bool isNew = true;
		for (Event* prev : previousActive)
		{
			if (prev == event)
			{
				isNew = false;
				break;
			}
		}
		
		if (isNew)
		{
			logEventForAffectedTrains(event, "ACTIVATED");
		}
	}
	
	// Try to generate new events (probability-based)
	std::vector<Event*> newEvents = _eventFactory->tryGenerateEvents(currentTimeFormatted);
	
	// Schedule new events
	for (Event* event : newEvents)
	{
		if (event)
		{
			eventManager.scheduleEvent(event);
		}
	}
}

void SimulationManager::logEventForAffectedTrains(Event* event, const std::string& action)
{
	if (!event)
	{
		return;
	}
	
	// Get event description polymorphically (SOLID: OCP + DIP)
	std::string eventDescription = event->getDescription();
	EventType type = event->getType();
	
	// Map event type to string
	std::string eventTypeStr;
	switch (type)
	{
		case EventType::STATION_DELAY:
			eventTypeStr = "STATION DELAY";
			break;

		case EventType::TRACK_MAINTENANCE:
			eventTypeStr = "TRACK MAINTENANCE";
			break;

		case EventType::SIGNAL_FAILURE:
			eventTypeStr = "SIGNAL FAILURE";
			break;

		case EventType::WEATHER:
			eventTypeStr = "WEATHER EVENT";
			break;
		default:
			eventTypeStr = "UNKNOWN EVENT";
			break;
	}
	
	// Check each train to see if event affects them
	for (Train* train : _trains)
	{
		if (!train)
		{
			continue;
		}
		
		// Skip trains that haven't departed
		if (!train->getCurrentState() || train->getCurrentState()->getName() == "Idle")
		{
			continue;
		}
		
		// Check if event affects this train (polymorphic check)
		bool trainAffected = false;
		
		// Check based on train's current/next location
		Node* currentNode = train->getCurrentNode();
		Node* nextNode = train->getNextNode();
		Rail* currentRail = train->getCurrentRail();
		
		if (event->affectsNode(currentNode) || event->affectsNode(nextNode) ||
		    (currentRail && event->affectsRail(currentRail)))
		{
			trainAffected = true;
		}
		
		// Log event if train is affected
		if (trainAffected)
		{
			auto writerIt = _outputWriters.find(train);
			if (writerIt != _outputWriters.end() && writerIt->second)
			{
				writerIt->second->writeEventNotification(_currentTime, eventTypeStr, 
				                                         eventDescription, action);
			}
		}
	}
}