#include "simulation/SimulationManager.hpp"
#include "simulation/SimulationConfig.hpp"
#include "simulation/SimulationContext.hpp"
#include "simulation/CollisionAvoidance.hpp"
#include "simulation/NetworkServicesFactory.hpp"
#include "patterns/mediator/TrafficController.hpp"
#include "patterns/factories/EventFactory.hpp"
#include "core/Train.hpp"
#include "core/Graph.hpp"
#include "core/Rail.hpp"
#include "core/Node.hpp"
#include "patterns/states/IdleState.hpp"
#include "io/FileOutputWriter.hpp"
#include "io/ISimulationOutput.hpp"
#include "patterns/observers/EventDispatcher.hpp"
#include "patterns/observers/EventScheduler.hpp"
#include "patterns/events/Event.hpp"
#include "analysis/StatsCollector.hpp"
#include "patterns/commands/CommandManager.hpp"
#include "patterns/commands/ICommand.hpp"
#include "rendering/IRenderer.hpp"
#include <chrono>
#include <algorithm>

SimulationManager::SimulationManager()
    : _eventScheduler(_eventDispatcher),
      _rng(0),
      _networkServicesFactory(nullptr, &_trains),
      _observerManager(_eventDispatcher),
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
      _lastEventGenerationTime(-60.0),
      _simulationWriter(nullptr),
      _lastSnapshotMinute(-1),
      _lastDashboardMinute(-1),
      _commandManager(nullptr),
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
    _networkServicesFactory = NetworkServicesFactory(_collisionSystem, &_trains);

    _lifecycle.setCommandRecorder(this);
    _eventPipeline.setCommandRecorder(this);
}

SimulationManager::~SimulationManager()
{
    cleanupOutputWriters();
    destroyNetworkServices();
    delete _collisionSystem;
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

void SimulationManager::destroyNetworkServices()
{
    delete _trafficController;
    _trafficController = nullptr;

    delete _context;
    _context = nullptr;

    delete _eventFactory;
    _eventFactory = nullptr;
}

void SimulationManager::setNetwork(Graph* network)
{
    _network = network;

    destroyNetworkServices();
    _rng.reseed(_rng.getSeed());

    NetworkServices svc = _networkServicesFactory.build(_network, _rng, &_eventScheduler);
    _trafficController  = svc.trafficController;
    _context            = svc.context;
    _eventFactory       = svc.eventFactory;
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
    _rng.reseed(seed);

    if (_network)
    {
        delete _eventFactory;
        _eventFactory = _networkServicesFactory.buildEventFactory(_network, _rng, &_eventScheduler);
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

void SimulationManager::start()
{
    _running             = true;
    _lastSnapshotMinute  = -1;
    _lastDashboardMinute = -1;

    if (!_network || !_context)
    {
        return;
    }

    _observerManager.wire(_trains, _network);
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

            accumulator += elapsed.count() * _simulationSpeed * SimConfig::SECONDS_PER_MINUTE;

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
        _commandManager->getCommandsForTime(_currentTime, _currentTime + _timestep);

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

void SimulationManager::cleanupOutputWriters()
{
    _outputWriters.clear();
}

double SimulationManager::getCurrentTime() const
{
    return _currentTime;
}

Time SimulationManager::getCurrentTimeFormatted() const
{
    int totalMinutes = static_cast<int>(_currentTime / SimConfig::SECONDS_PER_MINUTE);
    return Time(totalMinutes / 60, totalMinutes % 60);
}

const TrainList& SimulationManager::getTrains() const
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
    return _rng.getSeed();
}

double SimulationManager::getSimulationSpeed() const
{
    return _simulationSpeed;
}

void SimulationManager::setSimulationSpeed(double speed)
{
    _simulationSpeed = std::max(SimConfig::MIN_SPEED, std::min(SimConfig::MAX_SPEED, speed));
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

SimulationContext* SimulationManager::getContext() const
{
    return _context;
}

void SimulationManager::reset()
{
    cleanupOutputWriters();
    _trains.clear();
    _previousStates.clear();
    _currentTime             = 0.0;
    _running                 = false;
    _lastSnapshotMinute      = -1;
    _lastDashboardMinute     = -1;
    _lastEventGenerationTime = -60.0;
    _statsCollector          = nullptr;
    _commandManager          = nullptr;

    _eventScheduler.clear();
    _observerManager.clear();

    destroyNetworkServices();

    _network = nullptr;
}