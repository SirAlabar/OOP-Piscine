#ifndef IPATHFINDINGSTRATEGY_HPP
#define IPATHFINDINGSTRATEGY_HPP

#include <vector>
#include <string>

class Graph;
class Node;
class Rail;

class IPathfindingStrategy
{
public:
	using Path = std::vector<Rail*>;
	
	virtual ~IPathfindingStrategy() = default;
	
	virtual Path findPath(const Graph* graph, Node* start, Node* end) const = 0;
	virtual std::string getName() const = 0;
};

#endif