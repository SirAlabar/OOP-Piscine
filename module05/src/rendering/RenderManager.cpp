#include "rendering/RenderManager.hpp"
#include "simulation/SimulationManager.hpp"
#include "rendering/CameraManager.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include "core/Train.hpp"
#include <cmath>
#include <algorithm>

RenderManager::RenderManager()
	: _worldSeed(42),
	  _fontLoaded(false)
{
	loadFont();
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

void RenderManager::loadFont()
{
	if (_fontLoaded)
	{
		return;
	}
	
	_fontLoaded = _labelFont.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf");
	if (!_fontLoaded)
	{
		_fontLoaded = _labelFont.loadFromFile("/System/Library/Fonts/Helvetica.ttc");
	}
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
		// Only process CITY nodes
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
		
		// Mark this position as station occupied
		world.markStationOccupied(worldX, worldY);
		
		// Compute station bitmask based on rail connections
		int bitmask = 5; // Default
		if (world.isInBounds(worldX, worldY - 1) && world.isRailOccupied(worldX, worldY - 1))
		{
			bitmask |= 1;
		}
		if (world.isInBounds(worldX + 1, worldY) && world.isRailOccupied(worldX + 1, worldY))
		{
			bitmask |= 2;
		}
		if (world.isInBounds(worldX, worldY + 1) && world.isRailOccupied(worldX, worldY + 1))
		{
			bitmask |= 4;
		}
		if (world.isInBounds(worldX - 1, worldY) && world.isRailOccupied(worldX - 1, worldY))
		{
			bitmask |= 8;
		}
		
		if (bitmask == 0)
		{
			bitmask = 5;
		}
		
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
	renderWorld(window, atlas, camera, world);
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
			sf::Vector2f worldPos(static_cast<float>(x), static_cast<float>(y));
			sf::Vector2f screenPos = projectIsometric(worldPos, camera);
			
			std::pair<int, int> pos = std::make_pair(x, y);
			
			// Priority 1: Station (highest priority)
			if (_stationMapCache.count(pos) != 0)
			{
				const StationTile& station = _stationMapCache.at(pos);
				
				// Render station sprite
				std::string stationSprite = "station_" + std::to_string(station.bitmask) + ".png";
				if (!atlas.hasFrame(stationSprite))
				{
					stationSprite = "station_5.png";
				}
				_railRenderer.drawTileScaled(window, atlas, stationSprite, screenPos, zoom);
				
				// Render city label (using cached font)
				if (_fontLoaded && station.node)
				{
					sf::Text label;
					label.setFont(_labelFont); // Using cached font!
					label.setString(station.node->getName());
					label.setCharacterSize(static_cast<unsigned int>(12 * zoom));
					label.setFillColor(sf::Color(255, 255, 0)); // Yellow
					label.setOutlineColor(sf::Color::Black);
					label.setOutlineThickness(1.0f * zoom);
					
					sf::FloatRect bounds = label.getLocalBounds();
					label.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
					label.setPosition(screenPos.x, screenPos.y - 30.0f * zoom);
					
					window.draw(label);
				}
				
				continue; // Station rendered, skip to next tile
			}
			
			// Priority 2: Rail
			if (_railBitmaskCache.count(pos) != 0)
			{
				int bitmask = _railBitmaskCache.at(pos);
				std::string railSprite = getRailSpriteName(bitmask);
				if (!atlas.hasFrame(railSprite))
				{
					railSprite = "rail_5.png";
				}
				_railRenderer.drawTileScaled(window, atlas, railSprite, screenPos, zoom);
				continue; // Rail rendered, skip to next tile
			}
			
			// Priority 3: Terrain (lowest priority)
			std::string spriteName = world.getCachedSprite(x, y);
			
			if (!atlas.hasFrame(spriteName))
			{
				spriteName = "grass_01.png";
			}
			
			_railRenderer.drawTileScaled(window, atlas, spriteName, screenPos, zoom);
		}
	}
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
		sf::Vector2f trainPos = computeTrainPosition(train, camera, movingRight);
		_trainRenderer.draw(window, atlas, train, trainPos, movingRight, zoom);
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
	const PathSegment* segment = train->getCurrentPathSegment();
	if (!segment || !segment->rail)
	{
		movingRight = true;
		return camera.getOffset();
	}
	
	const Node* fromNode = segment->from;
	const Node* toNode = segment->to;
	
	if (_nodeWorldPositions.count(fromNode) == 0 || _nodeWorldPositions.count(toNode) == 0)
	{
		movingRight = true;
		return camera.getOffset();
	}
	
	// Use from->to direction (actual travel direction)
	const sf::Vector2f start = _nodeWorldPositions.at(fromNode);
	const sf::Vector2f end = _nodeWorldPositions.at(toNode);
	float dx = end.x - start.x;
	float dy = end.y - start.y;
	
	// Determine orientation based on rail direction
	// If rail is more horizontal (dx > dy), use right sprite
	// If rail is more vertical (dy > dx), use left sprite
	if (std::abs(dx) > std::abs(dy))
	{
		movingRight = true;  // Horizontal rail uses train_right_10.png
	}
	else
	{
		movingRight = false; // Vertical rail uses train_left_5.png
	}
	
	double railLengthMeters = segment->rail->getLength() * 1000.0;
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

std::string RenderManager::getRailSpriteName(int bitmask) const
{
	return "rail_" + std::to_string(bitmask) + ".png";
}

float RenderManager::calculateDayNightIntensity(double currentTime) const
{
	double normalizedTime = std::fmod(currentTime, static_cast<double>(DAY_LENGTH_MINUTES)) / static_cast<double>(DAY_LENGTH_MINUTES);
	
	float lightFactor = std::sin(normalizedTime * 2.0 * 3.14159265359 - 3.14159265359 / 2.0) * 0.5f + 0.5f;
	
	float darkness = 1.0f - lightFactor;
	
	return darkness;
}