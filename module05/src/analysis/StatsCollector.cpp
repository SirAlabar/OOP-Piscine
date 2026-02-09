#include "analysis/StatsCollector.hpp"
#include "core/Train.hpp"

StatsCollector::StatsCollector(unsigned int seed)
{
	_metrics.seed = seed;
	_metrics.totalDuration = 0.0;
	_metrics.totalEventsGenerated = 0;
	_metrics.collisionAvoidanceActivations = 0;
}

void StatsCollector::registerTrain(Train* train, double estimatedTime)
{
	if (!train)
	{
		return;
	}
	
	TrainMetrics metrics;
	metrics.trainName = train->getName();
	metrics.actualTravelTime = 0.0;
	metrics.estimatedTravelTime = estimatedTime;
	metrics.stateTransitions = 0;
	metrics.waitEvents = 0;
	metrics.emergencyEvents = 0;
	metrics.eventsAffectingTrain = 0;
	metrics.reachedDestination = false;
	
	_metrics.trainMetrics[train->getName()] = metrics;
	_previousStates[train] = "Idle";  // Initial state
}

void StatsCollector::recordStateTransition(Train* train, const std::string& oldState, const std::string& newState)
{
	(void)oldState;
	
	if (!train)
	{
		return;
	}
	
	auto it = _metrics.trainMetrics.find(train->getName());
	if (it != _metrics.trainMetrics.end())
	{
		it->second.stateTransitions++;
		
		// Track wait events (transition to Waiting state)
		if (newState == "Waiting")
		{
			it->second.waitEvents++;
		}
		
		// Track emergency events (transition to Emergency state)
		if (newState == "Emergency")
		{
			it->second.emergencyEvents++;
		}
	}
}

void StatsCollector::recordEventGenerated()
{
	_metrics.totalEventsGenerated++;
}

void StatsCollector::recordEventForTrain(Train* train)
{
	if (!train)
	{
		return;
	}
	
	auto it = _metrics.trainMetrics.find(train->getName());
	if (it != _metrics.trainMetrics.end())
	{
		it->second.eventsAffectingTrain++;
	}
}

void StatsCollector::recordCollisionAvoidance()
{
	_metrics.collisionAvoidanceActivations++;
}

void StatsCollector::recordWaitEvent(Train* train)
{
	if (!train)
	{
		return;
	}
	
	auto it = _metrics.trainMetrics.find(train->getName());
	if (it != _metrics.trainMetrics.end())
	{
		it->second.waitEvents++;
	}
}

void StatsCollector::recordEmergencyEvent(Train* train)
{
	if (!train)
	{
		return;
	}
	
	auto it = _metrics.trainMetrics.find(train->getName());
	if (it != _metrics.trainMetrics.end())
	{
		it->second.emergencyEvents++;
	}
}

void StatsCollector::finalize(double totalDuration)
{
	_metrics.totalDuration = totalDuration;
}

void StatsCollector::checkTrainDestination(Train* train)
{
	if (!train)
	{
		return;
	}
	
	auto it = _metrics.trainMetrics.find(train->getName());
	if (it != _metrics.trainMetrics.end())
	{
		it->second.reachedDestination = train->isFinished();
	}
}

const SimulationMetrics& StatsCollector::getMetrics() const
{
	return _metrics;
}