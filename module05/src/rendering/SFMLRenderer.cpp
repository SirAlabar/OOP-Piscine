#include "rendering/SFMLRenderer.hpp"
#include "simulation/SimulationManager.hpp"
#include "core/Graph.hpp"
#include "core/Rail.hpp"
#include "core/Node.hpp"
#include "core/Train.hpp"
#include <algorithm>
#include <cmath>
#include <queue>
#include <stdexcept>

SFMLRenderer::SFMLRenderer()
	: _window(sf::VideoMode(1400, 900), "Railway Simulation - Isometric"),
	  _cameraOffset(700.0f, 180.0f),
	  _timeAccumulator(0.0),
	  _simulationStepInterval(0.10),
	  _zoom(1.0),
	  _dragging(false),
	  _lastMousePixel(0, 0)
{
	_window.setFramerateLimit(60);
	if (!_atlas.loadFromFiles("assets/assets.png", "assets/assets.json"))
	{
		throw std::runtime_error("Failed to load assets atlas (assets/assets.png + assets/assets.json)");
	}
}

void SFMLRenderer::run(SimulationManager& simulation)
{
	buildGraphLayout(simulation.getNetwork());
	simulation.start();

	sf::Clock clock;
	while (_window.isOpen())
	{
		processEvents(simulation);
		double realDt = clock.restart().asSeconds();
		update(simulation, realDt);
		drawFrame(simulation);
	}

	simulation.stop();
}

void SFMLRenderer::processEvents(SimulationManager& simulation)
{
	sf::Event event;
	while (_window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
		{
			simulation.stop();
			_window.close();
		}
		else if (event.type == sf::Event::MouseWheelScrolled)
		{
			if (event.mouseWheelScroll.delta > 0.0f)
			{
				_zoom *= 1.10;
			}
			else if (event.mouseWheelScroll.delta < 0.0f)
			{
				_zoom /= 1.10;
			}
			_zoom = std::max(0.35, std::min(2.8, _zoom));
		}
		else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Middle)
		{
			_dragging = true;
			_lastMousePixel = sf::Mouse::getPosition(_window);
		}
		else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Middle)
		{
			_dragging = false;
		}
		else if (event.type == sf::Event::MouseMoved && _dragging)
		{
			const sf::Vector2i current = sf::Mouse::getPosition(_window);
			const sf::Vector2i delta = current - _lastMousePixel;
			_cameraOffset += sf::Vector2f(static_cast<float>(delta.x), static_cast<float>(delta.y));
			_lastMousePixel = current;
		}
	}
}

void SFMLRenderer::update(SimulationManager& simulation, double realDt)
{
	handleKeyboardPan(realDt);

	_timeAccumulator += realDt;
	while (_timeAccumulator >= _simulationStepInterval)
	{
		simulation.step();
		_timeAccumulator -= _simulationStepInterval;
	}
}

