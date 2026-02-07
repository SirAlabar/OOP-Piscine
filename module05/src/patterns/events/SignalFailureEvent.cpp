#include "patterns/events/SignalFailureEvent.hpp"
#include "core/Node.hpp"

SignalFailureEvent::SignalFailureEvent(Node* node, const Time& startTime,
                                       const Time& duration, const Time& stopDuration)
	: Event(EventType::SIGNAL_FAILURE, startTime, duration),
	  _node(node),
	  _stopDuration(stopDuration)
{
	// Setup visual data for isometric rendering
	_visualData.centerNode = node;
	_visualData.radius = 0.0;  // Point event
	_visualData.iconType = "signal";
	_visualData.animationSpeed = 1.5f;  // Blinking signal
}

void SignalFailureEvent::activate()
{
	// Event becomes active - trains will check this when reaching node
}

void SignalFailureEvent::deactivate()
{
	// Event ends
}

bool SignalFailureEvent::affectsNode(Node* node) const
{
	return node == _node;
}

bool SignalFailureEvent::affectsRail(Rail* rail) const
{
	(void)rail;  // Unused - signal failures affect nodes
	return false;
}

bool SignalFailureEvent::affectsTrain(Train* train) const
{
	(void)train;  // Unused - affects all trains at this node
	return true;
}

std::string SignalFailureEvent::getDescription() const
{
	return "Signal failure at " + _node->getName() + 
	       " (forced stop for " + _stopDuration.toString() + ")";
}

Node* SignalFailureEvent::getNode() const
{
	return _node;
}

Time SignalFailureEvent::getStopDuration() const
{
	return _stopDuration;
}