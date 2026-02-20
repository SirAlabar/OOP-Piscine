#ifndef RAILEVENTADAPTER_HPP
#define RAILEVENTADAPTER_HPP

#include "patterns/observers/IObserver.hpp"

class Rail;
class Event;

// Receives event notifications on behalf of a specific rail segment.
class RailEventAdapter : public IObserver
{
public:
    explicit RailEventAdapter(Rail* rail);
    ~RailEventAdapter() override = default;

    void onNotify(Event* event) override;

    Rail* getRail() const;

private:
    Rail* _rail;
};

#endif