#include "rendering/NodeRenderer.hpp"
#include "rendering/SpriteAtlas.hpp"
#include "core/Node.hpp"
#include "world/World.hpp"

void NodeRenderer::draw(sf::RenderWindow& window, const SpriteAtlas& atlas,
                        const Node* node, const sf::Vector2f& position, float zoom)
{
	if (!node)
	{
		return;
	}
	
	std::string spriteName = "station_5.png";
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

void NodeRenderer::draw(sf::RenderWindow& window, const SpriteAtlas& atlas,
                        const Node* node, const sf::Vector2f& position,
                        const World* world, int worldX, int worldY, float zoom)
{
	if (!node)
	{
		return;
	}
	
	int bitmask = 5;
	if (world)
	{
		bitmask = computeStationBitmask(world, worldX, worldY);
	}
	
	std::string spriteName = getStationSpriteName(bitmask);
	if (!atlas.hasFrame(spriteName))
	{
		spriteName = "station_5.png";
	}
	
	sf::Sprite sprite;
	sprite.setTexture(atlas.getTexture());
	sprite.setTextureRect(atlas.getFrame(spriteName));
	sprite.setOrigin(24.0f, 24.0f);
	sprite.setPosition(position);
	sprite.setScale(zoom, zoom);
	
	window.draw(sprite);
}

int NodeRenderer::computeStationBitmask(const World* world, int x, int y) const
{
	if (!world)
	{
		return 5;
	}
	
	int mask = 0;
	
	if (world->isInBounds(x, y - 1) && world->isRailOccupied(x, y - 1))
	{
		mask |= 1;
	}
	if (world->isInBounds(x + 1, y) && world->isRailOccupied(x + 1, y))
	{
		mask |= 2;
	}
	if (world->isInBounds(x, y + 1) && world->isRailOccupied(x, y + 1))
	{
		mask |= 4;
	}
	if (world->isInBounds(x - 1, y) && world->isRailOccupied(x - 1, y))
	{
		mask |= 8;
	}
	
	if (mask == 0)
	{
		mask = 5;
	}
	
	return mask;
}

std::string NodeRenderer::getStationSpriteName(int bitmask) const
{
	switch (bitmask)
	{
		case 1:  return "station_14.png";
		case 2:  return "station_13.png";
		case 3:  return "station_11.png";
		case 4:  return "station_14.png";
		case 5:  return "station_5.png";
		case 6:  return "station_7.png";
		case 7:  return "station_7.png";
		case 8:  return "station_7.png";
		case 9:  return "station_13.png";
		case 10: return "station_10.png";
		case 11: return "station_11.png";
		case 12: return "station_14.png";
		case 13: return "station_13.png";
		case 14: return "station_14.png";
		case 15: return "station_15.png";
		default: return "station_5.png";
	}
}