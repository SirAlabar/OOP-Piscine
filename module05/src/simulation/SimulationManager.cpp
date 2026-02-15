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
#include "patterns/states/IdleState.hpp"
#include "io/FileOutputWriter.hpp"
#include "io/IOutputWriter.hpp"
#include "patterns/observers/EventManager.hpp"
#include "patterns/factories/EventFactory.hpp"
#include "patterns/events/Event.hpp"
#include "analysis/StatsCollector.hpp"
#include <thread>
#include <chrono>

// Constructor
SimulationManager::SimulationManager()
	: _network(nullptr),
	  _collisionSystem(new CollisionAvoidance()),
	  _trafficController(nullptr),
	  _context(nullptr),
	  _eventFactory(nullptr),
	  _statsCollector(nullptr),
	  _currentTime(0.0),  // Minutes
	  _timestep(SimConfig::BASE_TIMESTEP_MINUTES),  // 1.0 minute
	  _simulationSpeed(SimConfig::DEFAULT_SPEED),  // 10.0x
	  _running(false),
	  _roundTripEnabled(false),
	  _eventSeed(0),
	  _lastEventGenerationTime(-60.0),
	  _simulationWriter(nullptr),
	  _lastSnapshotMinute(-1),
	  _lastDashboardMinute(-1)
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

void SimulationManager::setSimulationWriter(IOutputWriter* writer)
{
	_simulationWriter = writer;
}

void SimulationManager::setRoundTripMode(bool enabled)
{
	_roundTripEnabled = enabled;
}

void SimulationManager::registerOutputWriter(Train* train, FileOutputWriter* writer)
{
	if (!train || !writer)
	{
		return;
	}
	
	_outputWriters[train] = writer;
}

void SimulationManager::setStatsCollector(StatsCollector* stats)
{
	_statsCollector = stats;
}

// Simulation control
void SimulationManager::start()
{
	_running = true;
	_lastSnapshotMinute = -1;
	_lastDashboardMinute = -1;

	if (!_network || !_context)
	{
		return;
	}
	
	// Register all entities as observers for events
	registerObservers();

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

	// Clear output writer references (Application owns and closes them)
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
	// Note: Finished trains are marked by MovementSystem when they reach destination
	
	// Update events (generate new events + update active events)
	updateEvents();

	_currentTime += _timestep;

	// Write snapshots (handles state changes + periodic writes internally)
	writeSnapshots();
	
	// Display periodic status dashboard every 5 simulation minutes
	int currentMinute = static_cast<int>(_currentTime);
	bool shouldShowDashboard = (currentMinute % 5 == 0 && currentMinute != _lastDashboardMinute && currentMinute > 0);
	
	if (shouldShowDashboard && _simulationWriter)
	{
		// Count active trains (not idle, not finished)
		int activeTrains = 0;
		int completedTrains = 0;
		
		for (Train* train : _trains)
		{
			if (!train) continue;
			
			if (train->isFinished())
			{
				completedTrains++;
			}
			else if (train->getCurrentState() && train->getCurrentState()->getName() != "Idle")
			{
				activeTrains++;
			}
		}
		
		// Only show dashboard if there are active trains
		if (activeTrains > 0 || completedTrains > 0)
		{
			_lastDashboardMinute = currentMinute;
			
			// Count active events
			int activeEvents = static_cast<int>(EventManager::getInstance().getActiveEvents().size());
			
			_simulationWriter->writeDashboard(getCurrentTimeFormatted(), activeTrains, 
			                                  static_cast<int>(_trains.size()), 
			                                  completedTrains, activeEvents);
		}
	}
}

