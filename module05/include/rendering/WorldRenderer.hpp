#ifndef WORLDRENDERER_HPP
#define WORLDRENDERER_HPP

#include <SFML/Graphics.hpp>
#include <map>
#include <string>

class Node;
class SpriteAtlas;
class CameraManager;
class World;
struct StationTile;

// Draws terrain, rail and station tiles including city name labels.
class WorldRenderer
{
private:
    const sf::Font*                                  _font;
    const std::map<const Node*, sf::Vector2f>*       _nodePositions;
    const std::map<std::pair<int,int>, int>*          _railCache;
    const std::map<std::pair<int,int>, StationTile>*  _stationCache;

    sf::Vector2f projectIsometric(const sf::Vector2f& worldPoint, const CameraManager& camera) const;
    std::string  getRailSpriteName(int bitmask) const;

public:
    WorldRenderer(const sf::Font*                                  font,
                  const std::map<const Node*, sf::Vector2f>*       nodePositions,
                  const std::map<std::pair<int,int>, int>*          railCache,
                  const std::map<std::pair<int,int>, StationTile>*  stationCache);

    void draw(sf::RenderWindow& window, const SpriteAtlas& atlas,
              const CameraManager& camera, const World& world) const;
};

#endif