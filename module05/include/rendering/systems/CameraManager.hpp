#ifndef CAMERAMANAGER_HPP
#define CAMERAMANAGER_HPP

#include <SFML/Graphics.hpp>

class CameraManager
{
private:
	sf::Vector2f _offset;
	float _currentZoom;
	float _targetZoom;
	float _minZoom;
	float _maxZoom;
	float _zoomLerpSpeed;

public:
	CameraManager();

	void setOffset(const sf::Vector2f& offset);
	void moveOffset(const sf::Vector2f& delta);
	sf::Vector2f getOffset() const;

	void setTargetZoom(float zoom);
	void addZoomDelta(float delta);
	float getCurrentZoom() const;

	void update(double deltaTime);

	void setZoomLimits(float minZoom, float maxZoom);

private:
	float clampZoom(float zoom) const;
};

#endif