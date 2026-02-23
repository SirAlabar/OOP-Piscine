#include "rendering/renderers/WorldRenderer.hpp"
#include "rendering/renderers/RailRenderer.hpp"
#include "rendering/graphics/SpriteAtlas.hpp"
#include "rendering/systems/CameraManager.hpp"
#include "rendering/renderers/HudRenderer.hpp"
#include "utils/IsometricUtils.hpp"
#include "world/World.hpp"
#include "core/Node.hpp"

sf::Vector2f WorldRenderer::projectIsometric(const sf::Vector2f& worldPoint, const CameraManager& camera) const
{
    return IsometricUtils::project(worldPoint, camera);
}

std::string WorldRenderer::getRailSpriteName(int bitmask) const
{
    return "rail_" + std::to_string(bitmask) + ".png";
}

void WorldRenderer::draw(sf::RenderWindow&                                 window,
                          const SpriteAtlas&                                atlas,
                          const CameraManager&                              camera,
                          const World&                                      world,
                          const std::map<std::pair<int,int>, int>&          railCache,
                          const std::map<std::pair<int,int>, StationTile>&  stationCache,
                          HudRenderer&                                      hud) const
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

            if (stationCache.count(pos) != 0)
            {
                const StationTile& station = stationCache.at(pos);
                std::string sprite = "station_" + std::to_string(station.bitmask) + ".png";

                if (!atlas.hasFrame(sprite))
                {
                    sprite = "station_5.png";
                }

                railRenderer.drawTileScaled(window, atlas, sprite, screenPos, zoom);

                if (station.node)
                {
                    hud.drawCityLabel(window, station.node->getName(), screenPos, zoom);
                }

                continue;
            }

            if (railCache.count(pos) != 0)
            {
                std::string sprite = getRailSpriteName(railCache.at(pos));

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