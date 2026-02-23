#include "patterns/behavioral/strategies/AStarStrategy.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include "core/Train.hpp"
#include <map>
#include <queue>
#include <vector>
#include <limits>
#include <algorithm>

IPathfindingStrategy::Path AStarStrategy::findPath(const Graph* graph, Node* start, Node* end) const
{
	if (!graph || !start || !end)
	{
		return {};
	}
	
	if (start == end)
	{
		return {};
	}
	
	// g(n): actual cost from start to node
	std::map<Node*, double> gScore;
	// f(n): estimated total cost = g(n) + h(n)
	std::map<Node*, double> fScore;
	// Previous: Node -> Rail used to reach it
	std::map<Node*, Rail*> previous;
	// Priority queue: (f_score, node)
	using QueueItem = std::pair<double, Node*>;
	std::priority_queue<QueueItem, std::vector<QueueItem>, std::greater<QueueItem>> openSet;
	
	// Initialize scores
	const auto& nodes = graph->getNodes();
	for (auto* node : nodes)
	{
		gScore[node] = std::numeric_limits<double>::infinity();
		fScore[node] = std::numeric_limits<double>::infinity();
	}
	
	gScore[start] = 0.0;
	fScore[start] = heuristic(start, end);
	
	openSet.push({fScore[start], start});
	
	while (!openSet.empty())
	{
		double currentFScore = openSet.top().first;
		Node* current = openSet.top().second;
		openSet.pop();
		
		// Found destination
		if (current == end)
		{
			break;
		}
		
		// Skip if already processed with better f-score
		if (currentFScore > fScore[current])
		{
			continue;
		}
		
		// Check all neighbors
		auto rails = graph->getRailsFromNode(current);
		for (auto* rail : rails)
		{
			Node* neighbor = rail->getOtherNode(current);
			if (!neighbor)
			{
				continue;
			}
			
			// Calculate cost: travel time = distance / speed
			double edgeCost = rail->getLength() / rail->getSpeedLimit();
			double tentativeGScore = gScore[current] + edgeCost;
			
			// Found better path to neighbor
			if (tentativeGScore < gScore[neighbor])
			{
				gScore[neighbor] = tentativeGScore;
				fScore[neighbor] = tentativeGScore + heuristic(neighbor, end);
				previous[neighbor] = rail;
				openSet.push({fScore[neighbor], neighbor});
			}
		}
	}
	
	// No path found
	if (previous.find(end) == previous.end())
	{
		return {};
	}
	
	// Reconstruct path with direction information
	Path path;
	Node* current = end;
	
	while (current != start)
	{
		Rail* rail = previous[current];
		Node* previousNode = rail->getOtherNode(current);
		
		// Create PathSegment with explicit from->to direction
		PathSegment segment;
		segment.rail = rail;
		segment.from = previousNode;
		segment.to = current;
		
		path.push_back(segment);
		
		// Move to previous node
		current = previousNode;
	}
	
	// Reverse to get start->end order
	std::reverse(path.begin(), path.end());
	
	return path;
}

std::string AStarStrategy::getName() const
{
	return "A*";
}

double AStarStrategy::heuristic(Node* current, Node* goal) const
{
	// No node coordinates available - heuristic returns 0
	// This makes A* behave identically to Dijkstra
	// Add coordinates to Node for Euclidean distance heuristic
	
	(void)current;
	(void)goal;
	
	return 0.0;
}