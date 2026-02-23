#ifndef EVENT_HPP
#define EVENT_HPP

#include <string>
#include "utils/Time.hpp"

class Node;
class Rail;
class Train;

// Event types for factory and conflict checking
enum class EventType
{
    STATION_DELAY,
    TRACK_MAINTENANCE,
    SIGNAL_FAILURE,
    WEATHER
};

// Visual data for future isometric rendering
struct VisualData
{
    Node*       centerNode;
    double      radius;
    std::string iconType;
    float       animationSpeed;

    VisualData()
        : centerNode(nullptr), radius(0.0), iconType(""), animationSpeed(1.0f)
    {
    }
};

// Abstract base class for all events
class Event
{
protected:
    EventType   _type;
    Time        _startTime;
    Time        _duration;
    bool        _isActive;
    VisualData  _visualData;

public:
    Event(EventType type, const Time& startTime, const Time& duration);
    virtual ~Event() = default;

    virtual void activate()   = 0;
    virtual void deactivate() = 0;

    bool shouldBeActive(const Time& currentTime) const;
    void update(const Time& currentTime);

    EventType         getType()        const;
    Time              getStartTime()   const;
    Time              getDuration()    const;
    Time              getEndTime()     const;
    bool              isActive()       const;
    const VisualData& getVisualData()  const;

    virtual std::string getDescription()                   const = 0;
    virtual bool        affectsNode(Node* node)            const = 0;
    virtual bool        affectsRail(Rail* rail)            const = 0;
    virtual bool        affectsTrain(Train* train)         const = 0;
    virtual bool        isApplicableToTrain(Train* train)  const = 0;

    virtual const Node*  getAnchorNode()  const = 0;
    virtual const Rail*  getAnchorRail()  const = 0;

    // Convert an EventType enum to its human-readable display string.
    // Centralises the mapping so no caller needs a local switch.
    static std::string typeToString(EventType type);
};

#endif