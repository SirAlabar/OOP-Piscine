#ifndef ISOMETRICUTILS_HPP
#define ISOMETRICUTILS_HPP

#include <SFML/Graphics.hpp>
#include "rendering/systems/CameraManager.hpp"

namespace IsometricUtils
{
	static constexpr float BASE_TILE_WIDTH  = 48.0f;
	static constexpr float BASE_TILE_HEIGHT = 24.0f;

	inline sf::Vector2f project(const sf::Vector2f& worldPoint, const CameraManager& camera)
	{
		const float zoom   = camera.getCurrentZoom();
		const sf::Vector2f offset = camera.getOffset();

		const float isoX = (worldPoint.x - worldPoint.y) * (BASE_TILE_WIDTH  / 2.0f) * zoom;
		const float isoY = (worldPoint.x + worldPoint.y) * (BASE_TILE_HEIGHT / 2.0f) * zoom;

		return sf::Vector2f(isoX + offset.x, isoY + offset.y);
	}
}

#endif