#ifndef EVENTMANAGER_HPP
#define EVENTMANAGER_HPP

#include "ISubject.hpp"
#include "patterns/events/Event.hpp"
#include "utils/Time.hpp"
#include <vector>

class IObserver;

// Singleton manager for events using Observer Pattern
class EventManager : public ISubject
{
private:
	static EventManager* _instance;

	std::vector<IObserver*> _observers;
	std::vector<Event*>     _activeEvents;
	std::vector<Event*>     _scheduledEvents;  // Events waiting to start

	// Private constructor for Singleton
	EventManager();

	// Delete copy/move
	EventManager(const EventManager&) = delete;
	EventManager& operator=(const EventManager&) = delete;

public:
	~EventManager();

	// Singleton access
	static EventManager& getInstance();
	static void destroy();

	// ISubject implementation
	void attach(IObserver* observer) override;
	void detach(IObserver* observer) override;
	void notify(Event* event) override;

	// Event lifecycle management
	void scheduleEvent(Event* event);
	void update(const Time& currentTime);

	// Query methods
	const std::vector<Event*>& getActiveEvents() const;
	const std::vector<Event*>& getScheduledEvents() const;
	
	// Count active events by type
	int countActiveEventsByType(EventType type) const;

	// Check if specific location has active event
	bool hasActiveEventAt(Node* node) const;
	bool hasActiveEventAt(Rail* rail) const;

	// Clear all events (for testing/reset)
	void clear();
};

#endif