#ifndef SIMULATIONMANAGER_HPP
#define SIMULATIONMANAGER_HPP

#include <vector>
#include <map>
#include "utils/Time.hpp"

// Simulation time configuration (single source of truth)
namespace SimConfig
{
	constexpr double BASE_TIMESTEP_MINUTES = 1.0;
	constexpr double MINUTES_PER_HOUR = 60.0;
	constexpr double MINUTES_PER_DAY = 1440.0;
	
	// Speed limits
	constexpr double MIN_SPEED = 0.1;   // 0.1x (slow motion)
	constexpr double MAX_SPEED = 100.0; // 100x (fast forward)
	constexpr double DEFAULT_SPEED = 10.0;
}

class Train;
class Graph;
class CollisionAvoidance;
class SimulationContext;
class FileOutputWriter;
class IOutputWriter;
class TrafficController;
class EventManager;
class EventFactory;
class Event;
class StatsCollector;

class SimulationManager
{
public:
	using TrainList = std::vector<Train*>;

private:
	SimulationManager();
	
	Graph* _network;
	TrainList _trains;
	CollisionAvoidance* _collisionSystem;
	TrafficController* _trafficController;
	SimulationContext* _context;
	EventFactory* _eventFactory;
	StatsCollector* _statsCollector;  // Optional: for Monte Carlo mode only
	
	double _currentTime;  // Stored in MINUTES (not seconds)
	double _timestep;     // Stored in MINUTES (default 1.0)
	double _simulationSpeed;  // Runtime speed control (1.0 = real-time, 10.0 = 10x)
	bool _running;
	bool _roundTripEnabled;  // Enable train reversal at destination
	unsigned int _eventSeed;  // For deterministic event generation
	double _lastEventGenerationTime;  // Track when we last generated events (once per minute)
	
	// Output management
	IOutputWriter* _simulationWriter;  // Console/UI output (Dependency Inversion)
	std::map<Train*, FileOutputWriter*> _outputWriters;
	std::map<Train*, std::string> _previousStates;  // Track previous state for each train
	int _lastSnapshotMinute;
	int _lastDashboardMinute;  // Track dashboard display timing
	
	void updateTrainStates(double dt);
	void checkDepartures();
	void handleStateTransitions();
	void writeSnapshots();
	void cleanupOutputWriters();
	void updateEvents();  // Update event manager and generate new events
	void registerObservers();  // Register trains/rails/nodes as observers
	void logEventForAffectedTrains(Event* event, const std::string& action);  // Log events to train outputs

public:
	static SimulationManager& getInstance()
	{
		static SimulationManager instance;
		return instance;
	}
	
	SimulationManager(const SimulationManager&) = delete;
	SimulationManager& operator=(const SimulationManager&) = delete;
	SimulationManager(SimulationManager&&) = delete;
	SimulationManager& operator=(SimulationManager&&) = delete;
	
	~SimulationManager();
	
	void setNetwork(Graph* network);
	void addTrain(Train* train);
	void setTimestep(double timestep);
	void setEventSeed(unsigned int seed);  // Set seed for deterministic events
	void setRoundTripMode(bool enabled);  // Enable train reversal at destination
	void setSimulationWriter(IOutputWriter* writer);  // Set console/UI writer (Dependency Inversion)
	void registerOutputWriter(Train* train, FileOutputWriter* writer);  // Register writer from Application
	void setStatsCollector(StatsCollector* stats);  // Set stats collector for Monte Carlo mode
	
	void start();
	void stop();
	void step();
	void run(double maxTime);
	
	double getCurrentTime() const;
	Time getCurrentTimeFormatted() const;
	const TrainList& getTrains() const;
	const Graph* getNetwork() const;
	bool isRunning() const;
	unsigned int getSeed() const;
	
	// Simulation speed control
	double getSimulationSpeed() const;
	void setSimulationSpeed(double speed);

	SimulationContext* getContext() const { return _context; }

	void reset();
};

#endif