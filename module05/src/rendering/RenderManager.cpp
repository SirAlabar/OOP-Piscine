#include "rendering/RenderManager.hpp"
#include "simulation/SimulationManager.hpp"
#include "rendering/CameraManager.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include "core/Train.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>

RenderManager::RenderManager()
	: _worldSeed(42)
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

void RenderManager::setWorldSeed(unsigned int seed)
{
	_worldSeed = seed;
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
	
	std::cout << "[RenderManager] Built " << _railTiles.size() << " rail tiles with bitmasks" << std::endl;
}

void RenderManager::render(sf::RenderWindow& window, const SpriteAtlas& atlas,
                            const SimulationManager& simulation, const CameraManager& camera,
                            const World& world)
{
	window.clear(sf::Color(18, 18, 24));
	
	renderWorld(window, atlas, camera, world);
	renderRails(window, atlas, camera, world);
	
	const Graph* graph = simulation.getNetwork();
	if (graph)
	{
		for (const Node* node : graph->getNodes())
		{
			if (!node || _nodeWorldPositions.count(node) == 0)
			{
				continue;
			}
			
			sf::Vector2f screenPos = projectIsometric(_nodeWorldPositions.at(node), camera);
			
			if (_nodeGridPositions.count(node) != 0)
			{
				sf::Vector2i gridPos = _nodeGridPositions.at(node);
				_nodeRenderer.draw(window, atlas, node, screenPos, &world, gridPos.x, gridPos.y);
			}
			else
			{
				_nodeRenderer.draw(window, atlas, node, screenPos);
			}
		}
	}
	
	renderTrains(window, atlas, simulation, camera);
	renderDayNightOverlay(window, simulation);
	
	window.display();
}

void RenderManager::renderWorld(sf::RenderWindow& window, const SpriteAtlas& atlas,
                                 const CameraManager& camera, const World& world)
{
	float zoom = camera.getCurrentZoom();
	
	for (int y = 0; y < world.getHeight(); ++y)
	{
		for (int x = 0; x < world.getWidth(); ++x)
		{
			BiomeType biome = world.getTile(x, y);
			std::string spriteName = world.getBiomeSpriteName(biome, x, y, _worldSeed);
			
			if (!atlas.hasFrame(spriteName))
			{
				spriteName = "grass_01.png";
			}
			
			sf::Vector2f worldPos(static_cast<float>(x), static_cast<float>(y));
			sf::Vector2f screenPos = projectIsometric(worldPos, camera);
			
			_railRenderer.drawTileScaled(window, atlas, spriteName, screenPos, zoom);
		}
	}
}

void RenderManager::renderRails(sf::RenderWindow& window, const SpriteAtlas& atlas,
                                const CameraManager& camera, const World& world)
{
	for (const RailTile& tile : _railTiles)
	{
		BiomeType biome = world.getTile(tile.gridX, tile.gridY);
		std::string spriteName = getRailSpriteName(tile.bitmask, biome);
		
		if (!atlas.hasFrame(spriteName))
		{
			spriteName = "rail_" + std::to_string(tile.bitmask) + ".png";
			if (!atlas.hasFrame(spriteName))
			{
				spriteName = "rail_5.png";
			}
		}
		
		sf::Vector2f worldPos(static_cast<float>(tile.gridX), static_cast<float>(tile.gridY));
		sf::Vector2f screenPos = projectIsometric(worldPos, camera);
		
		float zoom = camera.getCurrentZoom();
		_railRenderer.drawTileScaled(window, atlas, spriteName, screenPos, zoom);
	}
}

