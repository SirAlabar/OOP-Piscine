#include "rendering/RenderManager.hpp"
#include "rendering/TrainPositionInterpolator.hpp"
#include "simulation/SimulationManager.hpp"
#include "rendering/CameraManager.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include "core/Train.hpp"
#include <cmath>
#include <algorithm>
#include <iomanip>

namespace
{
    constexpr int DEFAULT_RAIL_BITMASK = 5;
}

RenderManager::RenderManager()
	: _eventsAtlasLoaded(false),
	  _worldSeed(0)
{
}

void RenderManager::setNodePositions(const std::map<const Node*, sf::Vector2f>& positions)
{
	_nodeWorldPositions = positions;
}

void RenderManager::setNodeGridPositions(const std::map<const Node*, sf::Vector2i>& gridPositions)
{
	_nodeGridPositions = gridPositions;
}

void RenderManager::setRailPaths(const std::map<const Rail*, RailPath>& railPaths)
{
	_railPaths = railPaths;
}

void RenderManager::setWorldSeed(unsigned int seed)
{
	_worldSeed = seed;
}

bool RenderManager::loadEventsAtlas(const std::string& texturePath,
                                    const std::string& jsonPath)
{
	_eventsAtlasLoaded = _eventsAtlas.loadFromFiles(texturePath, jsonPath);
	return _eventsAtlasLoaded;
}

void RenderManager::rebuildLookupCaches()
{
	_railBitmaskCache.clear();
	for (const RailTile& tile : _railTiles)
	{
		_railBitmaskCache[std::make_pair(tile.gridX, tile.gridY)] = tile.bitmask;
	}

	_stationMapCache.clear();
	for (const StationTile& tile : _stationTiles)
	{
		_stationMapCache[std::make_pair(tile.gridX, tile.gridY)] = tile;
	}
}

void RenderManager::buildRailBitmasks(World& world)
{
	_railTiles.clear();

	for (int y = 0; y < world.getHeight(); ++y)
	{
		for (int x = 0; x < world.getWidth(); ++x)
		{
			if (world.isRailOccupied(x, y))
			{
				int bitmask = computeRailBitmask(world, x, y);

				RailTile tile;
				tile.gridX = x;
				tile.gridY = y;
				tile.bitmask = bitmask;
				_railTiles.push_back(tile);
			}
		}
	}
	rebuildLookupCaches();
}

void RenderManager::buildStationTiles(const Graph* graph, World& world)
{
	_stationTiles.clear();

	if (!graph)
	{
		return;
	}

	for (const Node* node : graph->getNodes())
	{
		if (!node || node->getType() != NodeType::CITY)
		{
			continue;
		}

		if (_nodeWorldPositions.count(node) == 0)
		{
			continue;
		}

		sf::Vector2f worldPos = _nodeWorldPositions.at(node);
		int worldX = static_cast<int>(worldPos.x);
		int worldY = static_cast<int>(worldPos.y);

		if (!world.isInBounds(worldX, worldY))
		{
			continue;
		}

		world.markStationOccupied(worldX, worldY);

		// Use explicit rail connections instead of proximity
		int bitmask = computeRailBitmask(world, worldX, worldY);

		// Remap corners to T-junctions for stations
		if (bitmask == 3)  bitmask = 7;   // N+E → N+E+S
		if (bitmask == 6)  bitmask = 14;  // E+S → E+S+W
		if (bitmask == 9)  bitmask = 11;  // N+W → N+S+W
		if (bitmask == 12) bitmask = 13;  // S+W → N+E+W

		StationTile stationTile;
		stationTile.gridX = worldX;
		stationTile.gridY = worldY;
		stationTile.bitmask = bitmask;
		stationTile.node = node;
		_stationTiles.push_back(stationTile);
	}
	rebuildLookupCaches();
}

void RenderManager::render(sf::RenderWindow& window, const SpriteAtlas& atlas,
                            const SimulationManager& simulation, const CameraManager& camera,
                            const World& world)
{
	window.clear(sf::Color(18, 18, 24));

	// Render world layers (bottom to top)
	renderWorld(window, atlas, camera, world);
	renderEvents(window, camera, simulation);
	renderTrains(window, atlas, simulation, camera);
	renderDayNightOverlay(window, simulation);

	// UI layer
	_hudRenderer.draw(window, simulation.getCurrentTime(), simulation.getSimulationSpeed());

	window.display();
}

void RenderManager::renderWorld(sf::RenderWindow& window, const SpriteAtlas& atlas,
                                 const CameraManager& camera, const World& world)
{
	_worldRenderer.draw(window, atlas, camera, world,
	                    _railBitmaskCache, _stationMapCache, _hudRenderer);
}

void RenderManager::renderTrains(sf::RenderWindow& window, const SpriteAtlas& atlas,
                                 const SimulationManager& simulation, const CameraManager& camera)
{
	float zoom = camera.getCurrentZoom();

	for (const Train* train : simulation.getTrains())
	{
		if (!train || !train->getCurrentRail())
		{
			continue;
		}

		bool movingRight = true;
		sf::Vector2f trainPos = TrainPositionInterpolator::compute(
			train, camera, _nodeWorldPositions, _railPaths, movingRight);
		_trainRenderer.draw(window, atlas, train, trainPos, movingRight, zoom);
		_hudRenderer.drawTrainLabel(window, train->getName(), trainPos, zoom);
	}
}

void RenderManager::renderEvents(sf::RenderWindow& window,
                                  const CameraManager& camera,
                                  const SimulationManager& simulation)
{
	if (!_eventsAtlasLoaded)
	{
		return;
	}

	const float timeSeconds = static_cast<float>(simulation.getCurrentTime());

	_eventRenderer.draw(window, _eventsAtlas, camera,
	                    _nodeWorldPositions, _railPaths,
	                    timeSeconds,
	                    simulation.getActiveEvents());
}

void RenderManager::renderDayNightOverlay(sf::RenderWindow& window, const SimulationManager& simulation)
{
	float intensity = calculateDayNightIntensity(simulation.getCurrentTime());

	if (intensity > 0.05f)
	{
		sf::RectangleShape overlay(sf::Vector2f(
			static_cast<float>(window.getSize().x),
			static_cast<float>(window.getSize().y)
		));

		sf::Uint8 alpha = static_cast<sf::Uint8>(intensity * 200.0f);
		overlay.setFillColor(sf::Color(20, 30, 60, alpha));

		window.draw(overlay);
	}
}

int RenderManager::computeRailBitmask(const World& world, int x, int y) const
{
	int mask = world.getRailMask(x, y);

	if (mask == 0)
	{
		mask = DEFAULT_RAIL_BITMASK;
	}

	return mask;
}

float RenderManager::calculateDayNightIntensity(double currentTime) const
{
	// currentTime is in SECONDS, convert to normalized day cycle
	double normalizedTime = std::fmod(currentTime, SimConfig::SECONDS_PER_DAY) / SimConfig::SECONDS_PER_DAY;

	float lightFactor = std::sin(normalizedTime * 2.0 * 3.14159265359 - 3.14159265359 / 2.0) * 0.5f + 0.5f;

	float darkness = 1.0f - lightFactor;

	return darkness;
}