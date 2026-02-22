#include "rendering/HudRenderer.hpp"
#include "simulation/SimConstants.hpp"
#include <sstream>
#include <iomanip>
#include <stdexcept>

HudRenderer::HudRenderer() : _fontLoaded(false)
{
    _fontLoaded = _font.loadFromFile("assets/Minecraft.ttf");

    if (!_fontLoaded)
    {
        throw std::runtime_error("Failed to load UI font: assets/Minecraft.ttf");
    }
}

bool HudRenderer::isFontLoaded() const
{
    return _fontLoaded;
}

void HudRenderer::draw(sf::RenderWindow& window, double currentTimeSeconds, double simulationSpeed) const
{
    if (!_fontLoaded)
    {
        return;
    }

    int totalMinutes = static_cast<int>(currentTimeSeconds / SimConfig::SECONDS_PER_MINUTE);
    int cycleMinutes = totalMinutes % SimConfig::MINUTES_PER_DAY;
    int hours        = cycleMinutes / 60;
    int minutes      = cycleMinutes % 60;

    std::ostringstream timeStream;
    timeStream << std::setfill('0') << std::setw(2) << hours << "h" << std::setw(2) << minutes;

    sf::Text timeLabel;
    timeLabel.setFont(_font);
    timeLabel.setString(timeStream.str());
    timeLabel.setCharacterSize(24);
    timeLabel.setFillColor(sf::Color::White);
    timeLabel.setOutlineColor(sf::Color::Black);
    timeLabel.setOutlineThickness(2.0f);

    sf::FloatRect timeBounds = timeLabel.getLocalBounds();
    timeLabel.setOrigin(timeBounds.width / 2.0f, 0);
    timeLabel.setPosition(window.getSize().x / 2.0f, 10.0f);
    window.draw(timeLabel);

    std::ostringstream speedStream;
    speedStream << std::fixed << std::setprecision(1) << simulationSpeed << "x";

    sf::Text speedLabel;
    speedLabel.setFont(_font);
    speedLabel.setString(speedStream.str());
    speedLabel.setCharacterSize(18);
    speedLabel.setFillColor(sf::Color(150, 255, 150));
    speedLabel.setOutlineColor(sf::Color::Black);
    speedLabel.setOutlineThickness(1.5f);

    sf::FloatRect speedBounds = speedLabel.getLocalBounds();
    speedLabel.setOrigin(speedBounds.width / 2.0f, 0);
    speedLabel.setPosition(window.getSize().x / 2.0f, 40.0f);
    window.draw(speedLabel);
}