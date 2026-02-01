#ifndef SIMULATIONMANAGER_HPP
#define SIMULATIONMANAGER_HPP

#include <vector>
#include "utils/Time.hpp"

class Train;
class Graph;
class CollisionAvoidance;
class SimulationContext;

class SimulationManager
{
public:
	using TrainList = std::vector<Train*>;

private:
	SimulationManager();
	
	Graph* _network;
	TrainList _trains;
	CollisionAvoidance* _collisionSystem;
	SimulationContext* _context;
	
	double _currentTime;
	double _timestep;
	bool _running;
	
	void updateTrainStates(double dt);
	void checkDepartures();
	void handleStateTransitions();

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
	
	void start();
	void stop();
	void step();
	void run(double maxTime);
	
	double getCurrentTime() const;
	Time getCurrentTimeFormatted() const;
	const TrainList& getTrains() const;
	const Graph* getNetwork() const;
	bool isRunning() const;
	
	void reset();
};

#endif