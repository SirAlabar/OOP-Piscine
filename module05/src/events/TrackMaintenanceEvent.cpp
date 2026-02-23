#include "events/TrackMaintenanceEvent.hpp"
#include "core/Rail.hpp"
#include "core/Node.hpp"
#include "core/Train.hpp"

TrackMaintenanceEvent::TrackMaintenanceEvent(Rail* rail, const Time& startTime,
                                             const Time& duration, double speedReductionFactor)
	: Event(EventType::TRACK_MAINTENANCE, startTime, duration),
	  _rail(rail),
	  _speedReductionFactor(speedReductionFactor),
	  _originalSpeedLimit(0.0)
{
	if (_rail)
	{
		_originalSpeedLimit = _rail->getSpeedLimit();

		// Setup visual data for isometric rendering
		_visualData.centerNode = _rail->getNodeA();  // Use start node as anchor
		_visualData.radius = _rail->getLength();     // Show affected rail length
		_visualData.iconType = "maintenance";
		_visualData.animationSpeed = 0.3f;
	}
}

void TrackMaintenanceEvent::activate()
{
	if (_rail)
	{
		// Reduce rail speed limit
		double reducedSpeed = _originalSpeedLimit * _speedReductionFactor;
		_rail->setSpeedLimit(reducedSpeed);
	}
}

void TrackMaintenanceEvent::deactivate()
{
	if (_rail)
	{
		// Restore original speed limit
		_rail->setSpeedLimit(_originalSpeedLimit);
	}
}

bool TrackMaintenanceEvent::affectsNode(Node* node) const
{
	(void)node;  // Unused - maintenance affects rails, not nodes
	return false;
}

bool TrackMaintenanceEvent::affectsRail(Rail* rail) const
{
	return rail == _rail;
}

bool TrackMaintenanceEvent::affectsTrain(Train* train) const
{
	(void)train;  // Unused - affects trains indirectly via rail speed limit
	return true;
}

bool TrackMaintenanceEvent::isApplicableToTrain(Train* train) const
{
	if (!train || !_rail)
	{
		return false;
	}
	
	// Track maintenance affects trains currently on the affected rail
	Rail* currentRail = train->getCurrentRail();
	return currentRail == _rail;
}

std::string TrackMaintenanceEvent::getDescription() const
{
	if (!_rail)
	{
		return "Track maintenance on unknown rail";
	}
	
	Node* nodeA = _rail->getNodeA();
	Node* nodeB = _rail->getNodeB();
	int percentage = static_cast<int>(_speedReductionFactor * 100);
	
	return "Track maintenance on rail " + nodeA->getName() + "-" + nodeB->getName() +
	       " (speed reduced to " + std::to_string(percentage) + "%)";
}

Rail* TrackMaintenanceEvent::getRail() const
{
	return _rail;
}

double TrackMaintenanceEvent::getSpeedReductionFactor() const
{
	return _speedReductionFactor;
}


const Node* TrackMaintenanceEvent::getAnchorNode() const
{
	return nullptr;
}

const Rail* TrackMaintenanceEvent::getAnchorRail() const
{
	return _rail;
}