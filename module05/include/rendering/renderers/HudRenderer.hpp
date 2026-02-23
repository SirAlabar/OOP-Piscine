#ifndef HUDRENDERER_HPP
#define HUDRENDERER_HPP

#include <SFML/Graphics.hpp>

// Draws the simulation clock and speed multiplier label.
class HudRenderer
{
private:
    sf::Font _font;
    bool     _fontLoaded;

    void drawLabel(sf::RenderWindow& window, const std::string& text,
                   unsigned int charSize, const sf::Color& fillColor,
                   float outlineThickness, const sf::Vector2f& position) const;

public:
    HudRenderer();

    void draw(sf::RenderWindow& window, double currentTimeSeconds, double simulationSpeed) const;
    void drawCityLabel(sf::RenderWindow& window, const std::string& name, const sf::Vector2f& position, float zoom) const;
    void drawTrainLabel(sf::RenderWindow& window, const std::string& name, const sf::Vector2f& position, float zoom) const;

    bool isFontLoaded() const;
};

#endif