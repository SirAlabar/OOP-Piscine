#ifndef RAILRENDERER_HPP
#define RAILRENDERER_HPP

#include <SFML/Graphics.hpp>
#include <string>

class SpriteAtlas;

class RailRenderer
{
public:
	RailRenderer() = default;
	
	void drawTile(sf::RenderWindow& window, const SpriteAtlas& atlas,
	              const std::string& spriteName, const sf::Vector2f& position);
	
	void drawTileScaled(sf::RenderWindow& window, const SpriteAtlas& atlas,
	                    const std::string& spriteName, const sf::Vector2f& position,
	                    float zoom);
};

#endif