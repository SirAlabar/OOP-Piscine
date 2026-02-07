#ifndef NODE_HPP
#define NODE_HPP

#include <string>
#include "patterns/observers/IObserver.hpp"

class Event;

// CITY = station where trains stop, JUNCTION = rail connection point
enum class NodeType
{
	CITY,
	JUNCTION
};

// Represents a point in the railway network graph
class Node : public IObserver
{
private:
	std::string _name;
	NodeType    _type;

public:
	Node();
	Node(const std::string& name, NodeType type = NodeType::CITY);
	Node(const Node&) = default;
	Node& operator=(const Node&) = default;
	~Node() = default;

	const std::string& getName() const;
	NodeType           getType() const;
	bool               isValid() const;
	std::string        getTypeString() const;

	// IObserver implementation
	void onNotify(Event* event) override;
};

#endif