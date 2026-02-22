#include "simulation/SimulationManager.hpp"
#include "simulation/SimulationConfig.hpp"
#include "simulation/SimulationContext.hpp"
#include "simulation/CollisionAvoidance.hpp"
#include "patterns/mediator/TrafficController.hpp"
#include "core/Train.hpp"
#include "core/Graph.hpp"
#include "core/Rail.hpp"
#include "core/Node.hpp"
#include "patterns/states/IdleState.hpp"
#include "io/FileOutputWriter.hpp"
#include "io/ISimulationOutput.hpp"
#include "patterns/observers/EventDispatcher.hpp"
#include "patterns/observers/EventScheduler.hpp"
#include "patterns/observers/TrainEventAdapter.hpp"
#include "patterns/observers/RailEventAdapter.hpp"
#include "patterns/factories/EventFactory.hpp"
#include "patterns/events/Event.hpp"
#include "analysis/StatsCollector.hpp"
#include "patterns/commands/CommandManager.hpp"
#include "patterns/commands/ICommand.hpp"
#include "rendering/IRenderer.hpp"
#include <chrono>
#include <algorithm>


SimulationManager::SimulationManager()
    : _eventScheduler(_eventDispatcher),
      _network(nullptr),
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
      _commandManager(nullptr),
      // Services: each holds references to the data members above.
      _lifecycle(
          _trains, _context, _trafficController,
          _currentTime, _timestep, _roundTripEnabled,
          _eventScheduler),
      _eventPipeline(
          _eventFactory, _eventScheduler, _trains, _context,
          _simulationWriter, _outputWriters, _statsCollector,
          _currentTime, _lastEventGenerationTime),
      _reporting(
          _simulationWriter, _outputWriters, _trains, _context,
          _eventScheduler, _currentTime, _previousStates,
          _lastSnapshotMinute, _lastDashboardMinute)
{
    // Services that need to record commands take `this` via the ICommandRecorder
    // interface.  Set after full construction to avoid calling virtuals during init.
    _lifecycle.setCommandRecorder(this);
    _eventPipeline.setCommandRecorder(this);
}

SimulationManager::~SimulationManager()
{
    cleanupOutputWriters();
    delete _collisionSystem;
    delete _trafficController;
    delete _context;
    delete _eventFactory;
    for (IObserver* adapter : _eventAdapters)
    {
        delete adapter;
    }
}

void SimulationManager::record(ICommand* cmd)
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

// Configuration setters
void SimulationManager::setNetwork(Graph* network)
{
    _network = network;

    delete _context;
    _context = nullptr;

    delete _trafficController;
    _trafficController = nullptr;

    delete _eventFactory;
    _eventFactory = nullptr;

    _trafficController =
        new TrafficController(_network, _collisionSystem, &_trains);

    _context =
        new SimulationContext(_network, _collisionSystem,
                              &_trains, _trafficController);

    _eventFactory =
        new EventFactory(_eventSeed, _network, &_eventScheduler);
}

void SimulationManager::addTrain(Train* train)
{
    if (!train)
    {
        return;
    }

    if (_context)
    {
        train->setState(_context->states().idle());
    }

    _trains.push_back(train);
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

    if (_network && _eventFactory)
    {
        delete _eventFactory;
        _eventFactory = new EventFactory(_eventSeed, _network, &_eventScheduler);
    }
}

void SimulationManager::setSimulationWriter(ISimulationOutput* writer)
{
    _simulationWriter = writer;
}

void SimulationManager::setRoundTripMode(bool enabled)
{
    _roundTripEnabled = enabled;
}

void SimulationManager::registerOutputWriter(Train* train,
                                              FileOutputWriter* writer)
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

void SimulationManager::configure(const SimulationConfig& config)
{
    setNetwork(config.network);
    setEventSeed(config.seed);
    setRoundTripMode(config.roundTrip);
    setSimulationWriter(config.writer);
}

void SimulationManager::setCommandManager(CommandManager* mgr)
{
    _commandManager = mgr;
}

// Simulation control
void SimulationManager::start()
{
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
    if (!_network || !_context)
    {
        return;
    }

    tick(false, true);
}

