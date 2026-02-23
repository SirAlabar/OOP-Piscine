#ifndef TRAINEVENTADAPTER_HPP
#define TRAINEVENTADAPTER_HPP

#include "patterns/behavioral/observers/IObserver.hpp"

class Train;
class Event;

// Receives event notifications on behalf of a specific train.
class TrainEventAdapter : public IObserver
{
public:
    explicit TrainEventAdapter(Train* train);
    ~TrainEventAdapter() override = default;

    void onNotify(Event* event) override;

    Train* getTrain() const;

private:
    Train* _train;
};

#endif