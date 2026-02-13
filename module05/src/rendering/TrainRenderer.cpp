#include "rendering/TrainRenderer.hpp"
#include "rendering/SpriteAtlas.hpp"
#include "core/Train.hpp"

void TrainRenderer::draw(sf::RenderWindow& window,
                         const SpriteAtlas& atlas,
                         const Train* train,
                         const sf::Vector2f& position,
                         bool movingRight) const
{
	if (!train)
	{
		return;
	}

	const std::string frameName = movingRight ? "train_right.png" : "train_left.png";
	if (!atlas.hasFrame(frameName))
	{
		return;
	}

	sf::Sprite sprite(atlas.getTexture(), atlas.getFrame(frameName));
	sprite.setOrigin(24.0f, 24.0f);
	sprite.setPosition(position);
	window.draw(sprite);
}
