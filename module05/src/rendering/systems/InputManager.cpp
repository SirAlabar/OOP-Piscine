#include "rendering/systems/InputManager.hpp"

InputManager::InputManager()
    : _dragging(false),
      _lastMousePos(0, 0)
{
}

InputState InputManager::processEvents(sf::RenderWindow& window, double deltaTime)
{
    InputState state      = {};
    state.closeRequested  = false;
    state.panX            = 0.0f;
    state.panY            = 0.0f;
    state.zoomDelta       = 0.0f;
    state.dragActive      = false;
    state.dragDelta       = sf::Vector2i(0, 0);
    state.speedMultiplier = 1.0;

    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            state.closeRequested = true;
        }
        else if (event.type == sf::Event::KeyPressed)
        {
            handleKeyPressed(event, state);
        }
        else if (event.type == sf::Event::MouseWheelScrolled)
        {
            handleMouseWheelScroll(event, state);
        }
        else if (event.type == sf::Event::MouseButtonPressed)
        {
            handleMouseButtonPressed(event, window);
        }
        else if (event.type == sf::Event::MouseButtonReleased)
        {
            handleMouseButtonReleased(event);
        }
        else if (event.type == sf::Event::MouseMoved)
        {
            handleMouseMoved(event, window, state);
        }
    }

    handleKeyboardInput(deltaTime, state);

    return state;
}

void InputManager::handleMouseWheelScroll(const sf::Event& event, InputState& state)
{
    if      (event.mouseWheelScroll.delta > 0.0f) 
    { 
        state.zoomDelta += 0.10f; 
    }
    else if (event.mouseWheelScroll.delta < 0.0f) 
    { 
        state.zoomDelta -= 0.10f; 
    }
}

void InputManager::handleKeyPressed(const sf::Event& event, InputState& state)
{
    // Speed control
    if (event.key.code == sf::Keyboard::Add || event.key.code == sf::Keyboard::Equal)
    {
        state.speedMultiplier = 2.0;
    }
    else if (event.key.code == sf::Keyboard::Subtract || event.key.code == sf::Keyboard::Hyphen)
    {
        state.speedMultiplier = 0.5;
    }
}

void InputManager::handleMouseButtonPressed(const sf::Event& event, sf::RenderWindow& window)
{
    if (event.mouseButton.button == sf::Mouse::Middle)
    {
        _dragging     = true;
        _lastMousePos = sf::Mouse::getPosition(window);
    }
}

void InputManager::handleMouseButtonReleased(const sf::Event& event)
{
    if (event.mouseButton.button == sf::Mouse::Middle)
    {
        _dragging = false;
    }
}

void InputManager::handleMouseMoved(const sf::Event&, sf::RenderWindow& window, InputState& state)
{
    if (_dragging)
    {
        sf::Vector2i currentPos = sf::Mouse::getPosition(window);
        state.dragDelta  = currentPos - _lastMousePos;
        state.dragActive = true;
        _lastMousePos    = currentPos;
    }
}

void InputManager::handleKeyboardInput(double deltaTime, InputState& state)
{
    const float speed = static_cast<float>(420.0 * deltaTime);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
    {
        state.panY += speed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
    {
        state.panY -= speed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
    {
        state.panX += speed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
    {
        state.panX -= speed;
    }
}