#ifndef HUDRENDERER_HPP
#define HUDRENDERER_HPP

#include <SFML/Graphics.hpp>

// Draws the simulation clock and speed multiplier label.
class HudRenderer
{
private:
    sf::Font _font;
    bool     _fontLoaded;

public:
    HudRenderer();

    void draw(sf::RenderWindow& window, double currentTimeSeconds, double simulationSpeed) const;

    bool isFontLoaded() const;
};

#endif