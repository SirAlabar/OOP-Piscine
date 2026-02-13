#ifndef SPRITEATLAS_HPP
#define SPRITEATLAS_HPP

#include <SFML/Graphics.hpp>
#include <map>
#include <string>

class SpriteAtlas
{
private:
	sf::Texture _texture;
	std::map<std::string, sf::IntRect> _frames;

public:
	bool loadFromFiles(const std::string& texturePath, const std::string& jsonPath);
	bool hasFrame(const std::string& frameName) const;
	sf::IntRect getFrame(const std::string& frameName) const;
	const sf::Texture& getTexture() const;
};

#endif