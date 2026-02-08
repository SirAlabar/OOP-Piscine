#ifndef SIGNALFAILUREEVENT_HPP
#define SIGNALFAILUREEVENT_HPP

#include "Event.hpp"
#include "utils/Time.hpp"

// Event: Node requires all trains to stop for specified duration
class SignalFailureEvent : public Event
{
private:
	Node* _node;             // Which node has signal failure
	Time  _stopDuration;     // Forced stop time

public:
	SignalFailureEvent(Node* node, const Time& startTime,
	                   const Time& duration, const Time& stopDuration);

	void activate() override;
	void deactivate() override;

	bool affectsNode(Node* node) const override;
	bool affectsRail(Rail* rail) const override;
	bool affectsTrain(Train* train) const override;
	bool isApplicableToTrain(Train* train) const override;
	
	std::string getDescription() const override;

	// Getters
	Node* getNode() const;
	Time getStopDuration() const;
};

#endif