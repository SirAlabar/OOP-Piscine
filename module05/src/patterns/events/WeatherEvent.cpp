#include "patterns/events/WeatherEvent.hpp"
#include "core/Rail.hpp"
#include "core/Node.hpp"

WeatherEvent::WeatherEvent(const std::string& weatherType, Node* centerNode,
                           const Time& startTime, const Time& duration,
                           double radiusKm, double speedReductionFactor, double frictionIncrease)
	: Event(EventType::WEATHER, startTime, duration),
	  _weatherType(weatherType),
	  _centerNode(centerNode),
	  _radiusKm(radiusKm),
	  _speedReductionFactor(speedReductionFactor),
	  _frictionIncrease(frictionIncrease)
{
	// Setup visual data for isometric rendering
	_visualData.centerNode = centerNode;
	_visualData.radius = radiusKm;
	_visualData.iconType = "storm";
	_visualData.animationSpeed = 2.0f;  // Fast particle movement
}

void WeatherEvent::activate()
{
	// Store original speed limits and reduce them
	_originalSpeedLimits.clear();
	for (Rail* rail : _affectedRails)
	{
		if (rail)
		{
			_originalSpeedLimits.push_back(rail->getSpeedLimit());
			double reducedSpeed = rail->getSpeedLimit() * _speedReductionFactor;
			rail->setSpeedLimit(reducedSpeed);
		}
	}
}

void WeatherEvent::deactivate()
{
	// Restore original speed limits
	for (size_t i = 0; i < _affectedRails.size() && i < _originalSpeedLimits.size(); ++i)
	{
		if (_affectedRails[i])
		{
			_affectedRails[i]->setSpeedLimit(_originalSpeedLimits[i]);
		}
	}
	_originalSpeedLimits.clear();
}

bool WeatherEvent::affectsNode(Node* node) const
{
	(void)node;  // Unused - weather affects rails, not nodes directly
	return false;
}

bool WeatherEvent::affectsRail(Rail* rail) const
{
	for (Rail* r : _affectedRails)
	{
		if (r == rail)
		{
			return true;
		}
	}
	return false;
}

bool WeatherEvent::affectsTrain(Train* train) const
{
	(void)train;  // Unused - affects trains via rail speed limits and friction
	return true;
}

std::string WeatherEvent::getDescription() const
{
	int radiusInt = static_cast<int>(_radiusKm);
	return _weatherType + " near " + _centerNode->getName() + 
	       " (radius " + std::to_string(radiusInt) + "km, reduced speed and increased friction)";
}

std::string WeatherEvent::getWeatherType() const
{
	return _weatherType;
}

Node* WeatherEvent::getCenterNode() const
{
	return _centerNode;
}

double WeatherEvent::getRadiusKm() const
{
	return _radiusKm;
}

double WeatherEvent::getSpeedReductionFactor() const
{
	return _speedReductionFactor;
}

double WeatherEvent::getFrictionIncrease() const
{
	return _frictionIncrease;
}

void WeatherEvent::setAffectedRails(const std::vector<Rail*>& rails)
{
	_affectedRails = rails;
}