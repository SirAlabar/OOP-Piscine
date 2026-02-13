#ifndef NODERENDERER_HPP
#define NODERENDERER_HPP

#include <SFML/Graphics.hpp>

class Node;
class SpriteAtlas;

class NodeRenderer
{
private:
	sf::Font _font;
	bool _fontLoaded;

public:
	NodeRenderer();

	void draw(sf::RenderWindow& window, const SpriteAtlas& atlas,
	          const Node* node, const sf::Vector2f& screenPosition) const;
};

#endif