void SimulationManager::tick(bool replayMode, bool advanceTime)
{
    _lifecycle.checkDepartures();
    refreshSimulationState();

    if (replayMode && _commandManager)
    {
        applyReplayCommands();
    }
    else
    {
        _lifecycle.handleStateTransitions();
    }

    _context->refreshAllRiskData();
    _lifecycle.updateTrainStates(_timestep);
    _eventPipeline.update();

    if (advanceTime)
    {
        _currentTime += _timestep;
    }

    _reporting.writeSnapshots();
    _reporting.updateDashboard();
}

void SimulationManager::run(double maxTime,
                            bool renderMode,
                            bool replayMode,
                            IRenderer* renderer,
                            const std::function<void()>& loopHook)
{
    using clock = std::chrono::steady_clock;

    start();

    if (renderMode && renderer)
    {
        renderer->initialize(*this);
    }

    auto   previous    = clock::now();
    double accumulator = 0.0;

    while (_running && _currentTime < maxTime)
    {
        if (loopHook)
        {
            loopHook();
        }

        if (renderMode)
        {
            if (!renderer || !renderer->processFrame(*this))
            {
                stop();
                break;
            }

            auto now = clock::now();
            std::chrono::duration<double> elapsed = now - previous;
            previous = now;

            accumulator +=
                elapsed.count() * _simulationSpeed * SimConfig::SECONDS_PER_MINUTE;

            while (accumulator >= _timestep && _running)
            {
                simulationTick(replayMode);
                accumulator -= _timestep;
            }
        }
        else
        {
            simulationTick(replayMode);
        }

        if (shouldStopEarly(replayMode))
        {
            stop();
        }
    }

    if (renderer)
    {
        renderer->shutdown();
    }
}


void SimulationManager::simulationTick(bool replayMode)
{
    tick(replayMode, true);
}

void SimulationManager::applyReplayCommands()
{
    std::vector<ICommand*> commands =
        _commandManager->getCommandsForTime(
            _currentTime,
            _currentTime + _timestep);

    for (ICommand* command : commands)
    {
        if (command)
        {
            command->applyReplay(this);
        }
    }
}

bool SimulationManager::shouldStopEarly(bool replayMode)
{
    if (_roundTripEnabled || replayMode)
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

void SimulationManager::refreshSimulationState()
{
    _collisionSystem->refreshRailOccupancy(_trains, _network);
    _context->refreshAllRiskData();
}

void SimulationManager::registerObservers()
{
    for (Train* train : _trains)
    {
        if (train)
        {
            IObserver* adapter = new TrainEventAdapter(train);
            _eventAdapters.push_back(adapter);
            _eventDispatcher.attach(adapter);
        }
    }

    if (!_network)
    {
        return;
    }

    for (Node* node : _network->getNodes())
    {
        if (node)
        {
            _eventDispatcher.attach(node);
        }
    }

    for (Rail* rail : _network->getRails())
    {
        if (rail)
        {
            IObserver* adapter = new RailEventAdapter(rail);
            _eventAdapters.push_back(adapter);
            _eventDispatcher.attach(adapter);
        }
    }
}

void SimulationManager::cleanupOutputWriters()
{
    _outputWriters.clear();
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
    return _trains;
}

const std::vector<Event*>& SimulationManager::getActiveEvents() const
{
    return _eventScheduler.getActiveEvents();
}

int SimulationManager::getTotalEventsGenerated() const
{
    return _eventScheduler.getTotalEventsGenerated();
}

const Graph* SimulationManager::getNetwork() const
{
    return _network;
}

bool SimulationManager::isRunning() const
{
    return _running;
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
    _simulationSpeed =
        std::max(SimConfig::MIN_SPEED, std::min(SimConfig::MAX_SPEED, speed));
}

Train* SimulationManager::findTrain(const std::string& name) const
{
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
    cleanupOutputWriters();
    _trains.clear();
    _previousStates.clear();
    _currentTime              = 0.0;
    _running                  = false;
    _lastSnapshotMinute       = -1;
    _lastDashboardMinute      = -1;
    _lastEventGenerationTime  = -60.0;
    _statsCollector           = nullptr;
    _commandManager           = nullptr;

    _eventScheduler.clear();
    _eventDispatcher.clearObservers();

    for (IObserver* adapter : _eventAdapters)
    {
        delete adapter;
    }
    _eventAdapters.clear();

    delete _context;
    _context = nullptr;

    delete _trafficController;
    _trafficController = nullptr;

    delete _eventFactory;
    _eventFactory = nullptr;

    _network = nullptr;
}