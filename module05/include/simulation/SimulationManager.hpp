#ifndef SIMULATIONMANAGER_HPP
#define SIMULATIONMANAGER_HPP

#include <vector>
#include <map>
#include <functional>
#include "utils/Time.hpp"
#include "simulation/SimulationConfig.hpp"
#include "patterns/observers/EventDispatcher.hpp"
#include "patterns/observers/EventScheduler.hpp"

// Simulation time configuration
namespace SimConfig
{
    constexpr double BASE_TIMESTEP_SECONDS  = 1.0;
    constexpr double SECONDS_PER_MINUTE     = 60.0;
    constexpr double SECONDS_PER_HOUR       = 3600.0;
    constexpr double SECONDS_PER_DAY        = 86400.0;

    constexpr int MINUTES_PER_DAY       = 1440;
    constexpr int MINUTES_PER_HALF_DAY  = 720;

    constexpr double MIN_SPEED     = 0.1;
    constexpr double MAX_SPEED     = 100.0;
    constexpr double DEFAULT_SPEED = 10.0;
}

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

// Regular class — owned by Application as a value member.
// No Singleton; one instance per simulation run.
class SimulationManager
{
private:
    // Event subsystem — value members; EventScheduler holds a reference to
    // EventDispatcher so declaration order matters (_dispatcher before _scheduler).
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

    ISimulationOutput*              _simulationWriter;
    std::map<Train*, FileOutputWriter*> _outputWriters;
    std::map<Train*, ITrainState*>  _previousStates;
    int _lastSnapshotMinute;
    int _lastDashboardMinute;

    CommandManager* _commandManager;  // Not owned; injected by Application

    std::vector<IObserver*> _eventAdapters;  // Owned; created in registerObservers(), cleared in reset()

    void tick(bool replayMode, bool advanceTime);
    void updateTrainStates(double dt);
    void checkDepartures();
    void handleStateTransitions();
    void writeSnapshots();
    void cleanupOutputWriters();
    void updateEvents();
    void registerObservers();
    void refreshSimulationState();
    void logEventForAffectedTrains(Event* event, const std::string& action);
    void simulationTick(bool replayMode);
    void applyReplayCommands();
    bool shouldStopEarly(bool replayMode);
    void updateDashboard();
    bool hasValidState(const Train* train) const;
    bool isTrainActive(const Train* train) const;
    bool hasAnyActiveTrain() const;
    void recordCommand(ICommand* cmd);

public:
    using TrainList = std::vector<Train*>;

    SimulationManager();
    ~SimulationManager();

    SimulationManager(const SimulationManager&)            = delete;
    SimulationManager& operator=(const SimulationManager&) = delete;
    SimulationManager(SimulationManager&&)                  = delete;
    SimulationManager& operator=(SimulationManager&&)       = delete;

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