#include "rendering/EventRenderer.hpp"
#include "rendering/SpriteAtlas.hpp"
#include "rendering/CameraManager.hpp"
#include "utils/IsometricUtils.hpp"
#include "patterns/events/Event.hpp"
#include "patterns/events/WeatherEvent.hpp"
#include "core/Rail.hpp"
#include "core/Node.hpp"
#include "rendering/RenderTypes.hpp"

#include <array>
#include <cmath>
#include <algorithm>
#include <limits>


void EventRenderer::draw(sf::RenderWindow&                          window,
                         const SpriteAtlas&                         eventsAtlas,
                         const CameraManager&                       camera,
                         const std::map<const Node*, sf::Vector2f>& nodeWorldPositions,
                         const std::map<const Rail*, RailPath>&     railPaths,
                         float                                      elapsedSeconds,
                         const std::vector<Event*>&                 activeEvents)
{
	const float zoom = camera.getCurrentZoom();

	for (const Event* event : activeEvents)
	{
		if (!event || !event->isActive())
		{
			continue;
		}

		// Calculate icon world position once for both icon and fog
		const sf::Vector2f worldPos =
			resolveIconWorldPosition(event, nodeWorldPositions, railPaths);

		// worldPos == (0,0) means we could not resolve a position — skip
		if (worldPos.x == 0.0f && worldPos.y == 0.0f)
		{
			continue;
		}

		// --- Fog layer first so icon renders on top ---
		if (event->getType() == EventType::WEATHER)
		{
			drawFogOverlay(window, event, camera,
			               worldPos, elapsedSeconds);
		}

		// --- Icon ---
		const std::string frameName = resolveIconFrame(event);
		if (frameName.empty() || !eventsAtlas.hasFrame(frameName))
		{
			continue;
		}

		const sf::Vector2f screenPos = IsometricUtils::project(worldPos, camera);
		drawEventIcon(window, eventsAtlas, frameName, screenPos, zoom);
	}
}

void EventRenderer::drawEventIcon(sf::RenderWindow&   window,
                                  const SpriteAtlas&  eventsAtlas,
                                  const std::string&  frameName,
                                  const sf::Vector2f& screenPos,
                                  float               zoom) const
{
	sf::Sprite sprite;
	sprite.setTexture(eventsAtlas.getTexture());
	sprite.setTextureRect(eventsAtlas.getFrame(frameName));

	const sf::IntRect rect = eventsAtlas.getFrame(frameName);
	sprite.setOrigin(rect.width / 2.0f, rect.height / 2.0f);

	// Icons are larger than tiles — scale them relative to tile zoom but
	// clamp so they don't shrink to invisibility at low zoom.
	const float iconScale = std::max(0.6f, zoom) * 0.8f;
	sprite.setScale(iconScale, iconScale);
	sprite.setPosition(screenPos.x, screenPos.y - 30.0f * zoom);

	// Apply 50% opacity
	sprite.setColor(sf::Color(255, 255, 255, 128));

	window.draw(sprite);
}

std::string EventRenderer::resolveIconFrame(const Event* event) const
{
	switch (event->getType())
	{
		case EventType::TRACK_MAINTENANCE: return "maintence_event.png";
		case EventType::SIGNAL_FAILURE:    return "signal_failure_event.png";
		case EventType::STATION_DELAY:     return "station_delay_event.png";
		case EventType::WEATHER:           return "storm_event.png";
		default:                           return "";
	}
}

sf::Vector2f EventRenderer::resolveIconWorldPosition(
	const Event*                               event,
	const std::map<const Node*, sf::Vector2f>& nodeWorldPositions,
	const std::map<const Rail*, RailPath>&     railPaths) const
{
	const Rail* rail = event->getAnchorRail();
	const Node* node = event->getAnchorNode();

	if (rail)
	{
		const auto it = railPaths.find(rail);
		if (it == railPaths.end())
		{
			return {};
		}
		return (it->second.start + it->second.end) * 0.5f;
	}

	if (node)
	{
		const auto it = nodeWorldPositions.find(node);
		if (it == nodeWorldPositions.end())
		{
			return {};
		}

		// Weather events use their center node but get a visual offset
		if (event->getType() == EventType::WEATHER)
		{
			return sf::Vector2f(it->second.x + 2.0f, it->second.y - 2.0f);
		}

		return it->second;
	}

	return {};
}


void EventRenderer::drawFogOverlay(
	sf::RenderWindow&      window,
	const Event*           event,
	const CameraManager&   camera,
	const sf::Vector2f&    iconWorldPos,
	float                  elapsedSeconds) const
{
	// Center fog on the icon position (same origin as the weather icon)
	const sf::Vector2f center = IsometricUtils::project(iconWorldPos, camera);

	// Get the event's actual radius in km
	const auto* weather = static_cast<const WeatherEvent*>(event);
	const double radiusKm = weather->getRadiusKm();

	// Convert km to screen pixels
	// Scale factor: 1 km ≈ 15 pixels at base zoom (adjust for visual match)
	// This makes fog radius match the game logic radius
	const float zoom = camera.getCurrentZoom();
	const float radiusPixels = static_cast<float>(radiusKm * 15.0 * zoom);

	// Pulsing alpha: oscillates between 55 and 100 over ~3 seconds
	const float pulse = std::sin(elapsedSeconds * 2.0f * 3.14159f / 3.0f) * 0.5f + 0.5f;
	const sf::Uint8 alpha = static_cast<sf::Uint8>(55.0f + pulse * 45.0f);

	// Isometric ellipse: screen width ≈ 2× height for the isometric projection
	// We draw 3 layered circles with scaling to fake an ellipse and give depth.
	const std::array<float, 3> radiusFactors = { 1.0f, 0.75f, 0.5f };
	const std::array<sf::Uint8, 3> alphaFactors = { alpha,
	                                                  static_cast<sf::Uint8>(alpha * 2 / 3),
	                                                  static_cast<sf::Uint8>(alpha / 3) };

	for (std::size_t i = 0; i < radiusFactors.size(); ++i)
	{
		const float r = radiusPixels * radiusFactors[i];

		sf::CircleShape circle(r);
		circle.setOrigin(r, r);
		circle.setPosition(center);
		// Isometric vertical squish: y-scale ≈ 0.5
		circle.setScale(1.0f, 0.5f);
		circle.setFillColor(sf::Color(200, 210, 220, alphaFactors[i]));
		circle.setOutlineThickness(0.0f);

		window.draw(circle);
	}
}