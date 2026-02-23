#ifndef TRACKMAINTENANCEEVENT_HPP
#define TRACKMAINTENANCEEVENT_HPP

#include "Event.hpp"
#include "utils/Time.hpp"

// Event: Rail segment has reduced speed limit during maintenance
class TrackMaintenanceEvent : public Event
{
private:
	Rail*  _rail;                 // Which rail is under maintenance
	double _speedReductionFactor; // 0.0-1.0 (e.g., 0.6 = 60% of original speed)
	double _originalSpeedLimit;   // Stored for restoration

public:
	TrackMaintenanceEvent(Rail* rail, const Time& startTime,
	                      const Time& duration, double speedReductionFactor);

	void activate() override;
	void deactivate() override;

	bool affectsNode(Node* node) const override;
	bool affectsRail(Rail* rail) const override;
	bool affectsTrain(Train* train) const override;
	bool isApplicableToTrain(Train* train) const override;
	
	std::string getDescription() const override;

	// Getters
	Rail*  getRail()                  const;
	double getSpeedReductionFactor()  const;

	const Node* getAnchorNode() const override;
	const Rail* getAnchorRail() const override;
};

#endif