void SimulationManager::run(double maxTime)
{
    start();

    using clock = std::chrono::steady_clock;

    auto previous = clock::now();
    double accumulator = 0.0;

    while (_running && _currentTime < maxTime)
    {
        auto now = clock::now();
        std::chrono::duration<double> elapsed = now - previous;
        previous = now;

        accumulator += elapsed.count() * _simulationSpeed;

        while (accumulator >= _timestep)
        {
            step();
            accumulator -= _timestep;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (!_roundTripEnabled)
        {
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
	// _currentTime is now in minutes
	int totalMinutes = static_cast<int>(_currentTime);
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
	_previousStates.clear();
	_currentTime = 0.0;
	_running = false;
	_lastSnapshotMinute = -1;
	_lastDashboardMinute = -1;
	_lastEventGenerationTime = -60.0;  // Reset so first check happens immediately
	_statsCollector = nullptr;  // Clear stats collector pointer
	
	EventManager::getInstance().clear();

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
	
	if (_eventFactory)
	{
		delete _eventFactory;
		_eventFactory = nullptr;
	}
	
	// Clear network pointer - new network will be set via setNetwork()
	_network = nullptr;
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

		// Skip finished trains UNLESS round-trip mode is enabled
        if (train->isFinished() && !_roundTripEnabled)
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
                
                // If train is finished (at destination) and round-trip is enabled, reverse journey
                if (train->isFinished() && _roundTripEnabled)
                {
                    // Reverse the journey
                    train->reverseJourney();
                    
                    // Calculate next departure time (same time, next day = +24 hours)
                    int currentMinutes = static_cast<int>(_currentTime);
                    Time currentTimeFormatted = getCurrentTimeFormatted();
                    Time trainDepartureTime = train->getDepartureTime();
                    
                    // Calculate minutes until next occurrence of departure time
                    int departureMinutes = trainDepartureTime.toMinutes();
                    int nextDeparture = departureMinutes + static_cast<int>(SimConfig::MINUTES_PER_DAY);
                    
                    // If we're past this time today, wait for tomorrow
                    while (nextDeparture <= currentMinutes)
                    {
                        nextDeparture += static_cast<int>(SimConfig::MINUTES_PER_DAY);
                    }
                    
					// Convert nextDeparture back to Time and update train
					int nextHours = nextDeparture / 60;
					int nextMins = nextDeparture % 60;
					train->setDepartureTime(Time(nextHours, nextMins));

					// Set train to Idle state - it will depart at the calculated time
                    static IdleState idleState;
                    train->setState(&idleState);
                }
            }
        }

        // Check for signal failures that force trains to stop
        MovementSystem::checkSignalFailures(train, _context);

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

				// Request access through TrafficController
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
	int currentMinute = static_cast<int>(_currentTime);
	bool periodicWrite = (currentMinute % 2 == 0 && currentMinute != _lastSnapshotMinute);
	
	if (periodicWrite)
	{
		_lastSnapshotMinute = currentMinute;
	}
	
	for (auto& pair : _outputWriters)
	{
		Train* train = pair.first;
		FileOutputWriter* writer = pair.second;

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
	// Writers are owned by Application, we only clear our references
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
	
	// Find newly activated events
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
			
			// Only notify on console/UI if writer available AND trains are active
			if (_simulationWriter)
			{
				// Check if any trains are active (not idle, not finished)
				bool hasActiveTrains = false;
				for (Train* train : _trains)
				{
					if (train && !train->isFinished() && 
					    train->getCurrentState() && train->getCurrentState()->getName() != "Idle")
					{
						hasActiveTrains = true;
						break;
					}
				}
				
				// Only display event if trains are running
				if (hasActiveTrains)
				{
					std::string eventTypeStr;
					switch (event->getType())
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
					
					_simulationWriter->writeEventActivated(getCurrentTimeFormatted(), 
					                                       eventTypeStr,
					                                       event->getDescription());
				}
			}
		}
	}
	
	// Try to generate new events
	double timeSinceLastGeneration = _currentTime - _lastEventGenerationTime;
	if (timeSinceLastGeneration >= 60.0)
	{
		std::vector<Event*> newEvents = _eventFactory->tryGenerateEvents(currentTimeFormatted, 60.0);
		
		// Schedule new events
		for (Event* event : newEvents)
		{
			if (event)
			{
				eventManager.scheduleEvent(event);
			}
		}
		
		_lastEventGenerationTime = _currentTime;
	}
}

void SimulationManager::logEventForAffectedTrains(Event* event, const std::string& action)
{
	if (!event)
	{
		return;
	}
	
	// Get event description
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
		
		// Each event type decides if it applies to this train at this moment
		bool trainAffected = event->isApplicableToTrain(train);
		
		// Log event if train is affected
		if (trainAffected)
		{
			// Count event for this specific train (Monte Carlo mode only)
			if (_statsCollector && action == "ACTIVATED")
			{
				_statsCollector->recordEventForTrain(train);
			}
			
			auto writerIt = _outputWriters.find(train);
			if (writerIt != _outputWriters.end() && writerIt->second)
			{
				writerIt->second->writeEventNotification(_currentTime, eventTypeStr, 
				                                         eventDescription, action);
			}
		}
	}
}

unsigned int SimulationManager::getSeed() const
{
	return _eventSeed;
}

double SimulationManager::getSimulationSpeed() const
{
	return _simulationSpeed;
}

void SimulationManager::setSimulationSpeed(double speed)
{
	// Clamp between min and max
	if (speed < SimConfig::MIN_SPEED)
	{
		_simulationSpeed = SimConfig::MIN_SPEED;
	}
	else if (speed > SimConfig::MAX_SPEED)
	{
		_simulationSpeed = SimConfig::MAX_SPEED;
	}
	else
	{
		_simulationSpeed = speed;
	}
}