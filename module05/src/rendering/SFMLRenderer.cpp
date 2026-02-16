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

    if (!graph)
        return;

    std::vector<Node*> nodes = graph->getNodes();
    if (nodes.empty())
        return;

    // --- PARAMETERS ---
    const int iterations = 300;
    const float repulsionStrength = 80.0f;
    const float attractionStrength = 0.08f;
    const float damping = 0.75f;
    const float minDistance = 6.0f;

    const float initialRadius = std::max(10.0f, nodes.size() * 2.0f);
    const float maxRadius = nodes.size() * 4.0f;

    // --- STATE ---
    std::map<const Node*, sf::Vector2f> positions;
    std::map<const Node*, sf::Vector2f> velocities;

    // deterministic circular initialization
    const float angleStep = 2.0f * 3.14159265f / static_cast<float>(nodes.size());

    for (size_t i = 0; i < nodes.size(); ++i)
    {
        const Node* node = nodes[i];

        float angle = i * angleStep;

        positions[node] = sf::Vector2f(
            std::cos(angle) * initialRadius,
            std::sin(angle) * initialRadius
        );

        velocities[node] = sf::Vector2f(0.0f, 0.0f);
    }

    // --- FORCE SIMULATION ---
    for (int iter = 0; iter < iterations; ++iter)
    {
        std::map<const Node*, sf::Vector2f> forces;

        for (const Node* node : nodes)
            forces[node] = sf::Vector2f(0.0f, 0.0f);

        // REPULSION
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            for (size_t j = i + 1; j < nodes.size(); ++j)
            {
                const Node* a = nodes[i];
                const Node* b = nodes[j];

                sf::Vector2f delta = positions[b] - positions[a];

                float distSq = delta.x * delta.x + delta.y * delta.y;
                float dist = std::sqrt(distSq);

                if (dist < 0.1f)
                    dist = 0.1f;

                sf::Vector2f dir = delta / dist;

                float force = repulsionStrength / distSq;

                forces[a] -= dir * force;
                forces[b] += dir * force;
            }
        }

        // ATTRACTION
        for (const Rail* rail : graph->getRails())
        {
            const Node* a = rail->getNodeA();
            const Node* b = rail->getNodeB();

            sf::Vector2f delta = positions[b] - positions[a];

            float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);

            if (dist < 0.1f)
                continue;

            sf::Vector2f dir = delta / dist;

            float force = (dist - minDistance) * attractionStrength;

            forces[a] += dir * force;
            forces[b] -= dir * force;
        }

        // INTEGRATION + CLAMP
        for (const Node* node : nodes)
        {
            velocities[node] =
                (velocities[node] + forces[node]) * damping;

            positions[node] += velocities[node];

            float len = std::sqrt(
                positions[node].x * positions[node].x +
                positions[node].y * positions[node].y
            );

            if (len > maxRadius)
            {
                positions[node] =
                    (positions[node] / len) * maxRadius;
            }
        }
    }

    // --- CENTER GRAPH ---
    sf::Vector2f center(0.0f, 0.0f);

    for (const Node* node : nodes)
        center += positions[node];

    center /= static_cast<float>(nodes.size());

    for (const Node* node : nodes)
        positions[node] -= center;

    // --- NORMALIZE LAYOUT TO FIXED SIZE ---

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();

    for (const Node* node : nodes)
    {
        const auto& p = positions[node];

        if (p.x < minX) minX = p.x;
        if (p.x > maxX) maxX = p.x;
        if (p.y < minY) minY = p.y;
        if (p.y > maxY) maxY = p.y;
    }

    float width = maxX - minX;
    float height = maxY - minY;

    float maxDim = std::max(width, height);

    if (maxDim < 1.0f)
        maxDim = 1.0f;

    // desired grid size
    const float targetSize = 40.0f;

    float scale = targetSize / maxDim;

    for (const Node* node : nodes)
    {
        sf::Vector2f p = positions[node];

        p.x = (p.x - minX) * scale;
        p.y = (p.y - minY) * scale;

        _nodeGridPositions[node] = sf::Vector2i(
            static_cast<int>(std::round(p.x)),
            static_cast<int>(std::round(p.y))
        );
    }

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
		
		RailPath path;
		path.start = sf::Vector2f(static_cast<float>(from.x), static_cast<float>(from.y));
		path.corner = sf::Vector2f(static_cast<float>(to.x), static_cast<float>(from.y));
		path.end = sf::Vector2f(static_cast<float>(to.x), static_cast<float>(to.y));
		
		railPaths[rail] = path;
		
		const int sx = (to.x > from.x) ? 1 : -1;
		const int sy = (to.y > from.y) ? 1 : -1;
		
		int tilesMarked = 0;
		
		// Add START node outgoing connection
		if (_world->isInBounds(from.x, from.y))
		{
			if (from.x != to.x)
			{
				// Horizontal segment exists - add outgoing horizontal
				if (sx > 0) _world->addRailConnection(from.x, from.y, RailDir::East);
				else _world->addRailConnection(from.x, from.y, RailDir::West);
				std::cout << "    START (" << from.x << "," << from.y << "): added " 
				          << (sx > 0 ? "East" : "West") << " -> mask=" 
				          << (int)_world->getRailMask(from.x, from.y) << std::endl;
			}
			else if (from.y != to.y)
			{
				// Only vertical segment - add outgoing vertical
				if (sy > 0) _world->addRailConnection(from.x, from.y, RailDir::South);
				else _world->addRailConnection(from.x, from.y, RailDir::North);
				std::cout << "    START (" << from.x << "," << from.y << "): added " 
				          << (sy > 0 ? "South" : "North") << " -> mask=" 
				          << (int)_world->getRailMask(from.x, from.y) << std::endl;
			}
		}
		
		// Mark horizontal segment
		sf::Vector2i p = from;
		p.x += sx; // Skip start node
		while (p.x != to.x)
		{
			if (_world->isInBounds(p.x, p.y))
			{
				_world->markRailOccupied(p.x, p.y);
				_world->addRailConnection(p.x, p.y, RailDir::East | RailDir::West);
				std::cout << "    H-SEG (" << p.x << "," << p.y << "): mask=" 
				          << (int)_world->getRailMask(p.x, p.y) << std::endl;
				tilesMarked++;
			}
			p.x += sx;
		}
		
		// Mark corner tile (if L-shaped rail)
		if (from.x != to.x && from.y != to.y)
		{
			if (_world->isInBounds(p.x, p.y))
			{
				_world->markRailOccupied(p.x, p.y);
				
				RailDir cornerDir = RailDir::None;
				if (sx > 0) cornerDir |= RailDir::West;
				else cornerDir |= RailDir::East;
				if (sy > 0) cornerDir |= RailDir::South;
				else cornerDir |= RailDir::North;
				
				_world->addRailConnection(p.x, p.y, cornerDir);
				std::cout << "    CORNER (" << p.x << "," << p.y << "): mask=" 
				          << (int)_world->getRailMask(p.x, p.y) << std::endl;
				tilesMarked++;
			}
			
			p.y += sy;
		}
		
		// Mark vertical segment
		while (p.y != to.y)
		{
			if (_world->isInBounds(p.x, p.y))
			{
				_world->markRailOccupied(p.x, p.y);
				_world->addRailConnection(p.x, p.y, RailDir::North | RailDir::South);
				std::cout << "    V-SEG (" << p.x << "," << p.y << "): mask=" 
				          << (int)_world->getRailMask(p.x, p.y) << std::endl;
				tilesMarked++;
			}
			p.y += sy;
		}
		
		// Mark destination - add ONLY incoming direction
		if (_world->isInBounds(to.x, to.y))
		{
			_world->markRailOccupied(to.x, to.y);
			
			if (from.y != to.y)
			{
				if (sy > 0) _world->addRailConnection(to.x, to.y, RailDir::North);
				else _world->addRailConnection(to.x, to.y, RailDir::South);
				std::cout << "    END (" << to.x << "," << to.y << "): added " 
				          << (sy > 0 ? "North" : "South") << " -> mask=" 
				          << (int)_world->getRailMask(to.x, to.y) << std::endl;
			}
			else if (from.x != to.x)
			{
				if (sx > 0) _world->addRailConnection(to.x, to.y, RailDir::West);
				else _world->addRailConnection(to.x, to.y, RailDir::East);
				std::cout << "    END (" << to.x << "," << to.y << "): added " 
				          << (sx > 0 ? "West" : "East") << " -> mask=" 
				          << (int)_world->getRailMask(to.x, to.y) << std::endl;
			}
			
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