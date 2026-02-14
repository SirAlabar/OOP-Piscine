#ifndef TRAINRENDERER_HPP
#define TRAINRENDERER_HPP

#include <SFML/Graphics.hpp>

class SpriteAtlas;
class Train;

class TrainRenderer
{
public:
	TrainRenderer() = default;
	
	void draw(sf::RenderWindow& window, const SpriteAtlas& atlas,
	          const Train* train, const sf::Vector2f& position, bool movingRight, float zoom);
};

#endif