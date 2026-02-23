#include "core/Node.hpp"
#include "events/Event.hpp"

// Default constructor
Node::Node() : _name(""), _type(NodeType::CITY)
{
}

// Parameterized constructor
Node::Node(const std::string& name, NodeType type)
	: _name(name), _type(type)
{
}

// Getters
const std::string& Node::getName() const
{
	return _name;
}

NodeType Node::getType() const
{
	return _type;
}

// Validation - name must not be empty
bool Node::isValid() const
{
	return !_name.empty();
}

// Convert type to string for logging/output
std::string Node::getTypeString() const
{
	switch (_type)
	{
		case NodeType::CITY:
			return "City";
		case NodeType::JUNCTION:
			return "Junction";
		default:
			return "Unknown";
	}
}

// IObserver implementation - handle event notifications
void Node::onNotify(Event* event)
{
	if (!event)
	{
		return;
	}

	// Nodes react to station delays and signal failures
	// Event effects are handled by trains when they reach the node
	// This method allows nodes to be aware of events affecting them
}