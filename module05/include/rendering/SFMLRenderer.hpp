#ifndef SFMLRENDERER_HPP
#define SFMLRENDERER_HPP

#include "rendering/IRenderer.hpp"
#include "rendering/NodeRenderer.hpp"
#include "rendering/RailRenderer.hpp"
#include "rendering/TrainRenderer.hpp"
#include "rendering/SpriteAtlas.hpp"
#include <SFML/Graphics.hpp>
#include <map>
#include <set>
#include <vector>
#include <utility>

class Graph;
class Node;
class Rail;
class Train;

class SFMLRenderer : public IRenderer
{
public:
	SFMLRenderer();
	~SFMLRenderer() override = default;

	void run(SimulationManager& simulation) override;

private:
	struct RailTile
	{
		sf::Vector2i grid;
		std::string sprite;
	};

	sf::RenderWindow _window;
	SpriteAtlas _atlas;
	NodeRenderer _nodeRenderer;
	RailRenderer _railRenderer;
	TrainRenderer _trainRenderer;

	std::map<const Node*, sf::Vector2i> _nodeGridPositions;
	std::map<const Node*, sf::Vector2f> _nodeWorldPositions;
	std::vector<RailTile> _railTiles;
	std::set<std::pair<int, int> > _occupiedTiles;
	sf::Vector2f _cameraOffset;
	double _timeAccumulator;
	double _simulationStepInterval;
	double _zoom;
	bool _dragging;
	sf::Vector2i _lastMousePixel;

	void processEvents(SimulationManager& simulation);
	void update(SimulationManager& simulation, double realDt);
	void handleKeyboardPan(double realDt);
	void drawFrame(const SimulationManager& simulation);

	void buildGraphLayout(const Graph* graph);
	void buildRailTiles(const Graph* graph);
	void routeRail(const sf::Vector2i& from, const sf::Vector2i& to);
	void pushRailTile(const sf::Vector2i& grid, const std::string& sprite);
	void drawEnvironment();
	sf::Vector2f projectIsometric(const sf::Vector2f& worldPoint) const;
	sf::Vector2f projectGrid(int gx, int gy) const;
	sf::Vector2f computeTrainPosition(const Train* train, bool& movingRight) const;
};

#endif