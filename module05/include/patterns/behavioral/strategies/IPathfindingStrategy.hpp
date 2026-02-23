#ifndef IPATHFINDINGSTRATEGY_HPP
#define IPATHFINDINGSTRATEGY_HPP

#include <vector>
#include <string>
#include "core/Train.hpp"

class Graph;
class Node;

class IPathfindingStrategy
{
public:
	using Path = std::vector<PathSegment>;
	
	virtual ~IPathfindingStrategy() = default;
	
	virtual Path findPath(const Graph* graph, Node* start, Node* end) const = 0;
	virtual std::string getName() const = 0;
};

#endif