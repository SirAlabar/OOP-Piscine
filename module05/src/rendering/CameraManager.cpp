#include "rendering/CameraManager.hpp"
#include <algorithm>

CameraManager::CameraManager()
	: _offset(700.0f, 180.0f),
	  _currentZoom(1.0f),
	  _targetZoom(1.0f),
	  _minZoom(0.5f),
	  _maxZoom(3.0f),
	  _zoomLerpSpeed(8.0f)
{
}

void CameraManager::setOffset(const sf::Vector2f& offset)
{
	_offset = offset;
}

void CameraManager::moveOffset(const sf::Vector2f& delta)
{
	_offset += delta;
}

sf::Vector2f CameraManager::getOffset() const
{
	return _offset;
}

void CameraManager::setTargetZoom(float zoom)
{
	_targetZoom = clampZoom(zoom);
}

void CameraManager::addZoomDelta(float delta)
{
	_targetZoom = clampZoom(_targetZoom + delta);
}

float CameraManager::getCurrentZoom() const
{
	return _currentZoom;
}

void CameraManager::update(double deltaTime)
{
	float diff = _targetZoom - _currentZoom;
	float step = diff * _zoomLerpSpeed * static_cast<float>(deltaTime);
	
	_currentZoom += step;
	
	if (std::abs(_targetZoom - _currentZoom) < 0.001f)
	{
		_currentZoom = _targetZoom;
	}
}

void CameraManager::setZoomLimits(float minZoom, float maxZoom)
{
	_minZoom = minZoom;
	_maxZoom = maxZoom;
	_currentZoom = clampZoom(_currentZoom);
	_targetZoom = clampZoom(_targetZoom);
}

float CameraManager::clampZoom(float zoom) const
{
	return std::max(_minZoom, std::min(_maxZoom, zoom));
}