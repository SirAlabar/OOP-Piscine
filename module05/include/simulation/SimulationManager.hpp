#ifndef SIMULATIONMANAGER_HPP
#define SIMULATIONMANAGER_HPP

#include <vector>
#include <map>
#include <functional>
#include "utils/Time.hpp"
#include "utils/SeededRNG.hpp"
#include "simulation/SimConstants.hpp"
#include "simulation/SimulationConfig.hpp"
#include "simulation/NetworkServicesFactory.hpp"
#include "patterns/observers/ObserverManager.hpp"
#include "patterns/observers/EventDispatcher.hpp"
#include "patterns/observers/EventScheduler.hpp"
#include "patterns/commands/ICommandRecorder.hpp"
#include "simulation/TrainLifecycleService.hpp"
#include "simulation/EventPipeline.hpp"
#include "simulation/SimulationReporting.hpp"

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
// Owns simulation data and wires three sub-services:
//   TrainLifecycleService — departures, state transitions, physics
//   EventPipeline         — event scheduling, activation, expiration
//   SimulationReporting   — snapshot files, console dashboard
class SimulationManager : public ICommandRecorder
{
private:
    EventDispatcher       _eventDispatcher;
    EventScheduler        _eventScheduler;

    SeededRNG              _rng;
    NetworkServicesFactory _networkServicesFactory;
    ObserverManager         _observerManager;

    Graph*                _network;
    TrainList             _trains;
    ICollisionAvoidance*  _collisionSystem;
    TrafficController*    _trafficController;
    SimulationContext*    _context;
    EventFactory*         _eventFactory;
    StatsCollector*       _statsCollector;

    double       _currentTime;
    double       _timestep;
    double       _simulationSpeed;
    bool         _running;
    bool         _roundTripEnabled;
    double       _lastEventGenerationTime;

    ISimulationOutput*                  _simulationWriter;
    std::map<Train*, FileOutputWriter*> _outputWriters;
    std::map<Train*, ITrainState*>      _previousStates;
    int _lastSnapshotMinute;
    int _lastDashboardMinute;

    CommandManager* _commandManager;

    TrainLifecycleService _lifecycle;
    EventPipeline         _eventPipeline;
    SimulationReporting   _reporting;

    void destroyNetworkServices();
    void tick(bool replayMode, bool advanceTime);
    void cleanupOutputWriters();
    void refreshSimulationState();
    void simulationTick(bool replayMode);
    void applyReplayCommands();
    bool shouldStopEarly(bool replayMode);

public:
    SimulationManager();
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

    Train* findTrain(const std::string& name) const;

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
    SimulationContext*         getContext()              const;

    void setSimulationSpeed(double speed);
    void reset();
};

#endif