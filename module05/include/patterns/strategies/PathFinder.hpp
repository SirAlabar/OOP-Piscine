#ifndef PATHFINDER_HPP
#define PATHFINDER_HPP

#include "patterns/strategies/IPathfindingStrategy.hpp"

class PathFinder
{
public:
	using Path = IPathfindingStrategy::Path;
	
	PathFinder();
	explicit PathFinder(IPathfindingStrategy* strategy);
	~PathFinder() = default;
	
	void setStrategy(IPathfindingStrategy* strategy);
	IPathfindingStrategy* getStrategy() const;
	
	Path findPath(const Graph* graph, Node* start, Node* end) const;
	
private:
	IPathfindingStrategy* _strategy;
};

#endif