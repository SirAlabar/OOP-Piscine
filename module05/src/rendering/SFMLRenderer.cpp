#include "rendering/SFMLRenderer.hpp"
#include "simulation/SimulationManager.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include <stdexcept>
#include <queue>
#include <algorithm>
#include <iostream>

SFMLRenderer::SFMLRenderer()
	: _window(sf::VideoMode(1400, 900), "Railway Simulation"),
	  _world(nullptr),
	  _worldGenerator(nullptr),
	  _timeAccumulator(0.0),
	  _simulationStepInterval(0.10)
{
	_window.setFramerateLimit(60);
	
	if (!_atlas.loadFromFiles("assets/assets.png", "assets/assets.json"))
	{
		throw std::runtime_error("Failed to load assets atlas");
	}
	
	_cameraManager.setZoomLimits(0.5f, 3.0f);
}

SFMLRenderer::~SFMLRenderer()
{
	delete _world;
	delete _worldGenerator;
}

void SFMLRenderer::run(SimulationManager& simulation)
{
	initializeWorld(simulation);
	simulation.start();
	mainLoop(simulation);
	simulation.stop();
}

void SFMLRenderer::initializeWorld(SimulationManager& simulation)
{
	buildGraphLayout(simulation.getNetwork());
	
	const int worldSize = 56;
	_world = new World(worldSize, worldSize);
	
	unsigned int seed = simulation.getSeed();
	std::cout << "[SFMLRenderer] Got seed from simulation: " << seed << std::endl;
	
	_worldGenerator = new WorldGenerator(seed, worldSize, worldSize);
	_renderManager.setWorldSeed(seed);
	
	std::cout << "[SFMLRenderer] Starting world generation..." << std::endl;
	markRailsInWorld(simulation.getNetwork());
	generateWorld(simulation.getNetwork());
	std::cout << "[SFMLRenderer] World initialization complete!" << std::endl;
}

void SFMLRenderer::buildGraphLayout(const Graph* graph)
{
	_nodeGridPositions.clear();
	_occupiedTiles.clear();
	
	if (!graph || graph->getNodes().empty())
	{
		return;
	}
	
	const Graph::NodeList nodes = graph->getNodes();
	
	const Node* root = nodes[0];
	size_t rootDegree = graph->getRailsFromNode(const_cast<Node*>(nodes[0])).size();
	for (size_t i = 1; i < nodes.size(); ++i)
	{
		const size_t degree = graph->getRailsFromNode(const_cast<Node*>(nodes[i])).size();
		if (degree > rootDegree)
		{
			root = nodes[i];
			rootDegree = degree;
		}
	}
	
	const std::vector<sf::Vector2i> dirs = {
		sf::Vector2i(4, 0), sf::Vector2i(-4, 0), sf::Vector2i(0, 4), sf::Vector2i(0, -4),
		sf::Vector2i(4, 4), sf::Vector2i(-4, -4), sf::Vector2i(4, -4), sf::Vector2i(-4, 4)
	};
	
	std::queue<const Node*> q;
	std::set<const Node*> visited;
	_nodeGridPositions[root] = sf::Vector2i(0, 0);
	visited.insert(root);
	q.push(root);
	
	while (!q.empty())
	{
		const Node* current = q.front();
		q.pop();
		const sf::Vector2i base = _nodeGridPositions[current];
		
		const Graph::RailList rails = graph->getRailsFromNode(const_cast<Node*>(current));
		size_t pick = 0;
		for (Rail* rail : rails)
		{
			if (!rail)
			{
				continue;
			}
			
			Node* otherMutable = rail->getOtherNode(const_cast<Node*>(current));
			const Node* other = otherMutable;
			if (!other)
			{
				continue;
			}
			
			if (visited.count(other) == 0)
			{
				sf::Vector2i candidate = base + dirs[pick % dirs.size()];
				while (_occupiedTiles.count(std::make_pair(candidate.x, candidate.y)) != 0)
				{
					pick++;
					candidate = base + dirs[pick % dirs.size()] + sf::Vector2i(static_cast<int>(pick / dirs.size()) * 2, 0);
				}
				_nodeGridPositions[other] = candidate;
				_occupiedTiles.insert(std::make_pair(candidate.x, candidate.y));
				visited.insert(other);
				q.push(other);
				pick++;
			}
		}
	}
	
	for (const Node* node : nodes)
	{
		if (_nodeGridPositions.count(node) == 0)
		{
			const int fallback = static_cast<int>(_nodeGridPositions.size()) * 3;
			_nodeGridPositions[node] = sf::Vector2i(fallback, fallback % 7);
			_occupiedTiles.insert(std::make_pair(fallback, fallback % 7));
		}
	}
}

