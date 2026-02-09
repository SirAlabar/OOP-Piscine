#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"

void Graph::addNode(Node* node)
{
	if (!node)
	{
		return;
	}

	if (hasNode(node->getName()))
	{
		return;
	}

	_adjacency[node] = {};
	_nodes.push_back(std::unique_ptr<Node>(node));
}

Node* Graph::getNode(const std::string& name)
{
	for (auto& node : _nodes)
	{
		if (node && node->getName() == name)
		{
			return node.get();
		}
	}
	return nullptr;
}

const Node* Graph::getNode(const std::string& name) const
{
	for (const auto& node : _nodes)
	{
		if (node && node->getName() == name)
		{
			return node.get();
		}
	}
	return nullptr;
}

Graph::NodeList Graph::getNodes() const
{
	NodeList result;
	result.reserve(_nodes.size());
	for (const auto& node : _nodes)
	{
		result.push_back(node.get());
	}
	return result;
}

bool Graph::hasNode(const std::string& name) const
{
	return getNode(name) != nullptr;
}

size_t Graph::getNodeCount() const
{
	return _nodes.size();
}

bool Graph::nodeExistsInGraph(Node* node) const
{
	for (const auto& n : _nodes)
	{
		if (n.get() == node)
		{
			return true;
		}
	}
	return false;
}

void Graph::addRail(Rail* rail)
{
	if (!rail || !rail->isValid())
	{
		return;
	}

	Node* nodeA = rail->getNodeA();
	Node* nodeB = rail->getNodeB();

	if (!nodeExistsInGraph(nodeA) || !nodeExistsInGraph(nodeB))
	{
		return;
	}

	_adjacency[nodeA].push_back(rail);
	_adjacency[nodeB].push_back(rail);
	_rails.push_back(std::unique_ptr<Rail>(rail));
}

Graph::RailList Graph::getRails() const
{
	RailList result;
	result.reserve(_rails.size());
	for (const auto& rail : _rails)
	{
		result.push_back(rail.get());
	}
	return result;
}

size_t Graph::getRailCount() const
{
	return _rails.size();
}

Graph::RailList Graph::getRailsFromNode(Node* node) const
{
	if (auto it = _adjacency.find(node); it != _adjacency.end())
	{
		return it->second;
	}

	return {};
}

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

bool Graph::isValid() const
{
	for (const auto& node : _nodes)
	{
		if (!node || !node->isValid())
		{
			return false;
		}
	}

	for (const auto& rail : _rails)
	{
		if (!rail || !rail->isValid())
		{
			return false;
		}

		if (!nodeExistsInGraph(rail->getNodeA()) ||
		    !nodeExistsInGraph(rail->getNodeB()))
		{
			return false;
		}
	}

	return true;
}

void Graph::clear()
{
	_nodes.clear();
	_rails.clear();
	_adjacency.clear();
}