#include "core/Node.hpp"

// Default constructor
Node::Node() : _name(""), _type(CITY)
{
}

// Parameterized constructor
Node::Node(const std::string& name, NodeType type) : _name(name), _type(type)
{
}

// Copy constructor
Node::Node(const Node& other) : _name(other._name), _type(other._type)
{
}

// Assignment operator
Node& Node::operator=(const Node& other)
{
	if (this != &other)
	{
		_name = other._name;
		_type = other._type;
	}
	return *this;
}

// Destructor
Node::~Node()
{
}

// Getters
std::string Node::getName() const
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
		case CITY:
			return "City";
		case JUNCTION:
			return "Junction";
		default:
			return "Unknown";
	}
}
