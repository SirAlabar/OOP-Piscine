#ifndef RAILRENDERER_HPP
#define RAILRENDERER_HPP

#include <SFML/Graphics.hpp>
#include <string>

class SpriteAtlas;

class RailRenderer
{
public:
	void drawTile(sf::RenderWindow& window, const SpriteAtlas& atlas,
	              const std::string& frameName, const sf::Vector2f& screenPosition) const;
};

#endif