void RenderManager::renderTrains(sf::RenderWindow& window, const SpriteAtlas& atlas,
                                 const SimulationManager& simulation, const CameraManager& camera)
{
	for (const Train* train : simulation.getTrains())
	{
		if (!train || !train->getCurrentRail())
		{
			continue;
		}
		
		bool movingRight = true;
		sf::Vector2f trainPos = computeTrainPosition(train, camera, movingRight);
		_trainRenderer.draw(window, atlas, train, trainPos, movingRight);
	}
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

sf::Vector2f RenderManager::projectIsometric(const sf::Vector2f& worldPoint, const CameraManager& camera) const
{
	const float zoom = camera.getCurrentZoom();
	const sf::Vector2f offset = camera.getOffset();
	
	const float baseTileWidth = 48.0f;
	const float baseTileHeight = 24.0f;
	
	const float isoX = (worldPoint.x - worldPoint.y) * (baseTileWidth / 2.0f) * zoom;
	const float isoY = (worldPoint.x + worldPoint.y) * (baseTileHeight / 2.0f) * zoom;
	
	return sf::Vector2f(isoX + offset.x, isoY + offset.y);
}

sf::Vector2f RenderManager::computeTrainPosition(const Train* train, const CameraManager& camera, bool& movingRight) const
{
	const Rail* rail = train->getCurrentRail();
	if (!rail)
	{
		movingRight = true;
		return camera.getOffset();
	}
	
	const Node* nodeA = rail->getNodeA();
	const Node* nodeB = rail->getNodeB();
	
	if (_nodeWorldPositions.count(nodeA) == 0 || _nodeWorldPositions.count(nodeB) == 0)
	{
		movingRight = true;
		return camera.getOffset();
	}
	
	const sf::Vector2f start = _nodeWorldPositions.at(nodeA);
	const sf::Vector2f end = _nodeWorldPositions.at(nodeB);
	movingRight = (end.x - start.x) >= 0.0f;
	
	double railLengthMeters = rail->getLength() * 1000.0;
	double t = (railLengthMeters > 0.0) ? (train->getPosition() / railLengthMeters) : 0.0;
	t = std::max(0.0, std::min(1.0, t));
	
	const sf::Vector2f world(
		start.x + static_cast<float>((end.x - start.x) * t),
		start.y + static_cast<float>((end.y - start.y) * t)
	);
	
	return projectIsometric(world, camera);
}

int RenderManager::computeRailBitmask(const World& world, int x, int y) const
{
	int mask = 0;
	
	if (world.isInBounds(x, y - 1) && world.isRailOccupied(x, y - 1))
	{
		mask |= 1;
	}
	if (world.isInBounds(x + 1, y) && world.isRailOccupied(x + 1, y))
	{
		mask |= 2;
	}
	if (world.isInBounds(x, y + 1) && world.isRailOccupied(x, y + 1))
	{
		mask |= 4;
	}
	if (world.isInBounds(x - 1, y) && world.isRailOccupied(x - 1, y))
	{
		mask |= 8;
	}
	
	if (mask == 0)
	{
		mask = 5;
	}
	
	return mask;
}

std::string RenderManager::getRailSpriteName(int bitmask, BiomeType biome) const
{
	std::string prefix;
	
	switch (biome)
	{
		case BiomeType::Snow:
			prefix = "snow_path_";
			break;
		case BiomeType::Desert:
			prefix = "desert_path_";
			break;
		case BiomeType::Forest:
			prefix = "forest_path_";
			break;
		case BiomeType::Grass:
		case BiomeType::Mountain:
		case BiomeType::Water:
		default:
			return "rail_" + std::to_string(bitmask) + ".png";
	}
	
	return prefix + std::to_string(bitmask) + ".png";
}

float RenderManager::calculateDayNightIntensity(double currentTime) const
{
	double normalizedTime = std::fmod(currentTime, static_cast<double>(DAY_LENGTH_MINUTES)) / static_cast<double>(DAY_LENGTH_MINUTES);
	
	float lightFactor = std::sin(normalizedTime * 2.0 * 3.14159265359 - 3.14159265359 / 2.0) * 0.5f + 0.5f;
	
	float darkness = 1.0f - lightFactor;
	
	return darkness;
}