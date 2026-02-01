#include "patterns/strategies/DijkstraStrategy.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include <map>
#include <queue>
#include <vector>
#include <limits>
#include <algorithm>

IPathfindingStrategy::Path DijkstraStrategy::findPath(const Graph* graph, Node* start, Node* end) const
{
	if (!graph || !start || !end)
	{
		return {};
	}
	
	if (start == end)
	{
		return {};
	}
	
	// Distance map: Node -> min cost
	std::map<Node*, double> distance;
	// Previous: Node -> Rail used to reach it
	std::map<Node*, Rail*> previous;
	// Priority queue: (cost, node)
	using QueueItem = std::pair<double, Node*>;
	std::priority_queue<QueueItem, std::vector<QueueItem>, std::greater<QueueItem>> pq;
	
	// Initialize distances
	const auto& nodes = graph->getNodes();
	for (auto* node : nodes)
	{
		distance[node] = std::numeric_limits<double>::infinity();
	}
	distance[start] = 0.0;
	
	pq.push({0.0, start});
	
	while (!pq.empty())
	{
		double currentDist = pq.top().first;
		Node* current = pq.top().second;
		pq.pop();
		
		// Found destination
		if (current == end)
		{
			break;
		}
		
		// Skip if already processed with better distance
		if (currentDist > distance[current])
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
			double cost = rail->getLength() / rail->getSpeedLimit();
			double newDist = currentDist + cost;
			
			if (newDist < distance[neighbor])
			{
				distance[neighbor] = newDist;
				previous[neighbor] = rail;
				pq.push({newDist, neighbor});
			}
		}
	}
	
	// No path found
	if (previous.find(end) == previous.end())
	{
		return {};
	}
	
	// Reconstruct path
	Path path;
	Node* current = end;
	
	while (current != start)
	{
		Rail* rail = previous[current];
		path.push_back(rail);
		current = rail->getOtherNode(current);
	}
	
	// Reverse to get start->end order
	std::reverse(path.begin(), path.end());
	
	return path;
}

std::string DijkstraStrategy::getName() const
{
	return "Dijkstra";
}