#ifndef EVENTMANAGER_HPP
#define EVENTMANAGER_HPP

#include "ISubject.hpp"
#include "simulation/IEventScheduler.hpp"
#include "patterns/events/Event.hpp"
#include "utils/Time.hpp"
#include <vector>

class IObserver;

// Singleton manager for events using Observer Pattern.
// Implements both ISubject (observer registration + notification) and
// IEventScheduler (event lifecycle management).
// Downstream code should depend on IEventScheduler where possible.
class EventManager : public ISubject, public IEventScheduler
{
private:
	static EventManager* _instance;

	std::vector<IObserver*> _observers;
	std::vector<Event*>     _activeEvents;
	std::vector<Event*>     _scheduledEvents;  // Events waiting to start
	int                     _totalEventsGenerated;  // Lifetime count for stats

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

	// IEventScheduler implementation
	void scheduleEvent(Event* event)                      override;
	void update(const Time& currentTime)                  override;
	const std::vector<Event*>& getActiveEvents()    const override;
	const std::vector<Event*>& getScheduledEvents() const override;
	int  countActiveEventsByType(EventType type)    const override;
	bool hasActiveEventAt(Node* node)               const override;
	bool hasActiveEventAt(Rail* rail)               const override;
	int  getTotalEventsGenerated()                  const override;
	void clear()                                          override;
};

#endif