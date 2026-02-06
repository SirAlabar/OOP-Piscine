#ifndef TRAIN_HPP
#define TRAIN_HPP

#include <string>
#include <vector>
#include "utils/Time.hpp"

class Rail;
class ITrainState;
class Time;
class Node;

// Path segment with explicit direction
struct PathSegment
{
	Rail* rail;
	Node* from;  // Source node
	Node* to;    // Destination node
};

// Represents a train with physical properties and journey parameters
class Train
{
public:
	using Path = std::vector<PathSegment>;

private:
	// Identity
	std::string _name;
	int         _id;  // Sequential deterministic ID
	bool _finished;

	// Physical properties
	double _mass;           // tons
	double _frictionCoef;   // dimensionless (0.001-0.01 typical)
	double _maxAccelForce;  // kN
	double _maxBrakeForce;  // kN

	// Current motion state
	double _velocity;  // m/s (starts at 0)
	double _position;  // meters along current rail (starts at 0)

	// Journey properties
	std::string _departureStation;
	std::string _arrivalStation;
	Time        _departureTime;   // HHhMM format
	Time        _stopDuration;    // HHhMM format

	// Path (computed by pathfinding)
	Path   _path;
	size_t _currentRailIndex;

	// State pattern
	ITrainState* _currentState;

	// Static ID counter for deterministic sequential IDs
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

	// Identity getters
	std::string getName() const;
	int         getID() const;
	bool		isFinished() const;
	void		markFinished();

	// Physical property getters
	double getMass() const;
	double getFrictionCoef() const;
	double getMaxAccelForce() const;
	double getMaxBrakeForce() const;

	// Motion state getters/setters
	double getVelocity() const;
	double getPosition() const;
	void   setVelocity(double velocity);
	void   setPosition(double position);

	// Journey getters
	std::string getDepartureStation() const;
	std::string getArrivalStation() const;
	Time        getDepartureTime() const;
	Time        getStopDuration() const;

	// Path management
	const Path& getPath() const;
	void        setPath(const Path& path);
	Rail*       getCurrentRail() const;
	size_t      getCurrentRailIndex() const;
	void        advanceToNextRail();

	// State management
	ITrainState* getCurrentState() const;
	void         setState(ITrainState* state);

	// Validation
	bool isValid() const;

	// Static ID management
	static void  resetIDCounter();  // For testing
	static int   getNextID();       // Peek at next ID without incrementing

    // Update (delegates to current state)
    void update(double dt);
};

#endif