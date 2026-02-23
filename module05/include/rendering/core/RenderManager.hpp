#ifndef RENDERMANAGER_HPP
#define RENDERMANAGER_HPP

#include <SFML/Graphics.hpp>
#include <map>
#include <vector>
#include <string>
#include "rendering/core/RenderTypes.hpp"
#include "rendering/graphics/SpriteAtlas.hpp"
#include "rendering/renderers/NodeRenderer.hpp"
#include "rendering/renderers/RailRenderer.hpp"
#include "rendering/renderers/TrainRenderer.hpp"
#include "rendering/renderers/EventRenderer.hpp"
#include "rendering/renderers/HudRenderer.hpp"
#include "rendering/renderers/WorldRenderer.hpp"
#include "world/World.hpp"

class SimulationManager;
class CameraManager;
class Graph;
class Node;
class Rail;
class Train;

class RenderManager
{
private:
	NodeRenderer  _nodeRenderer;
	RailRenderer  _railRenderer;
	TrainRenderer _trainRenderer;
	EventRenderer _eventRenderer;
	HudRenderer   _hudRenderer;
	WorldRenderer _worldRenderer;

	SpriteAtlas _eventsAtlas;
	bool        _eventsAtlasLoaded;

	std::vector<RailTile>               _railTiles;
	std::vector<StationTile>            _stationTiles;
	std::map<const Node*, sf::Vector2f> _nodeWorldPositions;
	std::map<const Node*, sf::Vector2i> _nodeGridPositions;
	std::map<const Rail*, RailPath>     _railPaths;
	unsigned int                        _worldSeed;

	std::map<std::pair<int, int>, int>        _railBitmaskCache;
	std::map<std::pair<int, int>, StationTile> _stationMapCache;

public:
	RenderManager();

	bool loadEventsAtlas(const std::string& texturePath,
	                     const std::string& jsonPath);

	void setNodePositions(const std::map<const Node*, sf::Vector2f>& positions);
	void setNodeGridPositions(const std::map<const Node*, sf::Vector2i>& gridPositions);
	void setRailPaths(const std::map<const Rail*, RailPath>& railPaths);
	void setWorldSeed(unsigned int seed);
	unsigned int getWorldSeed() const { return _worldSeed; }
	void buildRailBitmasks(World& world);
	void buildStationTiles(const Graph* graph, World& world);

	void render(sf::RenderWindow& window, const SpriteAtlas& atlas,
	            const SimulationManager& simulation, const CameraManager& camera,
	            const World& world);

private:
	void renderWorld(sf::RenderWindow& window, const SpriteAtlas& atlas,
	                 const CameraManager& camera, const World& world);
	void renderTrains(sf::RenderWindow& window, const SpriteAtlas& atlas,
	                  const SimulationManager& simulation, const CameraManager& camera);
	void renderEvents(sf::RenderWindow& window, const CameraManager& camera,
	                  const SimulationManager& simulation);
	void renderDayNightOverlay(sf::RenderWindow& window, const SimulationManager& simulation);

	int  computeRailBitmask(const World& world, int x, int y) const;

	float calculateDayNightIntensity(double currentTime) const;

	void rebuildLookupCaches();
};

#endif