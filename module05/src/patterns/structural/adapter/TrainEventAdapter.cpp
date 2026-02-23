#include "patterns/structural/adapter/TrainEventAdapter.hpp"
#include "core/Train.hpp"
#include "events/Event.hpp"

TrainEventAdapter::TrainEventAdapter(Train* train)
    : _train(train)
{
}

void TrainEventAdapter::onNotify(Event* event)
{
    if (!event || !_train)
    {
        return;
    }

    // Event filtering logic for trains belongs here.
    // Currently a no-op — event effects are handled via rail speed limits
    // and state machine transitions in MovementSystem and state classes.
}

Train* TrainEventAdapter::getTrain() const
{
    return _train;
}