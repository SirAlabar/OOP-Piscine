#ifndef RENDERTYPES_HPP
#define RENDERTYPES_HPP

#include <SFML/Graphics.hpp>

class Node;

struct RailTile
{
	int gridX;
	int gridY;
	int bitmask;
};

struct StationTile
{
	int gridX;
	int gridY;
	int bitmask;
	const Node* node;
};

struct RailPath
{
	sf::Vector2f start;
	sf::Vector2f corner;
	sf::Vector2f end;
};

#endif