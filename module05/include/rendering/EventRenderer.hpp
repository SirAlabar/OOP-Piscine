#ifndef EVENTRENDERER_HPP
#define EVENTRENDERER_HPP

#include <SFML/Graphics.hpp>
#include <map>
#include <string>

class SpriteAtlas;
class CameraManager;
class Node;
class Rail;
class Event;

struct RailPath;

class EventRenderer
{
public:
	EventRenderer() = default;

	// Main entry point — reads active events from EventManager singleton internally
	void draw(sf::RenderWindow&          window,
	          const SpriteAtlas&         eventsAtlas,
	          const CameraManager&       camera,
	          const std::map<const Node*, sf::Vector2f>& nodeWorldPositions,
	          const std::map<const Rail*, RailPath>&     railPaths,
	          float                      elapsedSeconds);

private:
	// --- Icon helpers ---
	void drawEventIcon(sf::RenderWindow&      window,
	                   const SpriteAtlas&     eventsAtlas,
	                   const std::string&     frameName,
	                   const sf::Vector2f&    screenPos,
	                   float                  zoom) const;

	std::string resolveIconFrame(const Event* event) const;

	sf::Vector2f resolveIconWorldPosition(
		const Event*                               event,
		const std::map<const Node*, sf::Vector2f>& nodeWorldPositions,
		const std::map<const Rail*, RailPath>&     railPaths) const;

	// --- Fog helpers (WEATHER only) ---
	void drawFogOverlay(sf::RenderWindow&      window,
	                    const Event*           event,
	                    const CameraManager&   camera,
	                    const sf::Vector2f&    iconWorldPos,
	                    float                  elapsedSeconds) const;
};

#endif