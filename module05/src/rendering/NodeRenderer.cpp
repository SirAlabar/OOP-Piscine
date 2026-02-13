#include "rendering/NodeRenderer.hpp"
#include "rendering/SpriteAtlas.hpp"
#include "core/Node.hpp"

NodeRenderer::NodeRenderer()
	: _fontLoaded(_font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"))
{
}

void NodeRenderer::draw(sf::RenderWindow& window,
                        const SpriteAtlas& atlas,
                        const Node* node,
                        const sf::Vector2f& screenPosition) const
{
	if (!node)
	{
		return;
	}

	const std::string spriteName = (node->getType() == NodeType::CITY)
		? "station_left.png"
		: "terrain_forest1.png";

	if (atlas.hasFrame(spriteName))
	{
		sf::Sprite sprite(atlas.getTexture(), atlas.getFrame(spriteName));
		sprite.setOrigin(24.0f, 24.0f);
		sprite.setPosition(screenPosition);
		window.draw(sprite);
	}

	if (_fontLoaded)
	{
		sf::Text label(node->getName(), _font, 12);
		label.setFillColor(sf::Color(242, 242, 242));
		label.setPosition(screenPosition.x + 12.0f, screenPosition.y - 16.0f);
		window.draw(label);
	}
}
