#ifndef OBSERVERMANAGER_HPP
#define OBSERVERMANAGER_HPP

#include <vector>

class Train;
class Graph;
class IObserver;
class EventDispatcher;

// Owns the creation and lifecycle of TrainEventAdapter and RailEventAdapter instances.
// Attaches all observers to the dispatcher on wire() and cleans up on clear().
class ObserverManager
{
private:
    EventDispatcher&        _dispatcher;
    std::vector<IObserver*> _adapters;

public:
    explicit ObserverManager(EventDispatcher& dispatcher);
    ~ObserverManager();

    // Create adapters for all trains and rails, attach nodes directly to dispatcher.
    void wire(const std::vector<Train*>& trains, const Graph* network);

    // Detach and delete all adapters; clear dispatcher observers.
    void clear();
};

#endif