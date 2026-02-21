#include "patterns/factories/EventFactory.hpp"
#include "simulation/IEventScheduler.hpp"
#include "patterns/events/StationDelayEvent.hpp"
#include "patterns/events/TrackMaintenanceEvent.hpp"
#include "patterns/events/SignalFailureEvent.hpp"
#include "patterns/events/WeatherEvent.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include <vector>

const EventConfig EventFactory::CONFIG_STATION_DELAY = {0.03, 15, 45};       // 3% per minute (~1.8/hour)
const EventConfig EventFactory::CONFIG_TRACK_MAINTENANCE = {0.015, 60, 180}; // 1.5% per minute (~0.9/hour)
const EventConfig EventFactory::CONFIG_SIGNAL_FAILURE = {0.01, 5, 20};       // 1% per minute (~0.6/hour)
const EventConfig EventFactory::CONFIG_WEATHER = {0.005, 120, 300};          // 0.5% per minute (~0.3/hour)

EventFactory::EventFactory(unsigned int seed, Graph* network, IEventScheduler* eventScheduler)
	: _rng(seed), _network(network), _eventManager(eventScheduler)
{
}

std::vector<Event*> EventFactory::tryGenerateEvents(const Time& currentTime, double timestepSeconds)
{
	(void)timestepSeconds;  // Probabilities are per-minute; this is only called once per minute
	std::vector<Event*> newEvents;

	// Try each event type based on per-minute probability
	if (_rng.getBool(CONFIG_STATION_DELAY.probabilityPerTimestep))
	{
		Event* event = createStationDelay(currentTime);
		if (event)
		{
			newEvents.push_back(event);
		}
	}

	if (_rng.getBool(CONFIG_TRACK_MAINTENANCE.probabilityPerTimestep))
	{
		Event* event = createTrackMaintenance(currentTime);
		if (event)
		{
			newEvents.push_back(event);
		}
	}

	if (_rng.getBool(CONFIG_SIGNAL_FAILURE.probabilityPerTimestep))
	{
		Event* event = createSignalFailure(currentTime);
		if (event)
		{
			newEvents.push_back(event);
		}
	}

	if (_rng.getBool(CONFIG_WEATHER.probabilityPerTimestep))
	{
		Event* event = createWeather(currentTime);
		if (event)
		{
			newEvents.push_back(event);
		}
	}

	return newEvents;
}

Event* EventFactory::createStationDelay(const Time& currentTime)
{
	if (!_network)
	{
		return nullptr;
	}

	const auto& nodes = _network->getNodes();
	if (nodes.empty())
	{
		return nullptr;
	}

	// Filter to get only CITY nodes (stations)
	std::vector<Node*> cityNodes;
	for (Node* node : nodes)
	{
		if (node && node->getType() == NodeType::CITY)
		{
			cityNodes.push_back(node);
		}
	}
	
	if (cityNodes.empty())
	{
		return nullptr;
	}

	// Pick random CITY (station)
	int cityIndex = _rng.getInt(0, static_cast<int>(cityNodes.size()) - 1);
	Node* station = cityNodes[cityIndex];

	// Check conflicts
	if (!canCreateStationDelay(station))
	{
		return nullptr;
	}

	// Generate event parameters
	Time duration = generateDuration(CONFIG_STATION_DELAY);
	int delayMinutes = _rng.getInt(CONFIG_STATION_DELAY.minDurationMinutes,
	                               CONFIG_STATION_DELAY.maxDurationMinutes);
	Time additionalDelay(0, delayMinutes);

	return new StationDelayEvent(station, currentTime, duration, additionalDelay);
}

Event* EventFactory::createTrackMaintenance(const Time& currentTime)
{
	if (!_network)
	{
		return nullptr;
	}

	const auto& rails = _network->getRails();
	if (rails.empty())
	{
		return nullptr;
	}

	// Pick random rail
	int railIndex = _rng.getInt(0, static_cast<int>(rails.size()) - 1);
	Rail* rail = rails[railIndex];

	// Check conflicts
	if (!canCreateTrackMaintenance(rail))
	{
		return nullptr;
	}

	// Generate event parameters
	Time duration = generateDuration(CONFIG_TRACK_MAINTENANCE);
	double speedReduction = _rng.getDouble(0.4, 0.7);  // 40-70% of original speed

	return new TrackMaintenanceEvent(rail, currentTime, duration, speedReduction);
}

