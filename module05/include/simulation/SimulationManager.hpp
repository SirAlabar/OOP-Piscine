#ifndef SIMULATIONMANAGER_HPP
#define SIMULATIONMANAGER_HPP

#include <vector>
#include <map>
#include "utils/Time.hpp"

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
	
	double _currentTime;
	double _timestep;
	bool _running;
	bool _roundTripEnabled;  // Enable train reversal at destination
	unsigned int _eventSeed;  // For deterministic event generation
	
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
	
	void start();
	void stop();
	void step();
	void run(double maxTime);
	
	double getCurrentTime() const;
	Time getCurrentTimeFormatted() const;
	const TrainList& getTrains() const;
	const Graph* getNetwork() const;
	bool isRunning() const;

	SimulationContext* getContext() const { return _context; }

	void reset();
};

#endif