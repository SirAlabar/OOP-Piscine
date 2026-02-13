#ifndef NODERENDERER_HPP
#define NODERENDERER_HPP

#include <SFML/Graphics.hpp>

class SpriteAtlas;
class Node;
class World;

class NodeRenderer
{
public:
	NodeRenderer() = default;
	
	void draw(sf::RenderWindow& window, const SpriteAtlas& atlas, 
	          const Node* node, const sf::Vector2f& position);
	
	void draw(sf::RenderWindow& window, const SpriteAtlas& atlas,
	          const Node* node, const sf::Vector2f& position,
	          const World* world, int gridX, int gridY);

private:
	int computeStationBitmask(const World* world, int x, int y) const;
	std::string getStationSpriteName(int bitmask) const;
};

#endif