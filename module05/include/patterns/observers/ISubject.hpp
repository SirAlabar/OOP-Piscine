#ifndef ISUBJECT_HPP
#define ISUBJECT_HPP

class IObserver;
class Event;

// Subject interface - objects that notify observers about events
class ISubject
{
public:
	virtual ~ISubject() = default;

	// Observer management
	virtual void attach(IObserver* observer) = 0;
	virtual void detach(IObserver* observer) = 0;

	// Notify all observers about an event
	virtual void notify(Event* event) = 0;
};

#endif