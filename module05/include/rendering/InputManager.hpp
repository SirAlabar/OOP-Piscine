#ifndef INPUTMANAGER_HPP
#define INPUTMANAGER_HPP

#include <SFML/Graphics.hpp>

struct InputState
{
	bool closeRequested;
	float panX;
	float panY;
	float zoomDelta;
	bool dragActive;
	sf::Vector2i dragDelta;
};

class InputManager
{
private:
	bool _dragging;
	sf::Vector2i _lastMousePos;

public:
	InputManager();

	InputState processEvents(sf::RenderWindow& window, double deltaTime);
	
private:
	void handleMouseWheelScroll(const sf::Event& event, InputState& state);
	void handleMouseButtonPressed(const sf::Event& event, sf::RenderWindow& window);
	void handleMouseButtonReleased(const sf::Event& event);
	void handleMouseMoved(const sf::Event& event, sf::RenderWindow& window, InputState& state);
	void handleKeyboardInput(double deltaTime, InputState& state);
};

#endif