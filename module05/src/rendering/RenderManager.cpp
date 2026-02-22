#include "rendering/RenderManager.hpp"
#include "rendering/TrainPositionInterpolator.hpp"
#include "simulation/SimulationManager.hpp"
#include "rendering/CameraManager.hpp"
#include "utils/IsometricUtils.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include "core/Train.hpp"
#include <cmath>
#include <algorithm>
#include <iomanip>

RenderManager::RenderManager()
	: _eventsAtlasLoaded(false),
	  _worldSeed(0),
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

void RenderManager::setRailPaths(const std::map<const Rail*, RailPath>& railPaths)
{
	_railPaths = railPaths;
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

    _fontLoaded = _labelFont.loadFromFile("assets/Minecraft.ttf");

    if (!_fontLoaded)
    {
        throw std::runtime_error("Failed to load UI font: assets/Minecraft.ttf");
    }
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
				uint8_t bitmask = world.getRailMask(x, y);

				if (bitmask == 0)
				{
					bitmask = 5;
				}

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
		int bitmask = world.getRailMask(worldX, worldY);

		if (bitmask == 0)
		{
			bitmask = 5;
		}

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

	// UI LAYER - Draw time and speed at top center
	if (_fontLoaded)
	{
		// _currentTime is in SECONDS, convert to minutes for display
		double currentTimeSeconds = simulation.getCurrentTime();
		int totalMinutes = static_cast<int>(currentTimeSeconds / SimConfig::SECONDS_PER_MINUTE);
		int cycleMinutes = totalMinutes % SimConfig::MINUTES_PER_DAY;  // Minutes in a day
		int hours = cycleMinutes / 60;
		int minutes = cycleMinutes % 60;

		std::ostringstream timeStream;
		timeStream << std::setfill('0') << std::setw(2) << hours
		        << "h" << std::setw(2) << minutes;

		sf::Text timeLabel;
		timeLabel.setFont(_labelFont);
		timeLabel.setString(timeStream.str());
		timeLabel.setCharacterSize(24);
		timeLabel.setFillColor(sf::Color::White);
		timeLabel.setOutlineColor(sf::Color::Black);
		timeLabel.setOutlineThickness(2.0f);

		sf::FloatRect bounds = timeLabel.getLocalBounds();
		timeLabel.setOrigin(bounds.width / 2.0f, 0);
		timeLabel.setPosition(window.getSize().x / 2.0f, 10.0f);

		window.draw(timeLabel);

		// Draw speed indicator below clock
		std::ostringstream speedStream;
		speedStream << std::fixed << std::setprecision(1) << simulation.getSimulationSpeed() << "x";

		sf::Text speedLabel;
		speedLabel.setFont(_labelFont);
		speedLabel.setString(speedStream.str());
		speedLabel.setCharacterSize(18);
		speedLabel.setFillColor(sf::Color(150, 255, 150));  // Light green
		speedLabel.setOutlineColor(sf::Color::Black);
		speedLabel.setOutlineThickness(1.5f);

		sf::FloatRect speedBounds = speedLabel.getLocalBounds();
		speedLabel.setOrigin(speedBounds.width / 2.0f, 0);
		speedLabel.setPosition(window.getSize().x / 2.0f, 40.0f);

		window.draw(speedLabel);
	}

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
		sf::Vector2f trainPos = TrainPositionInterpolator::compute(
			train, camera, _nodeWorldPositions, _railPaths, movingRight);
		_trainRenderer.draw(window, atlas, train, trainPos, movingRight, zoom);

		if (_fontLoaded)
		{
			sf::Text label;
			label.setFont(_labelFont);
			label.setString(train->getName());
			label.setCharacterSize(static_cast<unsigned int>(10 * zoom));
			label.setFillColor(sf::Color(255, 0, 0));
			label.setOutlineColor(sf::Color::Black);
			label.setOutlineThickness(1.0f * zoom);

			sf::FloatRect bounds = label.getLocalBounds();
			label.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
			label.setPosition(trainPos.x, trainPos.y - 20.0f * zoom);

			window.draw(label);
		}
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

sf::Vector2f RenderManager::projectIsometric(const sf::Vector2f& worldPoint, const CameraManager& camera) const
{
	return IsometricUtils::project(worldPoint, camera);
}

int RenderManager::computeRailBitmask(const World& world, int x, int y) const
{
	int mask = world.getRailMask(x, y);

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
	// currentTime is in SECONDS, convert to normalized day cycle
	double normalizedTime = std::fmod(currentTime, SimConfig::SECONDS_PER_DAY) / SimConfig::SECONDS_PER_DAY;

	float lightFactor = std::sin(normalizedTime * 2.0 * 3.14159265359 - 3.14159265359 / 2.0) * 0.5f + 0.5f;

	float darkness = 1.0f - lightFactor;

	return darkness;
}