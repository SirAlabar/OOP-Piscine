#ifndef STATS_COLLECTOR_HPP
#define STATS_COLLECTOR_HPP

#include <string>
#include <map>

class Train;

// Metrics for a single train
struct TrainMetrics
{
	std::string trainName;
	double actualTravelTime;      // seconds
	double estimatedTravelTime;   // seconds
	int stateTransitions;
	int waitEvents;
	int emergencyEvents;
	int eventsAffectingTrain;     // Events that actually affected this specific train
	bool reachedDestination;
};

// Metrics for entire simulation
struct SimulationMetrics
{
	unsigned int seed;
	double totalDuration;         // seconds
	int totalEventsGenerated;
	int collisionAvoidanceActivations;
	std::map<std::string, TrainMetrics> trainMetrics;
};

// Collects statistics during simulation run
class StatsCollector
{
private:
	SimulationMetrics _metrics;
	std::map<Train*, std::string> _previousStates;  // Track state changes

public:
	StatsCollector(unsigned int seed);
	
	// Initialize train tracking
	void registerTrain(Train* train, double estimatedTime);
	
	// Track events
	void recordStateTransition(Train* train, const std::string& oldState, const std::string& newState);
	void recordEventGenerated();  // Deprecated - use recordEventForTrain instead
	void recordEventForTrain(Train* train);  // Count events affecting specific train
	void recordCollisionAvoidance();
	void recordWaitEvent(Train* train);
	void recordEmergencyEvent(Train* train);
	
	// Finalize metrics
	void finalize(double totalDuration);
	void checkTrainDestination(Train* train);
	
	// Getters
	const SimulationMetrics& getMetrics() const;
};

#endif