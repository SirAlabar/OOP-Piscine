#include "patterns/events/Event.hpp"

Event::Event(EventType type, const Time& startTime, const Time& duration)
	: _type(type), _startTime(startTime), _duration(duration), _isActive(false)
{
}

bool Event::shouldBeActive(const Time& currentTime) const
{
	Time endTime = _startTime + _duration;
	return currentTime >= _startTime && currentTime < endTime;
}

void Event::update(const Time& currentTime)
{
	bool shouldActive = shouldBeActive(currentTime);

	// Activate when entering time window
	if (shouldActive && !_isActive)
	{
		activate();
		_isActive = true;
	}
	// Deactivate when leaving time window
	else if (!shouldActive && _isActive)
	{
		deactivate();
		_isActive = false;
	}
}

EventType Event::getType() const
{
	return _type;
}

Time Event::getStartTime() const
{
	return _startTime;
}

Time Event::getDuration() const
{
	return _duration;
}

Time Event::getEndTime() const
{
	return _startTime + _duration;
}

bool Event::isActive() const
{
	return _isActive;
}

const VisualData& Event::getVisualData() const
{
	return _visualData;
}