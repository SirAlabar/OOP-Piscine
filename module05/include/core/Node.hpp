#ifndef NODE_HPP
#define NODE_HPP

#include <string>

// CITY = station where trains stop, JUNCTION = rail connection point
enum NodeType
{
	CITY,
	JUNCTION
};

// Represents a point in the railway network graph
class Node
{
private:
	std::string _name;
	NodeType    _type;

public:
	Node();
	Node(const std::string& name, NodeType type = CITY);
	Node(const Node& other);
	Node& operator=(const Node& other);
	~Node();

	std::string getName() const;
	NodeType    getType() const;
	bool        isValid() const;
	std::string getTypeString() const;
};

#endif