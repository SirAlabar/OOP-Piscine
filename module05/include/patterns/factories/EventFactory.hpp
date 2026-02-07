#ifndef EVENTFACTORY_HPP
#define EVENTFACTORY_HPP

#include "patterns/events/Event.hpp"
#include "utils/SeededRNG.hpp"
#include "utils/Time.hpp"
#include <vector>

class Graph;
class EventManager;

// Event configuration
struct EventConfig
{
	double probabilityPerHour;  // 0.0-1.0
	int    minDurationMinutes;
	int    maxDurationMinutes;
};

// Factory for creating random events using seeded RNG
class EventFactory
{
private:
	SeededRNG     _rng;
	Graph*        _network;
	EventManager* _eventManager;  // For conflict checking

	// Event configurations
	static const EventConfig CONFIG_STATION_DELAY;
	static const EventConfig CONFIG_TRACK_MAINTENANCE;
	static const EventConfig CONFIG_SIGNAL_FAILURE;
	static const EventConfig CONFIG_WEATHER;

	// Helper methods
	Event* createStationDelay(const Time& currentTime);
	Event* createTrackMaintenance(const Time& currentTime);
	Event* createSignalFailure(const Time& currentTime);
	Event* createWeather(const Time& currentTime);

	// Generate random duration based on config
	Time generateDuration(const EventConfig& config);

	// Conflict checking helpers
	bool canCreateStationDelay(Node* node) const;
	bool canCreateTrackMaintenance(Rail* rail) const;
	bool canCreateSignalFailure(Node* node) const;
	bool canCreateWeather() const;

public:
	EventFactory(unsigned int seed, Graph* network, EventManager* eventManager);

	// Attempt to generate events based on probability
	// Returns vector of created events (may be empty)
	std::vector<Event*> tryGenerateEvents(const Time& currentTime);

	// Get the seed used
	unsigned int getSeed() const;
};

#endif