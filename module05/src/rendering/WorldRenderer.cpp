#include "rendering/WorldRenderer.hpp"
#include "rendering/RenderManager.hpp"
#include "rendering/RailRenderer.hpp"
#include "rendering/SpriteAtlas.hpp"
#include "rendering/CameraManager.hpp"
#include "utils/IsometricUtils.hpp"
#include "world/World.hpp"
#include "core/Node.hpp"

WorldRenderer::WorldRenderer(const sf::Font*                                  font,
                             const std::map<const Node*, sf::Vector2f>*       nodePositions,
                             const std::map<std::pair<int,int>, int>*          railCache,
                             const std::map<std::pair<int,int>, StationTile>*  stationCache)
    : _font(font), _nodePositions(nodePositions), _railCache(railCache), _stationCache(stationCache)
{
}

sf::Vector2f WorldRenderer::projectIsometric(const sf::Vector2f& worldPoint, const CameraManager& camera) const
{
    return IsometricUtils::project(worldPoint, camera);
}

std::string WorldRenderer::getRailSpriteName(int bitmask) const
{
    return "rail_" + std::to_string(bitmask) + ".png";
}

void WorldRenderer::draw(sf::RenderWindow& window, const SpriteAtlas& atlas,
                         const CameraManager& camera, const World& world) const
{
    RailRenderer railRenderer;
    float zoom = camera.getCurrentZoom();

    for (int y = 0; y < world.getHeight(); ++y)
    {
        for (int x = 0; x < world.getWidth(); ++x)
        {
            sf::Vector2f worldPos(static_cast<float>(x), static_cast<float>(y));
            sf::Vector2f screenPos = projectIsometric(worldPos, camera);
            std::pair<int,int> pos = std::make_pair(x, y);

            if (_stationCache && _stationCache->count(pos) != 0)
            {
                const StationTile& station = _stationCache->at(pos);
                std::string sprite = "station_" + std::to_string(station.bitmask) + ".png";

                if (!atlas.hasFrame(sprite))
                {
                    sprite = "station_5.png";
                }

                railRenderer.drawTileScaled(window, atlas, sprite, screenPos, zoom);

                if (_font && station.node)
                {
                    sf::Text label;
                    label.setFont(*_font);
                    label.setString(station.node->getName());
                    label.setCharacterSize(static_cast<unsigned int>(12 * zoom));
                    label.setFillColor(sf::Color(255, 255, 0));
                    label.setOutlineColor(sf::Color::Black);
                    label.setOutlineThickness(1.0f * zoom);

                    sf::FloatRect bounds = label.getLocalBounds();
                    label.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
                    label.setPosition(screenPos.x, screenPos.y - 30.0f * zoom);
                    window.draw(label);
                }

                continue;
            }

            if (_railCache && _railCache->count(pos) != 0)
            {
                std::string sprite = getRailSpriteName(_railCache->at(pos));

                if (!atlas.hasFrame(sprite))
                {
                    sprite = "rail_5.png";
                }

                railRenderer.drawTileScaled(window, atlas, sprite, screenPos, zoom);
                continue;
            }

            std::string sprite = world.getCachedSprite(x, y);

            if (!atlas.hasFrame(sprite))
            {
                sprite = "grass_01.png";
            }

            railRenderer.drawTileScaled(window, atlas, sprite, screenPos, zoom);
        }
    }
}