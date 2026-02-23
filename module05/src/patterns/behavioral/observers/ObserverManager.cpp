#include "event_system/ObserverManager.hpp"
#include "event_system/EventDispatcher.hpp"
#include "patterns/structural/adapter/TrainEventAdapter.hpp"
#include "patterns/structural/adapter/RailEventAdapter.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "core/Node.hpp"
#include "core/Graph.hpp"

ObserverManager::ObserverManager(EventDispatcher& dispatcher)
    : _dispatcher(dispatcher)
{
}

ObserverManager::~ObserverManager()
{
    clear();
}

void ObserverManager::wire(const std::vector<Train*>& trains, const Graph* network)
{
    for (Train* train : trains)
    {
        if (train)
        {
            IObserver* adapter = new TrainEventAdapter(train);
            _adapters.push_back(adapter);
            _dispatcher.attach(adapter);
        }
    }

    if (!network)
    {
        return;
    }

    for (Node* node : network->getNodes())
    {
        if (node)
        {
            _dispatcher.attach(node);
        }
    }

    for (Rail* rail : network->getRails())
    {
        if (rail)
        {
            IObserver* adapter = new RailEventAdapter(rail);
            _adapters.push_back(adapter);
            _dispatcher.attach(adapter);
        }
    }
}

void ObserverManager::clear()
{
    _dispatcher.clearObservers();

    for (IObserver* adapter : _adapters)
    {
        delete adapter;
    }

    _adapters.clear();
}