#ifndef SFMLRENDERER_HPP
#define SFMLRENDERER_HPP

#include "rendering/IRenderer.hpp"
#include "rendering/InputManager.hpp"
#include "rendering/CameraManager.hpp"
#include "rendering/RenderManager.hpp"
#include "rendering/SpriteAtlas.hpp"
#include "rendering/GraphLayoutEngine.hpp"
#include "world/World.hpp"
#include "world/WorldGenerator.hpp"
#include <SFML/Graphics.hpp>
#include <map>
#include <set>

class Graph;
class Node;
class SimulationManager;

class SFMLRenderer : public IRenderer
{
private:
	sf::RenderWindow _window;
	SpriteAtlas _atlas;
	
	InputManager    _inputManager;
	CameraManager   _cameraManager;
	RenderManager   _renderManager;
	GraphLayoutEngine _layoutEngine;  // Stateless; tuning knobs may be set before initialize().
	
	World* _world;
	WorldGenerator* _worldGenerator;
	
	std::map<const Node*, sf::Vector2i> _nodeGridPositions;
	std::set<std::pair<int, int> > _occupiedTiles;
	
	int _networkCenterX;
	int _networkCenterY;
	sf::Clock _clock;
	bool _initialized;

	void initializeWorld(SimulationManager& simulation);
	void markRailsInWorld(const Graph* graph);
	void generateWorld(const Graph* graph);
	void render(const SimulationManager& simulation);
	
	void resetWorldState();
	sf::Vector2i gridToWorldOffset(int gx, int gy) const;

public:
	SFMLRenderer();
	~SFMLRenderer() override;

	void initialize(SimulationManager& simulation) override;
	bool processFrame(SimulationManager& simulation) override;
	void shutdown() override;
};

#endif