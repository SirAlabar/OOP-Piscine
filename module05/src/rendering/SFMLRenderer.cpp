#include "rendering/SFMLRenderer.hpp"
#include "simulation/SimulationManager.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include <stdexcept>
#include <queue>
#include <algorithm>

SFMLRenderer::SFMLRenderer()
	: _window(sf::VideoMode(1400, 900), "Railway Simulation"),
	  _world(nullptr),
	  _worldGenerator(nullptr),
	  _networkCenterX(0),
	  _networkCenterY(0),
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
	
	// Calculate network bounding box
	int minX = 0, maxX = 0, minY = 0, maxY = 0;
	bool first = true;
	
	for (const auto& pair : _nodeGridPositions)
	{
		int x = pair.second.x;
		int y = pair.second.y;
		
		if (first)
		{
			minX = maxX = x;
			minY = maxY = y;
			first = false;
		}
		else
		{
			if (x < minX) minX = x;
			if (x > maxX) maxX = x;
			if (y < minY) minY = y;
			if (y > maxY) maxY = y;
		}
	}
	
	// Calculate network center in grid coordinates
	_networkCenterX = (minX + maxX) / 2;
	_networkCenterY = (minY + maxY) / 2;
	
	// Calculate required world size with padding
	const int padding = 10;
	int gridWidth = (maxX - minX) + 1;
	int gridHeight = (maxY - minY) + 1;
	
	// Add padding and ensure minimum size
	int worldSize = std::max(gridWidth + padding * 2, gridHeight + padding * 2);
	worldSize = std::max(worldSize, 56);  // Minimum 56x56

	_world = new World(worldSize, worldSize);
	
	unsigned int seed = simulation.getSeed();
	
	_worldGenerator = new WorldGenerator(seed, worldSize, worldSize);
	_renderManager.setWorldSeed(seed);
	
	markRailsInWorld(simulation.getNetwork());
	generateWorld(simulation.getNetwork());
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
	
	// PHASE 1: Separate cities and junctions
	std::vector<const Node*> cities;
	std::vector<const Node*> junctions;
	
	for (const Node* node : nodes)
	{
		if (node->getType() == NodeType::CITY)
		{
			cities.push_back(node);
		}
		else
		{
			junctions.push_back(node);
		}
	}

	// PHASE 2: Place cities in a circular/grid pattern
	if (cities.empty())
	{
		return;
	}
	
	// Place first city at origin
	_nodeGridPositions[cities[0]] = sf::Vector2i(0, 0);
	_occupiedTiles.insert(std::make_pair(0, 0));
	
	// Place remaining cities using BFS
	std::queue<const Node*> cityQueue;
	std::set<const Node*> visitedCities;
	cityQueue.push(cities[0]);
	visitedCities.insert(cities[0]);
	
	const std::vector<sf::Vector2i> cityDirections = {
		sf::Vector2i(8, 0), sf::Vector2i(-8, 0), sf::Vector2i(0, 8), sf::Vector2i(0, -8),
		sf::Vector2i(6, 6), sf::Vector2i(-6, -6), sf::Vector2i(6, -6), sf::Vector2i(-6, 6)
	};
	
	while (!cityQueue.empty())
	{
		const Node* current = cityQueue.front();
		cityQueue.pop();
		const sf::Vector2i basePos = _nodeGridPositions[current];
		
		// Find connected cities
		const Graph::RailList rails = graph->getRailsFromNode(const_cast<Node*>(current));
		size_t dirIndex = 0;
		
		for (Rail* rail : rails)
		{
			if (!rail) continue;
			
			Node* otherMutable = rail->getOtherNode(const_cast<Node*>(current));
			const Node* other = otherMutable;
			
			if (!other || other->getType() != NodeType::CITY) continue;
			if (visitedCities.count(other) != 0) continue;
			
			// Find free position for this city
			sf::Vector2i newPos = basePos + cityDirections[dirIndex % cityDirections.size()];
			int attempts = 0;
			while (_occupiedTiles.count(std::make_pair(newPos.x, newPos.y)) != 0 && attempts < 20)
			{
				dirIndex++;
				newPos = basePos + cityDirections[dirIndex % cityDirections.size()];
				newPos.x += (attempts / 4) * 2;
				attempts++;
			}
			
			_nodeGridPositions[other] = newPos;
			_occupiedTiles.insert(std::make_pair(newPos.x, newPos.y));
			visitedCities.insert(other);
			cityQueue.push(other);
			dirIndex++;
		}
	}
	
	// Place any unconnected cities
	for (const Node* city : cities)
	{
		if (_nodeGridPositions.count(city) == 0)
		{
			int fallbackX = static_cast<int>(_nodeGridPositions.size()) * 4;
			int fallbackY = 0;
			_nodeGridPositions[city] = sf::Vector2i(fallbackX, fallbackY);
			_occupiedTiles.insert(std::make_pair(fallbackX, fallbackY));
		}
	}
	
	// PHASE 3: Place junctions between their IMMEDIATE neighbors
	// Multiple passes to resolve junction positions iteratively
	for (int pass = 0; pass < 5; ++pass)
	{
		for (const Node* junction : junctions)
		{
			if (pass == 0 || _nodeGridPositions.count(junction) == 0)
			{
				// Get IMMEDIATE neighbors only
				std::vector<const Node*> neighbors;
				const Graph::RailList rails = graph->getRailsFromNode(const_cast<Node*>(junction));
				
				for (Rail* rail : rails)
				{
					if (!rail) continue;
					
					Node* neighborMutable = rail->getOtherNode(const_cast<Node*>(junction));
					const Node* neighbor = neighborMutable;
					
					if (neighbor)
					{
						neighbors.push_back(neighbor);
					}
				}
				
				if (neighbors.empty())
				{
					_nodeGridPositions[junction] = sf::Vector2i(0, 0);
					continue;
				}
				
				// Calculate position as average of positioned neighbors
				sf::Vector2f avgPos(0.0f, 0.0f);
				int count = 0;
				
				for (const Node* neighbor : neighbors)
				{
					if (_nodeGridPositions.count(neighbor) != 0)
					{
						sf::Vector2i neighborPos = _nodeGridPositions[neighbor];
						avgPos.x += static_cast<float>(neighborPos.x);
						avgPos.y += static_cast<float>(neighborPos.y);
						count++;
					}
				}
				
				if (count > 0)
				{
					avgPos.x /= static_cast<float>(count);
					avgPos.y /= static_cast<float>(count);
					
					sf::Vector2i junctionPos(
						static_cast<int>(std::round(avgPos.x)),
						static_cast<int>(std::round(avgPos.y))
					);
					
					// Handle collisions with small offset
					int offset = 0;
					while (_occupiedTiles.count(std::make_pair(junctionPos.x, junctionPos.y)) != 0 && offset < 20)
					{
						offset++;
						// Offset in a spiral pattern
						int ring = (offset - 1) / 8 + 1;
						int pos = (offset - 1) % 8;
						
						if (pos == 0) junctionPos.x += ring;
						else if (pos == 1) junctionPos.y += ring;
						else if (pos == 2) junctionPos.x -= ring;
						else if (pos == 3) junctionPos.y -= ring;
						else if (pos == 4) { junctionPos.x += ring; junctionPos.y += ring; }
						else if (pos == 5) { junctionPos.x -= ring; junctionPos.y += ring; }
						else if (pos == 6) { junctionPos.x += ring; junctionPos.y -= ring; }
						else if (pos == 7) { junctionPos.x -= ring; junctionPos.y -= ring; }
					}
					
					_nodeGridPositions[junction] = junctionPos;
					_occupiedTiles.insert(std::make_pair(junctionPos.x, junctionPos.y));
				}
			}
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
	
	unsigned int seed = _renderManager.getWorldSeed();
	_world->cacheTileSprites(seed);
	
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
	const int worldCenter = worldSize / 2;
	
	const int offsetX = worldCenter - _networkCenterX;
	const int offsetY = worldCenter - _networkCenterY;
	
	return sf::Vector2i(gx + offsetX, gy + offsetY);
}