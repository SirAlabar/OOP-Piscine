#ifndef TRAIN_HPP
#define TRAIN_HPP

#include <string>
#include <vector>
#include "utils/Time.hpp"

class Rail;
class ITrainState;
class Time;
class Node;
class Event;

// Path segment with explicit direction.
struct PathSegment
{
    Rail* rail;
    Node* from;
    Node* to;
};

// Represents a train with physical properties and journey parameters.
class Train
{
public:
    using Path = std::vector<PathSegment>;

private:
    // Identity
    std::string _name;
    int         _id;
    bool        _finished;

    // Physical properties
    double _mass;           // tons
    double _frictionCoef;   // dimensionless (0.001-0.01 typical)
    double _maxAccelForce;  // kN
    double _maxBrakeForce;  // kN

    // Current motion state
    double _velocity;  // m/s
    double _position;  // metres along current rail

    // Journey properties
    std::string _departureStation;
    std::string _arrivalStation;
    Time        _departureTime;
    Time        _stopDuration;

    // Path (computed by pathfinding)
    Path   _path;
    size_t _currentRailIndex;

    // State pattern
    ITrainState* _currentState;

    static int _nextID;

public:
    Train();
    Train(const std::string& name, double mass, double frictionCoef,
          double maxAccelForce, double maxBrakeForce,
          const std::string& departureStation, const std::string& arrivalStation,
          const Time& departureTime, const Time& stopDuration);
    Train(const Train& other);
    Train& operator=(const Train& other);
    ~Train() = default;

    // Identity
    std::string getName()     const;
    int         getID()       const;
    bool        isFinished()  const;
    void        markFinished();

    // Physical properties
    double getMass()          const;
    double getFrictionCoef()  const;
    double getMaxAccelForce() const;
    double getMaxBrakeForce() const;

    // Motion state
    double getVelocity()            const;
    double getPosition()            const;
    void   setVelocity(double v);
    void   setPosition(double p);

    // Journey
    std::string getDepartureStation() const;
    std::string getArrivalStation()   const;
    Time        getDepartureTime()    const;
    Time        getStopDuration()     const;
    void        setDepartureTime(const Time& time);

    // Path management
    const Path&        getPath()               const;
    void               setPath(const Path& path);
    Rail*              getCurrentRail()         const;
    const PathSegment* getCurrentPathSegment()  const;
    size_t             getCurrentRailIndex()    const;
    void               advanceToNextRail();
    void               reverseJourney();

    // State management
    ITrainState* getCurrentState() const;
    void         setState(ITrainState* state);

    // Validation
    bool isValid() const;

    // Static ID management
    static void resetIDCounter();
    static int  getNextID();

    // Update delegates to current state.
    void update(double dt);

    // Helpers for collision/context queries.
    Node* getCurrentNode() const;
    Node* getNextNode()    const;
};

#endif