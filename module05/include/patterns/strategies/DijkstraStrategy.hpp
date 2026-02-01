#ifndef DIJKSTRASTRATEGY_HPP
#define DIJKSTRASTRATEGY_HPP

#include "patterns/strategies/IPathfindingStrategy.hpp"

class DijkstraStrategy : public IPathfindingStrategy
{
public:
	DijkstraStrategy() = default;
	~DijkstraStrategy() override = default;
	
	Path findPath(const Graph* graph, Node* start, Node* end) const override;
	std::string getName() const override;
};

#endif