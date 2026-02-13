#include "rendering/RailRenderer.hpp"
#include "rendering/SpriteAtlas.hpp"

void RailRenderer::drawTile(sf::RenderWindow& window, const SpriteAtlas& atlas,
                            const std::string& spriteName, const sf::Vector2f& position)
{
	if (!atlas.hasFrame(spriteName))
	{
		return;
	}
	
	sf::Sprite sprite;
	sprite.setTexture(atlas.getTexture());
	sprite.setTextureRect(atlas.getFrame(spriteName));
	sprite.setOrigin(24.0f, 24.0f);
	sprite.setPosition(position);
	
	window.draw(sprite);
}

void RailRenderer::drawTileScaled(sf::RenderWindow& window, const SpriteAtlas& atlas,
                                   const std::string& spriteName, const sf::Vector2f& position,
                                   float zoom)
{
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