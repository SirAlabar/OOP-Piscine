#include "patterns/observers/EventManager.hpp"
#include "patterns/observers/IObserver.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include <algorithm>

// Initialize static instance
EventManager* EventManager::_instance = nullptr;

EventManager::EventManager() : _totalEventsGenerated(0)
{
}

EventManager::~EventManager()
{
	clear();
}

EventManager& EventManager::getInstance()
{
	if (!_instance)
	{
		_instance = new EventManager();
	}
	return *_instance;
}

void EventManager::destroy()
{
	if (_instance)
	{
		delete _instance;
		_instance = nullptr;
	}
}

void EventManager::attach(IObserver* observer)
{
	if (!observer)
	{
		return;
	}

	// Avoid duplicates
	for (IObserver* obs : _observers)
	{
		if (obs == observer)
		{
			return;
		}
	}

	_observers.push_back(observer);
}

void EventManager::detach(IObserver* observer)
{
	if (!observer)
	{
		return;
	}

	auto it = std::find(_observers.begin(), _observers.end(), observer);
	if (it != _observers.end())
	{
		_observers.erase(it);
	}
}

void EventManager::notify(Event* event)
{
	if (!event)
	{
		return;
	}

	// Notify all observers about the event
	for (IObserver* observer : _observers)
	{
		if (observer)
		{
			observer->onNotify(event);
		}
	}
}

void EventManager::scheduleEvent(Event* event)
{
	if (!event)
	{
		return;
	}

	_scheduledEvents.push_back(event);
	_totalEventsGenerated++;  // Track for statistics
}

void EventManager::update(const Time& currentTime)
{
	// Process scheduled events - move to active if time has come
	auto schedIt = _scheduledEvents.begin();
	while (schedIt != _scheduledEvents.end())
	{
		Event* event = *schedIt;
		
		if (event->shouldBeActive(currentTime))
		{
			// Move to active events
			_activeEvents.push_back(event);
			
			// Update event (will call activate())
			event->update(currentTime);
			
			// Notify observers about new active event
			notify(event);
			
			// Remove from scheduled
			schedIt = _scheduledEvents.erase(schedIt);
		}
		else
		{
			++schedIt;
		}
	}

	// Update active events
	auto activeIt = _activeEvents.begin();
	while (activeIt != _activeEvents.end())
	{
		Event* event = *activeIt;
		
		// Update event state
		event->update(currentTime);
		
		// If event is no longer active, remove it
		if (!event->isActive())
		{
			// Notify observers about event ending
			notify(event);
			
			delete event;
			activeIt = _activeEvents.erase(activeIt);
		}
		else
		{
			++activeIt;
		}
	}
}

const std::vector<Event*>& EventManager::getActiveEvents() const
{
	return _activeEvents;
}

const std::vector<Event*>& EventManager::getScheduledEvents() const
{
	return _scheduledEvents;
}

int EventManager::countActiveEventsByType(EventType type) const
{
	int count = 0;
	for (Event* event : _activeEvents)
	{
		if (event && event->getType() == type)
		{
			count++;
		}
	}
	return count;
}

bool EventManager::hasActiveEventAt(Node* node) const
{
	if (!node)
	{
		return false;
	}

	for (Event* event : _activeEvents)
	{
		if (event && event->affectsNode(node))
		{
			return true;
		}
	}

	return false;
}

bool EventManager::hasActiveEventAt(Rail* rail) const
{
	if (!rail)
	{
		return false;
	}

	for (Event* event : _activeEvents)
	{
		if (event && event->affectsRail(rail))
		{
			return true;
		}
	}

	return false;
}

void EventManager::clear()
{
	// Delete all active events
	for (Event* event : _activeEvents)
	{
		delete event;
	}
	_activeEvents.clear();

	// Delete all scheduled events
	for (Event* event : _scheduledEvents)
	{
		delete event;
	}
	_scheduledEvents.clear();

	// Clear observers
	_observers.clear();
	
	// Reset statistics counter
	_totalEventsGenerated = 0;
}

int EventManager::getTotalEventsGenerated() const
{
	return _totalEventsGenerated;
}