#ifndef WEATHEREVENT_HPP
#define WEATHEREVENT_HPP

#include "Event.hpp"
#include "utils/Time.hpp"
#include <vector>
#include <string>

// Event: Weather affects multiple rails within radius
class WeatherEvent : public Event
{
private:
	std::string        _weatherType;         // "Heavy Rain", "Storm", "Snow", etc.
	Node*              _centerNode;          // Epicenter
	double             _radiusKm;            // Affected area
	double             _speedReductionFactor; // 0.0-1.0
	double             _frictionIncrease;    // Added to train friction
	std::vector<Rail*> _affectedRails;       // Rails within radius
	std::vector<double> _originalSpeedLimits; // For restoration

public:
	WeatherEvent(const std::string& weatherType, Node* centerNode,
	             const Time& startTime, const Time& duration,
	             double radiusKm, double speedReductionFactor, double frictionIncrease);

	void activate() override;
	void deactivate() override;

	bool affectsNode(Node* node) const override;
	bool affectsRail(Rail* rail) const override;
	bool affectsTrain(Train* train) const override;
	bool isApplicableToTrain(Train* train) const override;
	
	std::string getDescription() const override;

	// Getters
	std::string getWeatherType() const;
	Node* getCenterNode() const;
	double getRadiusKm() const;
	double getSpeedReductionFactor() const;
	double getFrictionIncrease() const;

	// Set affected rails (called by EventFactory after creation)
	void setAffectedRails(const std::vector<Rail*>& rails);
};

#endif