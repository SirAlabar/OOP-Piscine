#ifndef EVENT_HPP
#define EVENT_HPP

#include <string>
#include "utils/Time.hpp"

class Node;
class Rail;
class Train;

// Event types for factory and conflict checking
enum class EventType
{
	STATION_DELAY,
	TRACK_MAINTENANCE,
	SIGNAL_FAILURE,
	WEATHER
};

// Visual data for future isometric rendering
struct VisualData
{
	Node*       centerNode;      // Positioning anchor
	double      radius;          // Area of effect (km) - 0 for point events
	std::string iconType;        // "storm", "maintenance", "signal", "delay"
	float       animationSpeed;  // Speed multiplier for particle effects

	VisualData()
		: centerNode(nullptr), radius(0.0), iconType(""), animationSpeed(1.0f)
	{
	}
};

// Abstract base class for all events
class Event
{
protected:
	EventType   _type;
	Time        _startTime;
	Time        _duration;      // How long event lasts
	bool        _isActive;      // Currently affecting simulation
	VisualData  _visualData;    // For isometric rendering

public:
	Event(EventType type, const Time& startTime, const Time& duration);
	virtual ~Event() = default;

	// Event lifecycle
	virtual void activate() = 0;      // Apply event effects
	virtual void deactivate() = 0;    // Remove event effects
	
	// Check if event should be active at given time
	bool shouldBeActive(const Time& currentTime) const;
	
	// Update event state based on current time
	void update(const Time& currentTime);

	// Getters
	EventType getType() const;
	Time getStartTime() const;
	Time getDuration() const;
	Time getEndTime() const;
	bool isActive() const;
	const VisualData& getVisualData() const;
	
	// Event description for logging
	virtual std::string getDescription() const = 0;

	// Type checking helpers
	virtual bool affectsNode(Node* node) const = 0;
	virtual bool affectsRail(Rail* rail) const = 0;
	virtual bool affectsTrain(Train* train) const = 0;
	
	// Check if event is applicable to train at current position/state
	virtual bool isApplicableToTrain(Train* train) const = 0;
};

#endif