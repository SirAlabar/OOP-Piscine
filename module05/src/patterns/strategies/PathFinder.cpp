#include "patterns/strategies/PathFinder.hpp"

PathFinder::PathFinder()
	: _strategy(nullptr)
{
}

PathFinder::PathFinder(IPathfindingStrategy* strategy)
	: _strategy(strategy)
{
}

void PathFinder::setStrategy(IPathfindingStrategy* strategy)
{
	_strategy = strategy;
}

IPathfindingStrategy* PathFinder::getStrategy() const
{
	return _strategy;
}

PathFinder::Path PathFinder::findPath(const Graph* graph, Node* start, Node* end) const
{
	if (!_strategy)
	{
		return {};
	}
	
	return _strategy->findPath(graph, start, end);
}