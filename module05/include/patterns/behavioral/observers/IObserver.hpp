#ifndef IOBSERVER_HPP
#define IOBSERVER_HPP

class Event;

// Observer interface - objects that react to events
class IObserver
{
public:
	virtual ~IObserver() = default;

	// Called when an event occurs
	// Observer checks if event affects it and applies effects
	virtual void onNotify(Event* event) = 0;
};

#endif