#ifndef WORLDRENDERER_HPP
#define WORLDRENDERER_HPP

#include "rendering/RenderTypes.hpp"
#include <SFML/Graphics.hpp>
#include <map>
#include <string>

class SpriteAtlas;
class CameraManager;
class HudRenderer;
class World;

class WorldRenderer
{
private:
    sf::Vector2f projectIsometric(const sf::Vector2f& worldPoint, const CameraManager& camera) const;
    std::string  getRailSpriteName(int bitmask) const;

public:
    WorldRenderer() = default;

    void draw(sf::RenderWindow&                              window,
              const SpriteAtlas&                             atlas,
              const CameraManager&                           camera,
              const World&                                   world,
              const std::map<std::pair<int,int>, int>&       railCache,
              const std::map<std::pair<int,int>, StationTile>& stationCache,
              HudRenderer&                                   hud) const;
};

#endif