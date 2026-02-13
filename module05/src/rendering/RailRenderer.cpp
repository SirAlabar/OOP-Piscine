#include "rendering/RailRenderer.hpp"
#include "rendering/SpriteAtlas.hpp"

void RailRenderer::drawTile(sf::RenderWindow& window,
                            const SpriteAtlas& atlas,
                            const std::string& frameName,
                            const sf::Vector2f& screenPosition) const
{
	if (!atlas.hasFrame(frameName))
	{
		return;
	}

	sf::Sprite sprite(atlas.getTexture(), atlas.getFrame(frameName));
	sprite.setOrigin(24.0f, 24.0f);
	sprite.setPosition(screenPosition);
	window.draw(sprite);
}
