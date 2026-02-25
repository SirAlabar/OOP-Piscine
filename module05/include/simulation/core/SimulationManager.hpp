#ifndef SIMULATIONMANAGER_HPP
#define SIMULATIONMANAGER_HPP

#include <vector>
#include <map>
#include <memory>
#include <functional>
#include "utils/Time.hpp"
#include "utils/SeededRNG.hpp"
#include "simulation/core/SimConstants.hpp"
#include "simulation/core/SimulationConfig.hpp"
#include "simulation/services/NetworkServicesFactory.hpp"
#include "event_system/ObserverManager.hpp"
#include "event_system/EventDispatcher.hpp"
#include "event_system/EventScheduler.hpp"
#include "patterns/behavioral/command/ICommandRecorder.hpp"
#include "simulation/interfaces/IReplayTarget.hpp"
#include "simulation/services/TrainLifecycleService.hpp"
#include "simulation/systems/EventPipeline.hpp"
#include "simulation/reporting/SimulationReporting.hpp"

class Train;
class Graph;
class ICollisionAvoidance;
class SimulationContext;
class FileOutputWriter;
class ISimulationOutput;
class TrafficController;
class EventFactory;
class Event;
class StatsCollector;
class CommandManager;
class ICommand;
class ITrainState;

using TrainList = std::vector<Train*>;

// Thin facade and composition root.
// Owns all simulation services via unique_ptr — no raw owning pointers.
// Wires three sub-services:
//   TrainLifecycleService — departures, state transitions, physics
//   EventPipeline         — event scheduling, activation, expiration
//   SimulationReporting   — snapshot files, console dashboard
class SimulationManager : public ICommandRecorder, public IReplayTarget
{
private:
    EventDispatcher        _eventDispatcher;
    EventScheduler         _eventScheduler;
    SeededRNG              _rng;
    NetworkServicesFactory _networkServicesFactory;
    ObserverManager        _observerManager;

    // Collision: owned when created internally, non-owning when injected.
    std::unique_ptr<ICollisionAvoidance> _ownedCollision;
    ICollisionAvoidance*                 _collisionSystem;

    // Network-bound services: owned here, referenced by sub-services.
    std::unique_ptr<TrafficController> _trafficController;
    std::unique_ptr<SimulationContext> _context;
    std::unique_ptr<EventFactory>      _eventFactory;

    Graph*    _network;
    TrainList _trains;

    double _currentTime;
    double _timestep;
    double _simulationSpeed;
    bool   _running;
    bool   _roundTripEnabled;
    double _lastEventGenerationTime;

    ISimulationOutput*                  _simulationWriter;
    StatsCollector*                     _statsCollector;
    CommandManager*                     _commandManager;

    std::map<Train*, FileOutputWriter*> _outputWriters;
    std::map<Train*, ITrainState*>      _previousStates;

    int _lastSnapshotMinute;
    int _lastDashboardMinute;

    TrainLifecycleService _lifecycle;
    EventPipeline         _eventPipeline;
    SimulationReporting   _reporting;

    void resetNetworkServices();
    void tick(bool replayMode, bool advanceTime);
    void cleanupOutputWriters();
    void refreshSimulationState();
    void simulationTick(bool replayMode);
    void applyReplayCommands();
    bool shouldStopEarly(bool replayMode);

public:
    SimulationManager();
    explicit SimulationManager(ICollisionAvoidance* collision);
    ~SimulationManager();

    SimulationManager(const SimulationManager&)            = delete;
    SimulationManager& operator=(const SimulationManager&) = delete;
    SimulationManager(SimulationManager&&)                  = delete;
    SimulationManager& operator=(SimulationManager&&)       = delete;

    void record(ICommand* cmd) override;

    void setNetwork(Graph* network);
    void addTrain(Train* train);
    void setTimestep(double timestep);
    void setEventSeed(unsigned int seed);
    void setRoundTripMode(bool enabled);
    void setSimulationWriter(ISimulationOutput* writer);
    void registerOutputWriter(Train* train, FileOutputWriter* writer);
    void setStatsCollector(StatsCollector* stats);
    void configure(const SimulationConfig& config);
    void setCommandManager(CommandManager* mgr);

    Train* findTrain(const std::string& name) const override;

    void start();
    void stop();
    void step();

    void run(double maxTime,
             bool renderMode = false,
             bool replayMode = false,
             class IRenderer* renderer = nullptr,
             const std::function<void()>& loopHook = std::function<void()>());

    double                     getCurrentTime()          const;
    Time                       getCurrentTimeFormatted() const;
    const TrainList&           getTrains()               const;
    const std::vector<Event*>& getActiveEvents()         const;
    int                        getTotalEventsGenerated() const;
    const Graph*               getNetwork()              const;
    bool                       isRunning()               const;
    unsigned int               getSeed()                 const;
    double                     getSimulationSpeed()      const;
    SimulationContext*         getContext()              const override;

    void setSimulationSpeed(double speed);
    void reset();
};

#endif