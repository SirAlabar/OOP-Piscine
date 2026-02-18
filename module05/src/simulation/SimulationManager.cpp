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
// Command Pattern
#include "patterns/commands/CommandManager.hpp"
#include "patterns/commands/ICommand.hpp"
#include "patterns/commands/TrainStateChangeCommand.hpp"
#include "patterns/commands/TrainDepartureCommand.hpp"
#include "patterns/commands/TrainAdvanceRailCommand.hpp"
#include "patterns/commands/SimEventCommand.hpp"
#include <thread>
#include <chrono>

SimulationManager::SimulationManager()
    : _network(nullptr),
      _collisionSystem(new CollisionAvoidance()),
      _trafficController(nullptr),
      _context(nullptr),
      _eventFactory(nullptr),
      _statsCollector(nullptr),
      _currentTime(0.0),
      _timestep(SimConfig::BASE_TIMESTEP_SECONDS),
      _simulationSpeed(SimConfig::DEFAULT_SPEED),
      _running(false),
      _roundTripEnabled(false),
      _eventSeed(0),
      _lastEventGenerationTime(-60.0),
      _simulationWriter(nullptr),
      _lastSnapshotMinute(-1),
      _lastDashboardMinute(-1),
      _commandManager(nullptr)
{
}

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
    std::unique_lock lock(_dataMutex);

    _network = network;

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

    _trafficController =
        new TrafficController(_network, _collisionSystem, &_trains);

    _context =
        new SimulationContext(_network, _collisionSystem,
                              &_trains, _trafficController);

    _eventFactory =
        new EventFactory(_eventSeed, _network,
                         &EventManager::getInstance());
}

void SimulationManager::addTrain(Train* train)
{
    if (!train) 
    {
        return;
    }

    std::unique_lock lock(_dataMutex);

    if (_context)
    {
        train->setState(_context->states().idle());
    }

    _trains.push_back(train);
}

bool SimulationManager::isTrainActive(const Train* train) const
{
    return train
        && train->getCurrentState()
        && !train->isFinished()
        && train->getCurrentState() != _context->states().idle();
}

bool SimulationManager::hasAnyActiveTrain() const
{
    for (Train* train : _trains)
    {
        if (isTrainActive(train))
        {
            return true;
        }
    }

    return false;
}

void SimulationManager::setTimestep(double timestep)
{
    if (timestep > 0.0) { _timestep = timestep; }
}

