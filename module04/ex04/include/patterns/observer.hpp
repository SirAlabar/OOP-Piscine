#ifndef OBSERVER_HPP
#define OBSERVER_HPP

#include "enums/Event.hpp"

class IObserver
{
public:
	virtual ~IObserver() {}
	virtual void update(Event event) = 0;
};

#endif