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
	  _networkCenterX(0),
	  _networkCenterY(0)
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
	
	sf::Clock clock;
	while (_window.isOpen())
	{
		double realDt = clock.restart().asSeconds();
		
		InputState input = _inputManager.processEvents(_window, realDt);
		
		if (input.closeRequested)
		{
			_window.close();
			break;
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
		
		// Apply speed changes
		if (input.speedMultiplier != 1.0)
		{
			double currentSpeed = simulation.getSimulationSpeed();
			simulation.setSimulationSpeed(currentSpeed * input.speedMultiplier);
		}
		
		_cameraManager.update(realDt);
		
		// Just render - simulation runs in separate thread
		render(simulation);
	}
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

void SFMLRenderer::render(const SimulationManager& simulation)
{
	if (!_world)
	{
		return;
	}
	
	_renderManager.render(_window, _atlas, simulation, _cameraManager, *_world);
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
	
	std::cout << "\n=== PHASE 2: CITY PLACEMENT ===" << std::endl;
	std::cout << "Total cities: " << cities.size() << std::endl;
	
	// Place cities in a 2D grid pattern for better visualization
	// For 6 cities: 3x2 grid
	const int spacing = 16;  // Grid spacing between cities
	const int cols = 3;      // 3 columns
	
	for (size_t i = 0; i < cities.size(); ++i)
	{
		int row = static_cast<int>(i) / cols;
		int col = static_cast<int>(i) % cols;
		
		// Stagger odd rows for more interesting layout
		int offsetX = (row % 2 == 1) ? spacing / 2 : 0;
		
		sf::Vector2i pos(col * spacing + offsetX, row * spacing);
		
		_nodeGridPositions[cities[i]] = pos;
		_occupiedTiles.insert(std::make_pair(pos.x, pos.y));
		
		std::cout << "  " << cities[i]->getName() << " placed at (" << pos.x << ", " << pos.y << ")" << std::endl;
	}
	
	std::cout << "City placement complete!" << std::endl;
	
	// PHASE 3: Place junctions between their IMMEDIATE neighbors
	// Multiple passes to resolve junction positions iteratively
	std::cout << "\n=== PHASE 3: JUNCTION PLACEMENT ===" << std::endl;
	std::cout << "Total junctions to place: " << junctions.size() << std::endl;
	
	for (int pass = 0; pass < 5; ++pass)
	{
		std::cout << "\n--- Pass " << pass << " ---" << std::endl;
		for (const Node* junction : junctions)
		{
			std::cout << "Processing junction: " << junction->getName();
			
			if (pass == 0 || _nodeGridPositions.count(junction) == 0)
			{
				std::cout << " (needs placement)" << std::endl;
				
				// Get IMMEDIATE neighbors, separated by type
				std::vector<const Node*> cityNeighbors;
				std::vector<const Node*> junctionNeighbors;
				const Graph::RailList rails = graph->getRailsFromNode(const_cast<Node*>(junction));
				
				std::cout << "  Total rails: " << rails.size() << std::endl;
				
				for (Rail* rail : rails)
				{
					if (!rail) continue;
					
					Node* neighborMutable = rail->getOtherNode(const_cast<Node*>(junction));
					const Node* neighbor = neighborMutable;
					
					if (neighbor)
					{
						if (neighbor->getType() == NodeType::CITY)
						{
							cityNeighbors.push_back(neighbor);
						}
						else
						{
							junctionNeighbors.push_back(neighbor);
						}
					}
				}
				
				if (cityNeighbors.empty() && junctionNeighbors.empty())
				{
					std::cout << "  WARNING: No neighbors found!" << std::endl;
					_nodeGridPositions[junction] = sf::Vector2i(0, 0);
					continue;
				}
				
				std::cout << "  City neighbors: " << cityNeighbors.size() << " [";
				for (const Node* city : cityNeighbors)
				{
					std::cout << city->getName();
					if (_nodeGridPositions.count(city) == 0)
					{
						std::cout << "(NOT PLACED YET) ";
					}
					else
					{
						std::cout << " ";
					}
				}
				std::cout << "]" << std::endl;
				
				std::cout << "  Junction neighbors: " << junctionNeighbors.size() << " [";
				for (const Node* junc : junctionNeighbors)
				{
					std::cout << junc->getName() << " ";
				}
				std::cout << "]" << std::endl;
				
				// SMART PLACEMENT: Prioritize city connections
				sf::Vector2f avgPos(0.0f, 0.0f);
				int count = 0;
				
				if (!cityNeighbors.empty())
				{
					std::cout << "  STRATEGY: Using CITY positions only (ignoring " << junctionNeighbors.size() << " junctions)" << std::endl;
					
					// Use ONLY city neighbors for positioning (ignore junction neighbors)
					for (const Node* city : cityNeighbors)
					{
						if (_nodeGridPositions.count(city) != 0)
						{
							sf::Vector2i cityPos = _nodeGridPositions[city];
							std::cout << "    Adding city " << city->getName() << " at (" << cityPos.x << ", " << cityPos.y << ")" << std::endl;
							avgPos.x += static_cast<float>(cityPos.x);
							avgPos.y += static_cast<float>(cityPos.y);
							count++;
						}
					}
				}
				else
				{
					std::cout << "  STRATEGY: No cities available, using junction positions" << std::endl;
					
					// Fallback: use junction neighbors if no cities
					for (const Node* junc : junctionNeighbors)
					{
						if (_nodeGridPositions.count(junc) != 0)
						{
							sf::Vector2i juncPos = _nodeGridPositions[junc];
							std::cout << "    Adding junction " << junc->getName() << " at (" << juncPos.x << ", " << juncPos.y << ")" << std::endl;
							avgPos.x += static_cast<float>(juncPos.x);
							avgPos.y += static_cast<float>(juncPos.y);
							count++;
						}
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
					std::cout << "  -> Calculated position: (" << junctionPos.x << ", " << junctionPos.y << ")" << std::endl;
					
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
					
					if (offset > 0) { std::cout << "  -> Collision! Moved to: (" << junctionPos.x << ", " << junctionPos.y << ")" << std::endl; }
					_nodeGridPositions[junction] = junctionPos;
					_occupiedTiles.insert(std::make_pair(junctionPos.x, junctionPos.y));
				}
			}
			else
			{
				std::cout << " (already placed)" << std::endl;
			}
		}
	}
	
	std::cout << "\n=== JUNCTION PLACEMENT COMPLETE ===" << std::endl;
	std::cout << "Final positions:" << std::endl;
	for (const Node* junction : junctions)
	{
		if (_nodeGridPositions.count(junction) != 0)
		{
			sf::Vector2i pos = _nodeGridPositions[junction];
			std::cout << "  " << junction->getName() << ": (" << pos.x << ", " << pos.y << ")" << std::endl;
		}
		else
		{
			std::cout << "  " << junction->getName() << ": NOT PLACED!" << std::endl;
		}
	}
	std::cout << std::endl;
}

void SFMLRenderer::markRailsInWorld(const Graph* graph)
{
	if (!graph || !_world)
	{
		return;
	}
	
	std::cout << "\n=== MARKING RAILS IN WORLD ===" << std::endl;
	
	std::map<const Node*, sf::Vector2f> worldPositions;
	std::map<const Rail*, RailPath> railPaths;
	
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
	
	int railCount = 0;
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
		
		std::cout << "  Rail " << ++railCount << ": " << a->getName() << " → " << b->getName() 
		          << " | Grid: (" << from.x << "," << from.y << ") → (" << to.x << "," << to.y << ")" << std::endl;
		
		// Create L-path: horizontal first, then vertical
		// Corner point is at (to.x, from.y)
		RailPath path;
		path.start = sf::Vector2f(static_cast<float>(from.x), static_cast<float>(from.y));
		path.corner = sf::Vector2f(static_cast<float>(to.x), static_cast<float>(from.y));
		path.end = sf::Vector2f(static_cast<float>(to.x), static_cast<float>(to.y));
		
		railPaths[rail] = path;
		
		// Mark horizontal segment
		sf::Vector2i p = from;
		const int sx = (to.x > p.x) ? 1 : -1;
		const int sy = (to.y > p.y) ? 1 : -1;
		
		int tilesMarked = 0;
		while (p.x != to.x)
		{
			if (_world->isInBounds(p.x, p.y))
			{
				_world->markRailOccupied(p.x, p.y);
				tilesMarked++;
			}
			p.x += sx;
		}
		
		// Mark vertical segment
		while (p.y != to.y)
		{
			if (_world->isInBounds(p.x, p.y))
			{
				_world->markRailOccupied(p.x, p.y);
				tilesMarked++;
			}
			p.y += sy;
		}
		
		if (_world->isInBounds(to.x, to.y))
		{
			_world->markRailOccupied(to.x, to.y);
			tilesMarked++;
		}
		
		std::cout << "    Marked " << tilesMarked << " tiles" << std::endl;
	}
	
	std::cout << "Total rails marked: " << railCount << std::endl << std::endl;
	
	_renderManager.setRailPaths(railPaths);
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

sf::Vector2i SFMLRenderer::gridToWorldOffset(int gx, int gy) const
{
	const int worldSize = _world ? _world->getWidth() : 56;
	const int worldCenter = worldSize / 2;
	
	const int offsetX = worldCenter - _networkCenterX;
	const int offsetY = worldCenter - _networkCenterY;
	
	return sf::Vector2i(gx + offsetX, gy + offsetY);
}