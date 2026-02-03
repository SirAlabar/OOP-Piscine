#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"

// Add node to graph (checks for duplicates)
void Graph::addNode(Node* node)
{
	if (!node)
    {
		return;
    }

	// Check if node with same name already exists
	if (hasNode(node->getName()))
    {
		return;
    }

	_nodes.push_back(node);
	_adjacency[node] = {};
}

// Get node by name (returns nullptr if not found)
Node* Graph::getNode(const std::string& name)
{
    for (auto* node : _nodes)
    {
        if (node && node->getName() == name)
        {
            return node;
        }
    }
    return nullptr;
}

const Node* Graph::getNode(const std::string& name) const
{
	for (const auto* node : _nodes)
	{
		if (node && node->getName() == name)
        {
			return node;
        }
	}
	return nullptr;
}

// Get all nodes
const Graph::NodeList& Graph::getNodes() const
{
	return _nodes;
}

// Check if node exists by name
bool Graph::hasNode(const std::string& name) const
{
	return getNode(name) != nullptr;
}

// Get node count
size_t Graph::getNodeCount() const
{
	return _nodes.size();
}

// Helper: check if node exists in graph
bool Graph::nodeExistsInGraph(Node* node) const
{
	for (const auto* n : _nodes)
	{
		if (n == node)
        {
			return true;
        }
	}
	return false;
}

// Add rail to graph (builds bidirectional adjacency)
void Graph::addRail(Rail* rail)
{
	if (!rail || !rail->isValid())
    {
		return;
    }

	Node* nodeA = rail->getNodeA();
	Node* nodeB = rail->getNodeB();

	// Both nodes must exist in graph
	if (!nodeExistsInGraph(nodeA) || !nodeExistsInGraph(nodeB))
    {
		return;
    }

	_rails.push_back(rail);

	// Build bidirectional adjacency (rail is bidirectional)
	_adjacency[nodeA].push_back(rail);
	_adjacency[nodeB].push_back(rail);
}

// Get all rails
const Graph::RailList& Graph::getRails() const
{
	return _rails;
}

// Get rail count
size_t Graph::getRailCount() const
{
	return _rails.size();
}

// Get all rails connected to a node
Graph::RailList Graph::getRailsFromNode(Node* node) const
{
	if (auto it = _adjacency.find(node); it != _adjacency.end())
    {
		return it->second;
    }

	return {};
}

// Get all neighbor nodes of a given node
std::vector<Node*> Graph::getNeighbors(Node* node) const
{
	std::vector<Node*> neighbors;
	const auto rails = getRailsFromNode(node);

	for (const auto* rail : rails)
	{
		if (auto* neighbor = rail->getOtherNode(node))
        {
			neighbors.push_back(neighbor);
        }
	}

	return neighbors;
}

// Validation - all nodes valid, all rails valid and connect existing nodes
bool Graph::isValid() const
{
	// Check all nodes are valid
	for (const auto* node : _nodes)
	{
		if (!node || !node->isValid())
        {
			return false;
        }
	}

	// Check all rails are valid
	for (const auto* rail : _rails)
	{
		if (!rail || !rail->isValid())
        {
			return false;
        }

		// Verify rail nodes exist in graph
		if (!nodeExistsInGraph(rail->getNodeA()) ||
		    !nodeExistsInGraph(rail->getNodeB()))
        {
			return false;
        }
	}

	return true;
}

// Clear all nodes and rails (doesn't delete them)
void Graph::clear()
{
	_nodes.clear();
	_rails.clear();
	_adjacency.clear();
}
