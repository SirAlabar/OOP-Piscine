#ifndef STATIONDELAYEVENT_HPP
#define STATIONDELAYEVENT_HPP

#include "Event.hpp"
#include "utils/Time.hpp"

// Event: Trains must wait additional time when stopping at affected station
class StationDelayEvent : public Event
{
private:
	Node* _station;           // Which station is affected
	Time  _additionalDelay;   // Extra waiting time

public:
	StationDelayEvent(Node* station, const Time& startTime, 
	                  const Time& duration, const Time& additionalDelay);

	void activate() override;
	void deactivate() override;

	bool affectsNode(Node* node) const override;
	bool affectsRail(Rail* rail) const override;
	bool affectsTrain(Train* train) const override;
	
	std::string getDescription() const override;

	// Getters
	Node* getStation() const;
	Time getAdditionalDelay() const;
};

#endif