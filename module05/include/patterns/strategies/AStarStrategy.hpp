#ifndef ASTARSTRATEGY_HPP
#define ASTARSTRATEGY_HPP

#include "patterns/strategies/IPathfindingStrategy.hpp"

class Graph;
class Node;

class AStarStrategy : public IPathfindingStrategy
{
public:
	AStarStrategy() = default;
	~AStarStrategy() override = default;
	
	Path findPath(const Graph* graph, Node* start, Node* end) const override;
	std::string getName() const override;

private:
	// Heuristic function: estimates cost from node to goal
	double heuristic(Node* current, Node* goal) const;
};

#endif