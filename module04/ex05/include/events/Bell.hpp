#ifndef BELL_HPP
#define BELL_HPP

#include <vector>
#include "patterns/observer.hpp"

class Bell
{
private:
	std::vector<IObserver*> _observers;

public:
	Bell();
	~Bell();
	
	void subscribe(IObserver* observer);
	void unsubscribe(IObserver* observer);
	void notify(Event event);
	void ring();
};

#endif