void SFMLRenderer::handleKeyboardPan(double realDt)
{
	const float speed = static_cast<float>(420.0 * realDt);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
	{
		_cameraOffset.y += speed;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
	{
		_cameraOffset.y -= speed;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
	{
		_cameraOffset.x += speed;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
	{
		_cameraOffset.x -= speed;
	}
}

void SFMLRenderer::drawFrame(const SimulationManager& simulation)
{
	_window.clear(sf::Color(18, 18, 24));

	drawEnvironment();

	for (std::vector<RailTile>::const_iterator it = _railTiles.begin(); it != _railTiles.end(); ++it)
	{
		const sf::Vector2f pos = projectGrid(it->grid.x, it->grid.y);
		_railRenderer.drawTile(_window, _atlas, it->sprite, pos);
	}

	const Graph* graph = simulation.getNetwork();
	if (graph)
	{
		for (const Node* node : graph->getNodes())
		{
			if (!node || _nodeWorldPositions.count(node) == 0)
			{
				continue;
			}

			_nodeRenderer.draw(_window, _atlas, node, projectIsometric(_nodeWorldPositions[node]));
		}
	}

	for (const Train* train : simulation.getTrains())
	{
		if (!train || !train->getCurrentRail())
		{
			continue;
		}

		bool movingRight = true;
		const sf::Vector2f trainPos = computeTrainPosition(train, movingRight);
		_trainRenderer.draw(_window, _atlas, train, trainPos, movingRight);
	}

	_window.display();
}

void SFMLRenderer::buildGraphLayout(const Graph* graph)
{
	_nodeGridPositions.clear();
	_nodeWorldPositions.clear();
	_railTiles.clear();
	_occupiedTiles.clear();

	if (!graph)
	{
		return;
	}

	const Graph::NodeList nodes = graph->getNodes();
	if (nodes.empty())
	{
		return;
	}

	const Node* root = nodes[0];
	size_t rootDegree = graph->getRailsFromNode(const_cast<Node*>(nodes[0])).size();
	for (size_t i = 1; i < nodes.size(); ++i)
	{
		const size_t degree = graph->getRailsFromNode(const_cast<Node*>(nodes[i])).size();
		if (degree > rootDegree)
		{
			root = nodes[i];
			rootDegree = degree;
		}
	}

	const std::vector<sf::Vector2i> dirs = {
		sf::Vector2i(4, 0), sf::Vector2i(-4, 0), sf::Vector2i(0, 4), sf::Vector2i(0, -4),
		sf::Vector2i(4, 4), sf::Vector2i(-4, -4), sf::Vector2i(4, -4), sf::Vector2i(-4, 4)
	};

	std::queue<const Node*> q;
	std::set<const Node*> visited;
	_nodeGridPositions[root] = sf::Vector2i(0, 0);
	visited.insert(root);
	q.push(root);

	while (!q.empty())
	{
		const Node* current = q.front();
		q.pop();
		const sf::Vector2i base = _nodeGridPositions[current];

		const Graph::RailList rails = graph->getRailsFromNode(const_cast<Node*>(current));
		size_t pick = 0;
		for (Rail* rail : rails)
		{
			if (!rail)
			{
				continue;
			}

			Node* otherMutable = rail->getOtherNode(const_cast<Node*>(current));
			const Node* other = otherMutable;
			if (!other)
			{
				continue;
			}

			if (visited.count(other) == 0)
			{
				sf::Vector2i candidate = base + dirs[pick % dirs.size()];
				while (_occupiedTiles.count(std::make_pair(candidate.x, candidate.y)) != 0)
				{
					pick++;
					candidate = base + dirs[pick % dirs.size()] + sf::Vector2i(static_cast<int>(pick / dirs.size()) * 2, 0);
				}
				_nodeGridPositions[other] = candidate;
				_occupiedTiles.insert(std::make_pair(candidate.x, candidate.y));
				visited.insert(other);
				q.push(other);
				pick++;
			}
		}
	}

	for (const Node* node : nodes)
	{
		if (_nodeGridPositions.count(node) == 0)
		{
			const int fallback = static_cast<int>(_nodeGridPositions.size()) * 3;
			_nodeGridPositions[node] = sf::Vector2i(fallback, fallback % 7);
			_occupiedTiles.insert(std::make_pair(fallback, fallback % 7));
		}
		const sf::Vector2i p = _nodeGridPositions[node];
		_nodeWorldPositions[node] = sf::Vector2f(static_cast<float>(p.x), static_cast<float>(p.y));
	}

	buildRailTiles(graph);
}

void SFMLRenderer::pushRailTile(const sf::Vector2i& grid, const std::string& sprite)
{
	if (_nodeGridPositions.empty())
	{
		return;
	}
	RailTile tile;
	tile.grid = grid;
	tile.sprite = sprite;
	_railTiles.push_back(tile);
	_occupiedTiles.insert(std::make_pair(grid.x, grid.y));
}

void SFMLRenderer::routeRail(const sf::Vector2i& from, const sf::Vector2i& to)
{
	sf::Vector2i p = from;
	const int sx = (to.x > p.x) ? 1 : -1;
	const int sy = (to.y > p.y) ? 1 : -1;

	bool movedX = false;
	while (p.x != to.x)
	{
		p.x += sx;
		if (p != to)
		{
			pushRailTile(p, "rail_x.png");
		}
		movedX = true;
	}

	if (movedX && p.y != to.y)
	{
		const std::string turn = ((sx > 0 && sy > 0) || (sx < 0 && sy < 0)) ? "right_turn_down.png" : "left_turn_down.png";
		pushRailTile(p, turn);
	}

	while (p.y != to.y)
	{
		p.y += sy;
		if (p != to)
		{
			pushRailTile(p, "rail_x.png");
		}
	}
}

void SFMLRenderer::buildRailTiles(const Graph* graph)
{
	if (!graph)
	{
		return;
	}

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

		routeRail(_nodeGridPositions[a], _nodeGridPositions[b]);
	}
}

void SFMLRenderer::drawEnvironment()
{
	if (!_atlas.hasFrame("terrain_grass_01.png"))
	{
		return;
	}

	const int minX = -28;
	const int maxX = 28;
	const int minY = -20;
	const int maxY = 20;

	for (int y = minY; y <= maxY; ++y)
	{
		for (int x = minX; x <= maxX; ++x)
		{
			if (_occupiedTiles.count(std::make_pair(x, y)) != 0)
			{
				continue;
			}

			const int hash = std::abs((x * 73856093) ^ (y * 19349663));
			std::string frame = "terrain_grass_01.png";
			if (hash % 19 == 0 && _atlas.hasFrame("water_01.png"))
			{
				frame = "water_01.png";
			}
			else if (hash % 13 == 0 && _atlas.hasFrame("terrain_forest3.png"))
			{
				frame = "terrain_forest3.png";
			}
			else if (hash % 23 == 0 && _atlas.hasFrame("mountain_01.png"))
			{
				frame = "mountain_01.png";
			}

			_railRenderer.drawTile(_window, _atlas, frame, projectGrid(x, y));
		}
	}
}

sf::Vector2f SFMLRenderer::projectIsometric(const sf::Vector2f& worldPoint) const
{
	const float tileWidth = static_cast<float>(48.0 * _zoom);
	const float tileHeight = static_cast<float>(24.0 * _zoom);
	const float isoX = (worldPoint.x - worldPoint.y) * (tileWidth / 2.0f);
	const float isoY = (worldPoint.x + worldPoint.y) * (tileHeight / 2.0f);
	return sf::Vector2f(isoX + _cameraOffset.x, isoY + _cameraOffset.y);
}

sf::Vector2f SFMLRenderer::projectGrid(int gx, int gy) const
{
	return projectIsometric(sf::Vector2f(static_cast<float>(gx), static_cast<float>(gy)));
}

sf::Vector2f SFMLRenderer::computeTrainPosition(const Train* train, bool& movingRight) const
{
	const Rail* rail = train->getCurrentRail();
	if (!rail)
	{
		movingRight = true;
		return _cameraOffset;
	}

	const Node* nodeA = rail->getNodeA();
	const Node* nodeB = rail->getNodeB();
	if (_nodeWorldPositions.count(nodeA) == 0 || _nodeWorldPositions.count(nodeB) == 0)
	{
		movingRight = true;
		return _cameraOffset;
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

	return projectIsometric(world);
}