void SFMLRenderer::markRailsInWorld(const Graph* graph)
{
	if (!graph || !_world)
	{
		return;
	}
	
	std::map<const Node*, sf::Vector2f> worldPositions;
	for (const auto& pair : _nodeGridPositions)
	{
		sf::Vector2i gridPos = gridToWorldOffset(pair.second.x, pair.second.y);
		worldPositions[pair.first] = sf::Vector2f(static_cast<float>(gridPos.x), static_cast<float>(gridPos.y));
		
		if (_world->isInBounds(gridPos.x, gridPos.y))
		{
			_world->markRailOccupied(gridPos.x, gridPos.y);
		}
	}
	
	_renderManager.setNodePositions(worldPositions);
	_renderManager.setNodeGridPositions(_nodeGridPositions);
	
	for (const Rail* rail : graph->getRails())
	{
		if (!rail)
		{
			continue;
		}
		
		const Node* a = rail->getNodeA();
		const Node* b = rail->getNodeB();
		
		if (_nodeGridPositions.count(a) == 0 || _nodeGridPositions.count(b) == 0)
		{
			continue;
		}
		
		sf::Vector2i from = gridToWorldOffset(_nodeGridPositions[a].x, _nodeGridPositions[a].y);
		sf::Vector2i to = gridToWorldOffset(_nodeGridPositions[b].x, _nodeGridPositions[b].y);
		
		sf::Vector2i p = from;
		const int sx = (to.x > p.x) ? 1 : -1;
		const int sy = (to.y > p.y) ? 1 : -1;
		
		while (p.x != to.x)
		{
			if (_world->isInBounds(p.x, p.y))
			{
				_world->markRailOccupied(p.x, p.y);
			}
			p.x += sx;
		}
		
		while (p.y != to.y)
		{
			if (_world->isInBounds(p.x, p.y))
			{
				_world->markRailOccupied(p.x, p.y);
			}
			p.y += sy;
		}
		
		if (_world->isInBounds(to.x, to.y))
		{
			_world->markRailOccupied(to.x, to.y);
		}
	}
}

void SFMLRenderer::generateWorld(const Graph* graph)
{
	if (!_world || !_worldGenerator)
	{
		return;
	}
	
	_worldGenerator->generate(*_world);
	_renderManager.buildRailBitmasks(*_world);
	_renderManager.buildStationTiles(graph, *_world);
}

void SFMLRenderer::mainLoop(SimulationManager& simulation)
{
	sf::Clock clock;
	
	while (_window.isOpen())
	{
		double realDt = clock.restart().asSeconds();
		
		processInput(simulation, realDt);
		updateSimulation(simulation, realDt);
		render(simulation);
	}
}

void SFMLRenderer::processInput(SimulationManager& simulation, double deltaTime)
{
	InputState input = _inputManager.processEvents(_window, deltaTime);
	
	if (input.closeRequested)
	{
		simulation.stop();
		_window.close();
		return;
	}
	
	if (input.zoomDelta != 0.0f)
	{
		_cameraManager.addZoomDelta(input.zoomDelta);
	}
	
	if (input.dragActive)
	{
		_cameraManager.moveOffset(sf::Vector2f(
			static_cast<float>(input.dragDelta.x),
			static_cast<float>(input.dragDelta.y)
		));
	}
	
	if (input.panX != 0.0f || input.panY != 0.0f)
	{
		_cameraManager.moveOffset(sf::Vector2f(input.panX, input.panY));
	}
}

void SFMLRenderer::updateSimulation(SimulationManager& simulation, double realDt)
{
	_cameraManager.update(realDt);
	
	_timeAccumulator += realDt;
	while (_timeAccumulator >= _simulationStepInterval)
	{
		simulation.step();
		_timeAccumulator -= _simulationStepInterval;
	}
}

void SFMLRenderer::render(const SimulationManager& simulation)
{
	if (!_world)
	{
		return;
	}
	
	_renderManager.render(_window, _atlas, simulation, _cameraManager, *_world);
}

sf::Vector2i SFMLRenderer::gridToWorldOffset(int gx, int gy) const
{
	const int worldSize = _world ? _world->getWidth() : 56;
	const int offset = worldSize / 2;
	return sf::Vector2i(gx + offset, gy + offset);
}