#ifndef SIMULATIONMANAGER_HPP
#define SIMULATIONMANAGER_HPP

#include <vector>
#include <map>
#include <functional>
#include "utils/Time.hpp"
#include "simulation/SimConstants.hpp"
#include "simulation/SimulationConfig.hpp"
#include "patterns/observers/EventDispatcher.hpp"
#include "patterns/observers/EventScheduler.hpp"
#include "patterns/commands/ICommandRecorder.hpp"
#include "simulation/TrainLifecycleService.hpp"
#include "simulation/EventPipeline.hpp"
#include "simulation/SimulationReporting.hpp"

class Train;
class Graph;
class CollisionAvoidance;
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
class IObserver;

// Thin facade / composition root.
// Owns all simulation data and wires the three sub-services:
//   TrainLifecycleService — departures, state transitions, physics
//   EventPipeline         — event scheduling, activation, expiration
//   SimulationReporting   — snapshot files, console dashboard
//
class SimulationManager : public ICommandRecorder
{
private:
    // Event subsystem — _dispatcher before _scheduler (constructor order matters).
    EventDispatcher _eventDispatcher;
    EventScheduler  _eventScheduler;

    Graph*              _network;
    TrainList           _trains;
    CollisionAvoidance* _collisionSystem;
    TrafficController*  _trafficController;
    SimulationContext*  _context;
    EventFactory*       _eventFactory;
    StatsCollector*     _statsCollector;

    double       _currentTime;
    double       _timestep;
    double       _simulationSpeed;
    bool         _running;
    bool         _roundTripEnabled;
    unsigned int _eventSeed;
    double       _lastEventGenerationTime;

    ISimulationOutput*                   _simulationWriter;
    std::map<Train*, FileOutputWriter*>  _outputWriters;
    std::map<Train*, ITrainState*>       _previousStates;
    int _lastSnapshotMinute;
    int _lastDashboardMinute;

    CommandManager* _commandManager;  // Not owned; injected by Application.

    std::vector<IObserver*> _eventAdapters;  // Owned; created in registerObservers().

    // Sub-services (initialized last; hold references to the members above).
    TrainLifecycleService _lifecycle;
    EventPipeline         _eventPipeline;
    SimulationReporting   _reporting;

    // Thin coordination methods that remain in SimulationManager.
    void tick(bool replayMode, bool advanceTime);
    void cleanupOutputWriters();
    void registerObservers();
    void refreshSimulationState();
    void simulationTick(bool replayMode);
    void applyReplayCommands();
    bool shouldStopEarly(bool replayMode);
	
public:
    using TrainList = std::vector<Train*>;

public:
    SimulationManager();
    ~SimulationManager();

    SimulationManager(const SimulationManager&)            = delete;
    SimulationManager& operator=(const SimulationManager&) = delete;
    SimulationManager(SimulationManager&&)                  = delete;
    SimulationManager& operator=(SimulationManager&&)       = delete;

    // ICommandRecorder — takes ownership of cmd.
    void record(ICommand* cmd) override;

    // Configuration setters
    void setNetwork(Graph* network);
    void addTrain(Train* train);
    void setTimestep(double timestep);
    void setEventSeed(unsigned int seed);
    void setRoundTripMode(bool enabled);
    void setSimulationWriter(ISimulationOutput* writer);
    void registerOutputWriter(Train* train, FileOutputWriter* writer);
    void setStatsCollector(StatsCollector* stats);

    // Configure network, seed, round-trip mode, and writer in one call.
    void configure(const SimulationConfig& config);

    // Inject CommandManager before calling run(). Pass nullptr to disable.
    void setCommandManager(CommandManager* mgr);

    Train* findTrain(const std::string& name) const;

    void start();
    void stop();
    void step();

    // renderMode = true  → real-time loop with renderer integrated in the same loop
    // replayMode = true  → skip autonomous state transitions; apply recorded commands
    void run(double maxTime,
             bool renderMode = false,
             bool replayMode = false,
             class IRenderer* renderer = nullptr,
             const std::function<void()>& loopHook = std::function<void()>());

    double             getCurrentTime()          const;
    Time               getCurrentTimeFormatted() const;
    const TrainList&   getTrains()               const;
    const std::vector<Event*>& getActiveEvents() const;
    int                getTotalEventsGenerated() const;
    const Graph*       getNetwork()              const;
    bool               isRunning()               const;
    unsigned int       getSeed()                 const;

    double getSimulationSpeed() const;
    void   setSimulationSpeed(double speed);

    SimulationContext* getContext() const { return _context; }

    void reset();
};

#endif