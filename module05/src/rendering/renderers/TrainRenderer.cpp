#include "rendering/renderers/TrainRenderer.hpp"
#include "rendering/graphics/SpriteAtlas.hpp"
#include "core/Train.hpp"

void TrainRenderer::draw(sf::RenderWindow& window, const SpriteAtlas& atlas,
                         const Train* train, const sf::Vector2f& position, bool movingRight, float zoom)
{
	if (!train)
	{
		return;
	}
	
	std::string spriteName = movingRight ? "train_right_10.png" : "train_left_5.png";
	
	if (!atlas.hasFrame(spriteName))
	{
		return;
	}
	
	sf::Sprite sprite;
	sprite.setTexture(atlas.getTexture());
	sprite.setTextureRect(atlas.getFrame(spriteName));
	sprite.setOrigin(24.0f, 24.0f);
	sprite.setPosition(position);
	sprite.setScale(zoom, zoom);
	
	window.draw(sprite);
}