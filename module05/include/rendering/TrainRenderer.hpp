#ifndef TRAINRENDERER_HPP
#define TRAINRENDERER_HPP

#include <SFML/Graphics.hpp>

class Train;
class SpriteAtlas;

class TrainRenderer
{
public:
	void draw(sf::RenderWindow& window, const SpriteAtlas& atlas,
	          const Train* train, const sf::Vector2f& position,
	          bool movingRight) const;
};

#endif