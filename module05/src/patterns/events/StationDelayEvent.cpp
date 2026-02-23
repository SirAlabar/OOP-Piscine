#include "patterns/events/StationDelayEvent.hpp"
#include "core/Node.hpp"
#include "core/Train.hpp"

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

bool StationDelayEvent::isApplicableToTrain(Train* train) const
{
	if (!train || !_station)
	{
		return false;
	}
	
	// All trains approaching/at this station get notified
	
	// Check if this station is in the train's path
	Node* currentNode = train->getCurrentNode();
	Node* nextNode = train->getNextNode();
	
	// Train is either at the station or will arrive at it
	bool stationInPath = (currentNode == _station || nextNode == _station);
	if (!stationInPath)
	{
		return false;
	}
	
	// Check if train has a next segment (if no next rail, it's at final destination)
	const auto& path = train->getPath();
	size_t currentIndex = train->getCurrentRailIndex();
	
	// If on last segment or beyond, this is the final destination
	if (currentIndex >= path.size())
	{
		return false;  // At or past final destination
	}
	
	// Apply delay - train will stop at this station during journey
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


const Node* StationDelayEvent::getAnchorNode() const
{
	return _station;
}

const Rail* StationDelayEvent::getAnchorRail() const
{
	return nullptr;
}