void SimulationManager::setEventSeed(unsigned int seed)
{
    _eventSeed = seed;

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

void SimulationManager::refreshSimulationState()
{
    // Collision system update
    _collisionSystem->refreshRailOccupancy(_trains, _network);
    // Context refresh
    _context->refreshAllRiskData();
}


void SimulationManager::recordCommand(ICommand* cmd)
{
    if (!cmd)
    {
        return;
    }

    if (_commandManager && _commandManager->isRecording())
    {
        _commandManager->record(cmd);
    }
    else
    {
        delete cmd;
    }
}

void SimulationManager::registerOutputWriter(Train* train, FileOutputWriter* writer)
{
    if (train && writer)
    {
        _outputWriters[train] = writer;
    }
}

void SimulationManager::setStatsCollector(StatsCollector* stats)
{
    _statsCollector = stats;
}

void SimulationManager::setCommandManager(CommandManager* mgr)
{
    _commandManager = mgr;
}

// Simulation control
void SimulationManager::start()
{
    std::unique_lock<std::shared_mutex> lock(_dataMutex);

    _running             = true;
    _lastSnapshotMinute  = -1;
    _lastDashboardMinute = -1;

    if (!_network || !_context)
    {
        return;
    }

    registerObservers();
    refreshSimulationState();
}

void SimulationManager::stop()
{
    _running = false;
    cleanupOutputWriters();
}

void SimulationManager::step()
{
    std::unique_lock lock(_dataMutex);

    if (!_network || !_context)
    {
        return;
    }

    tick(false, true);
}

void SimulationManager::tick(bool replayMode, bool advanceTime)
{
    // Departures
    checkDepartures();
	// Collision system update and context refresh
	refreshSimulationState();
    // Replay or normal transitions
    if (replayMode && _commandManager)
    {
        applyReplayCommands();
    }
    else
    {
        handleStateTransitions();
    }
    // Refresh after transitions
    _context->refreshAllRiskData();
    // Movement
    updateTrainStates(_timestep);
    // Events
    updateEvents();
    // Advance time only if requested
    if (advanceTime)
    {
        _currentTime += _timestep;
    }
    // Output
    writeSnapshots();
    updateDashboard();
}

// Simulation loop entry point
void SimulationManager::run(double maxTime, bool renderMode, bool replayMode)
{
    start();

    if (renderMode)
    {
        runRenderLoop(maxTime, replayMode);
    }
    else
    {
        runConsoleLoop(maxTime, replayMode);
    }
}

void SimulationManager::runConsoleLoop(double maxTime, bool replayMode)
{
    while (_running && _currentTime < maxTime)
    {
        simulationTick(replayMode);

        if (shouldStopEarly(replayMode))
        {
            stop();
        }
    }
}

// Render loop (real-time pacing with speed multiplier)
void SimulationManager::runRenderLoop(double maxTime, bool replayMode)
{
    using clock = std::chrono::steady_clock;

    auto previous = clock::now();

    double accumulator = 0.0;

    while (_running && _currentTime < maxTime)
    {
        auto now = clock::now();

        std::chrono::duration<double> elapsed = now - previous;

        previous = now;

        accumulator += elapsed.count()
                     * _simulationSpeed
                     * SimConfig::SECONDS_PER_MINUTE;

        while (accumulator >= _timestep)
        {
            simulationTick(replayMode);

            accumulator -= _timestep;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (shouldStopEarly(replayMode))
        {
            stop();
        }
    }
}

void SimulationManager::simulationTick(bool replayMode)
{
    std::unique_lock<std::shared_mutex> lock(_dataMutex);

    tick(replayMode, true);
}

// Apply recorded replay commands for current timestep
void SimulationManager::applyReplayCommands()
{
    std::vector<ICommand*> commands =
        _commandManager->getCommandsForTime(
            _currentTime,
            _currentTime + _timestep
        );

    for (ICommand* command : commands)
    {
        if (command)
        {
            command->applyReplay(this);
        }
    }
}


// Early exit condition (non-round-trip, non-replay only)
bool SimulationManager::shouldStopEarly(bool replayMode)
{
    if (_roundTripEnabled)
    {
        return false;
    }

    if (replayMode)
    {
        return false;
    }

    for (Train* train : _trains)
    {
        if (train && !train->isFinished())
        {
            return false;
        }
    }

    return true;
}

// Dashboard update logic (every 5 minutes)
void SimulationManager::updateDashboard()
{
    if (!_simulationWriter)
    {
        return;
    }

    int currentMinute =
        static_cast<int>(_currentTime / SimConfig::SECONDS_PER_MINUTE);

    bool shouldUpdate =
        (currentMinute % 5 == 0)
        && (currentMinute != _lastDashboardMinute)
        && (currentMinute > 0);

    if (!shouldUpdate)
    {
        return;
    }

    int activeTrains    = 0;
    int completedTrains = 0;

    for (Train* train : _trains)
    {
        if (!train)
        {
            continue;
        }

        if (train->isFinished())
        {
            completedTrains++;
        }
		else if (isTrainActive(train))
		{
			activeTrains++;
		}
    }

    if (activeTrains == 0 && completedTrains == 0)
    {
        return;
    }

    _lastDashboardMinute = currentMinute;

    int activeEvents =
        static_cast<int>(
            EventManager::getInstance()
                .getActiveEvents()
                .size()
        );

    _simulationWriter->writeDashboard(
        getCurrentTimeFormatted(),
        activeTrains,
        static_cast<int>(_trains.size()),
        completedTrains,
        activeEvents
    );
}


// Getters
double SimulationManager::getCurrentTime() const
{
	return _currentTime;
}

Time SimulationManager::getCurrentTimeFormatted() const
{
    int totalMinutes = static_cast<int>(_currentTime / SimConfig::SECONDS_PER_MINUTE);
    return Time(totalMinutes / 60, totalMinutes % 60);
}

const SimulationManager::TrainList& SimulationManager::getTrains() const
{ 
	std::shared_lock lock(_dataMutex);
	return _trains; 
}
const Graph*                        SimulationManager::getNetwork()  const
{ 
	std::shared_lock lock(_dataMutex);
	return _network; 
}

bool                                SimulationManager::isRunning()   const
{
	return _running;
}

unsigned int                        SimulationManager::getSeed()     const
{
	return _eventSeed;
}

double                              SimulationManager::getSimulationSpeed() const
{
	return _simulationSpeed;
}

void SimulationManager::setSimulationSpeed(double speed)
{
    _simulationSpeed = std::max(SimConfig::MIN_SPEED, std::min(SimConfig::MAX_SPEED, speed));
}

Train* SimulationManager::findTrain(const std::string& name) const
{
    std::shared_lock lock(_dataMutex);

    for (Train* train : _trains)
    {
        if (train && train->getName() == name)
        {
            return train;
        }
    }
    return nullptr;
}

void SimulationManager::reset()
{
    std::unique_lock lock(_dataMutex);

    cleanupOutputWriters();
    _trains.clear();
    _previousStates.clear();
    _currentTime              = 0.0;
    _running                  = false;
    _lastSnapshotMinute       = -1;
    _lastDashboardMinute      = -1;
    _lastEventGenerationTime  = -60.0;
    _statsCollector           = nullptr;
    _commandManager           = nullptr;  // Application re-injects after reset

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

    _network = nullptr;
}

// Private helpers
void SimulationManager::updateTrainStates(double dt)
{
    for (Train* train : _trains)
    {
        if (!train)
		{
			continue;
		}
        if (train->isFinished() && !_roundTripEnabled)
		{
			continue;
		}

        train->update(dt);

		if (train->getCurrentState() == _context->states().stopped())
        {
            bool expired = _context->decrementStopDuration(train, dt);
            if (expired)
            {
                _context->clearStopDuration(train);

                if (train->isFinished() && _roundTripEnabled)
                {
                    train->reverseJourney();

                    int  currentMinutes  = static_cast<int>(_currentTime / SimConfig::SECONDS_PER_MINUTE);
                    int  departureMinutes = train->getDepartureTime().toMinutes();
                    int  nextDeparture   = departureMinutes + SimConfig::MINUTES_PER_HALF_DAY;

                    while (nextDeparture <= currentMinutes)
                    {
                        nextDeparture += SimConfig::MINUTES_PER_HALF_DAY;
                    }

                    train->setDepartureTime(Time(nextDeparture / 60, nextDeparture % 60));
					train->setState(_context->states().idle());
                }
            }
        }

        MovementSystem::checkSignalFailures(train, _context);

        // Record rail advancement before resolveProgress changes the index
        std::size_t prevRailIndex = train->getCurrentRailIndex();
        MovementSystem::resolveProgress(train, _context);
        std::size_t newRailIndex  = train->getCurrentRailIndex();

		if (newRailIndex != prevRailIndex)
		{
			recordCommand(new TrainAdvanceRailCommand(_currentTime,
											train->getName(),
											newRailIndex));
		}
    }
}

void SimulationManager::checkDepartures()
{
    Time currentTimeFormatted = getCurrentTimeFormatted();

    for (Train* train : _trains)
    {
		if (!train || !train->getCurrentState() || train->isFinished())
		{
			continue;
		}
        if (train->getCurrentState() == _context->states().idle())
        {
            if (currentTimeFormatted >= train->getDepartureTime())
            {
                const auto& path = train->getPath();
                if (path.empty() || !path[0].rail)
				{
					continue;
				}

                if (_trafficController)
                {
                    TrafficController::AccessDecision decision =
                        _trafficController->requestRailAccess(train, path[0].rail);

                    if (decision == TrafficController::GRANT)
                    {
						train->setState(_context->states().accelerating());
                        // Record departure
						recordCommand(new TrainDepartureCommand(_currentTime, train->getName()));
                    }
                }
            }
        }
    }
}

void SimulationManager::handleStateTransitions()
{
    for (Train* train : _trains)
    {
		if (!isTrainActive(train))
		{
			continue;
		}

        std::string    prevStateName = train->getCurrentState()->getName();
        ITrainState*   newState      = train->getCurrentState()->checkTransition(train, _context);

        if (newState)
        {
            train->setState(newState);

            // Record state change
			recordCommand(new TrainStateChangeCommand(_currentTime,
                                train->getName(),
                                prevStateName,
                                newState->getName()));

        }
    }
}

void SimulationManager::writeSnapshots()
{
    int  currentMinute = static_cast<int>(_currentTime / SimConfig::SECONDS_PER_MINUTE);
    bool periodicWrite = (currentMinute % 2 == 0 && currentMinute != _lastSnapshotMinute);

    if (periodicWrite)
	{
		_lastSnapshotMinute = currentMinute;
	}

    for (auto& pair : _outputWriters)
    {
        Train*           train  = pair.first;
        FileOutputWriter* writer = pair.second;

		if (!writer || !isTrainActive(train))
		{
			continue;
		}

        ITrainState* currentState = train->getCurrentState();
        bool        stateChanged     = false;

        auto prevIt = _previousStates.find(train);
        if (prevIt != _previousStates.end())
        {
            stateChanged = (prevIt->second != currentState);
        }
        else
        {
            _previousStates[train] = currentState;
            stateChanged           = true;
        }

        if (stateChanged || periodicWrite)
        {
            writer->writeSnapshot(_currentTime);
            _previousStates[train] = currentState;
        }
    }
}

void SimulationManager::cleanupOutputWriters()
{
    _outputWriters.clear();
}

void SimulationManager::registerObservers()
{
    EventManager& eventManager = EventManager::getInstance();

    for (Train* train : _trains)
    {
        if (train)
		{
			eventManager.attach(train);
		}
    }

    if (_network)
    {
        for (Node* node : _network->getNodes())
        {
            if (node)
			{
				eventManager.attach(node);
			}
        }
        for (Rail* rail : _network->getRails())
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
    std::vector<Event*> previousActive = eventManager.getActiveEvents();
    Time currentTimeFormatted = getCurrentTimeFormatted();
    eventManager.update(currentTimeFormatted);
    std::vector<Event*> currentActive = eventManager.getActiveEvents();

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
        if (!isNew)
        {
            continue;
        }

        std::string eventTypeStr = Event::typeToString(event->getType());
        logEventForAffectedTrains(event, "ACTIVATED");

		recordCommand(new SimEventCommand(
				_currentTime,
				eventTypeStr,
				event->getDescription()
			)
		);

        if (_simulationWriter && hasAnyActiveTrain())
        {
            _simulationWriter->writeEventActivated(
                getCurrentTimeFormatted(),
                eventTypeStr,
                event->getDescription()
            );
        }
    }

    double timeSinceLastGeneration =
        _currentTime - _lastEventGenerationTime;

    if (timeSinceLastGeneration >= 60.0)
    {
        std::vector<Event*> newEvents =
            _eventFactory->tryGenerateEvents(
                currentTimeFormatted,
                60.0
            );

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

    std::string eventTypeStr = Event::typeToString(event->getType());

	for (Train* train : _trains)
	{
		if (!isTrainActive(train))
		{
			continue;
		}

		if (!event->isApplicableToTrain(train))
		{
			continue;
		}

		if (_statsCollector && action == "ACTIVATED")
		{
			_statsCollector->recordEventForTrain(train);
		}

		auto writerIt = _outputWriters.find(train);

		if (writerIt != _outputWriters.end() && writerIt->second)
		{
			writerIt->second->writeEventNotification(
				_currentTime,
				eventTypeStr,
				event->getDescription(),
				action
			);
		}
	}
}
