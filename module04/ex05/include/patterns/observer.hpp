#ifndef OBSERVER_HPP
#define OBSERVER_HPP

#include "events/Event.hpp"

class IObserver
{
public:
	virtual ~IObserver() {}
	virtual void update(Event event) = 0;
};

#endif