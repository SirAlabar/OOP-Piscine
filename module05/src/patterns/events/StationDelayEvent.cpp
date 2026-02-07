#include "patterns/events/StationDelayEvent.hpp"
#include "core/Node.hpp"

StationDelayEvent::StationDelayEvent(Node* station, const Time& startTime,
                                     const Time& duration, const Time& additionalDelay)
	: Event(EventType::STATION_DELAY, startTime, duration),
	  _station(station),
	  _additionalDelay(additionalDelay)
{
	// Setup visual data for isometric rendering
	_visualData.centerNode = station;
	_visualData.radius = 0.0;  // Point event
	_visualData.iconType = "delay";
	_visualData.animationSpeed = 0.5f;
}

void StationDelayEvent::activate()
{
	// Event becomes active - observers will check this
}

void StationDelayEvent::deactivate()
{
	// Event ends - observers will check this
}

bool StationDelayEvent::affectsNode(Node* node) const
{
	return node == _station;
}

bool StationDelayEvent::affectsRail(Rail* rail) const
{
	(void)rail;  // Unused - station delays don't affect rails
	return false;
}

bool StationDelayEvent::affectsTrain(Train* train) const
{
	(void)train;  // Unused - affects all trains at this station
	return true;
}

std::string StationDelayEvent::getDescription() const
{
	return "Station delay at " + _station->getName() + 
	       " (+" + _additionalDelay.toString() + " additional stop time)";
}

Node* StationDelayEvent::getStation() const
{
	return _station;
}

Time StationDelayEvent::getAdditionalDelay() const
{
	return _additionalDelay;
}