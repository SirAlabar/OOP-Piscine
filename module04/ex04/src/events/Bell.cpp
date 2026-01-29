#include "events/Bell.hpp"
#include <iostream>
#include <algorithm>

#define CYAN "\033[36m"
#define RESET "\033[0m"

Bell::Bell()
{
}

Bell::~Bell()
{
}

void Bell::subscribe(IObserver* observer)
{
	if (observer)
    {
		_observers.push_back(observer);
    }
}

void Bell::unsubscribe(IObserver* observer)
{
	std::vector<IObserver*>::iterator it = std::find(_observers.begin(), _observers.end(), observer);
	if (it != _observers.end())
    {
		_observers.erase(it);
    }
}

void Bell::notify(Event event)
{
	for (std::vector<IObserver*>::iterator it = _observers.begin(); it != _observers.end(); ++it)
	{
		(*it)->update(event);
	}
}

void Bell::ring()
{
	std::cout << CYAN << "🔔 *RING RING* Bell is ringing!" << RESET << std::endl;
	notify(RingBell);
}