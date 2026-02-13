#ifndef SFMLRENDERER_HPP
#define SFMLRENDERER_HPP

#include "rendering/IRenderer.hpp"
#include "rendering/InputManager.hpp"
#include "rendering/CameraManager.hpp"
#include "rendering/RenderManager.hpp"
#include "rendering/SpriteAtlas.hpp"
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
	
	InputManager _inputManager;
	CameraManager _cameraManager;
	RenderManager _renderManager;
	
	World* _world;
	WorldGenerator* _worldGenerator;
	
	std::map<const Node*, sf::Vector2i> _nodeGridPositions;
	std::set<std::pair<int, int> > _occupiedTiles;
	
	double _timeAccumulator;
	double _simulationStepInterval;

public:
	SFMLRenderer();
	~SFMLRenderer() override;

	void run(SimulationManager& simulation) override;

private:
	void initializeWorld(SimulationManager& simulation);
	void buildGraphLayout(const Graph* graph);
	void markRailsInWorld(const Graph* graph);
	void generateWorld();
	
	void mainLoop(SimulationManager& simulation);
	void processInput(SimulationManager& simulation, double deltaTime);
	void updateSimulation(SimulationManager& simulation, double realDt);
	void render(const SimulationManager& simulation);
	
	sf::Vector2i gridToWorldOffset(int gx, int gy) const;
};

#endif