Event* EventFactory::createSignalFailure(const Time& currentTime)
{
	if (!_network)
	{
		return nullptr;
	}

	const auto& nodes = _network->getNodes();
	if (nodes.empty())
	{
		return nullptr;
	}

	// Pick random node
	int nodeIndex = _rng.getInt(0, static_cast<int>(nodes.size()) - 1);
	Node* node = nodes[nodeIndex];

	// Check conflicts
	if (!canCreateSignalFailure(node))
	{
		return nullptr;
	}

	// Generate event parameters
	Time duration = generateDuration(CONFIG_SIGNAL_FAILURE);
	int stopMinutes = _rng.getInt(CONFIG_SIGNAL_FAILURE.minDurationMinutes,
	                              CONFIG_SIGNAL_FAILURE.maxDurationMinutes);
	Time stopDuration(0, stopMinutes);

	return new SignalFailureEvent(node, currentTime, duration, stopDuration);
}

Event* EventFactory::createWeather(const Time& currentTime)
{
	if (!_network)
	{
		return nullptr;
	}

	// Check if weather can be created (only 1 active weather)
	if (!canCreateWeather())
	{
		return nullptr;
	}

	const auto& nodes = _network->getNodes();
	if (nodes.empty())
	{
		return nullptr;
	}

	// Pick random center
	int nodeIndex = _rng.getInt(0, static_cast<int>(nodes.size()) - 1);
	Node* centerNode = nodes[nodeIndex];

	// Generate event parameters
	Time duration = generateDuration(CONFIG_WEATHER);
	double radius = _rng.getDouble(20.0, 50.0);  // 20-50 km radius
	double speedReduction = _rng.getDouble(0.5, 0.8);  // 50-80% of original
	double frictionIncrease = _rng.getDouble(0.01, 0.03);  // +0.01 to +0.03

	// Weather type selection
	const char* weatherTypes[] = {"Heavy Rain", "Storm", "Snow", "Fog"};
	int typeIndex = _rng.getInt(0, 3);

	WeatherEvent* event = new WeatherEvent(weatherTypes[typeIndex], centerNode,
	                                       currentTime, duration, radius,
	                                       speedReduction, frictionIncrease);

	// Calculate affected rails (within radius)
	std::vector<Rail*> affectedRails;
	const auto& rails = _network->getRails();
	for (Rail* rail : rails)
	{
		if (rail && (rail->getNodeA() == centerNode || rail->getNodeB() == centerNode ||
		             rail->getLength() <= radius))
		{
			affectedRails.push_back(rail);
		}
	}

	event->setAffectedRails(affectedRails);

	return event;
}

Time EventFactory::generateDuration(const EventConfig& config)
{
	int durationMinutes = _rng.getInt(config.minDurationMinutes, config.maxDurationMinutes);
	return Time(durationMinutes / 60, durationMinutes % 60);
}

bool EventFactory::canCreateStationDelay(Node* node) const
{
	if (!_eventManager)
	{
		return true;
	}

	// Check if node already has station delay or signal failure
	const auto& activeEvents = _eventManager->getActiveEvents();
	for (Event* event : activeEvents)
	{
		if (event->affectsNode(node))
		{
			EventType type = event->getType();
			if (type == EventType::STATION_DELAY || type == EventType::SIGNAL_FAILURE)
			{
				return false;  // Cannot have both on same node
			}
		}
	}

	return true;
}

bool EventFactory::canCreateTrackMaintenance(Rail* rail) const
{
	if (!_eventManager)
	{
		return true;
	}

	// Check max maintenance events (limit: 3)
	int maintenanceCount = 0;
	const auto& activeEvents = _eventManager->getActiveEvents();
	for (Event* event : activeEvents)
	{
		if (event->getType() == EventType::TRACK_MAINTENANCE)
		{
			maintenanceCount++;
			
			// Check if this specific rail already has maintenance or weather
			if (event->affectsRail(rail))
			{
				return false;
			}
		}
		// Cannot have maintenance if rail has weather
		if (event->getType() == EventType::WEATHER && event->affectsRail(rail))
		{
			return false;
		}
	}

	return maintenanceCount < 3;
}

bool EventFactory::canCreateSignalFailure(Node* node) const
{
	if (!_eventManager)
	{
		return true;
	}

	// Check max signal failures (limit: 2)
	int signalCount = 0;
	const auto& activeEvents = _eventManager->getActiveEvents();
	for (Event* event : activeEvents)
	{
		if (event->getType() == EventType::SIGNAL_FAILURE)
		{
			signalCount++;
			
			// Check if this node already has signal or station delay
			if (event->affectsNode(node))
			{
				return false;
			}
		}
		// Cannot have signal if node has station delay
		if (event->getType() == EventType::STATION_DELAY && event->affectsNode(node))
		{
			return false;
		}
	}

	return signalCount < 2;
}

bool EventFactory::canCreateWeather() const
{
	if (!_eventManager)
	{
		return true;
	}

	// Only 1 active weather event allowed
	const auto& activeEvents = _eventManager->getActiveEvents();
	for (Event* event : activeEvents)
	{
		if (event->getType() == EventType::WEATHER)
		{
			return false;
		}
	}

	return true;
}

unsigned int EventFactory::getSeed() const
{
	return _rng.getSeed();
}