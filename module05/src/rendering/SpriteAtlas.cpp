#include "rendering/SpriteAtlas.hpp"
#include <fstream>
#include <regex>
#include <sstream>
#include <stdexcept>

bool SpriteAtlas::loadFromFiles(const std::string& texturePath, const std::string& jsonPath)
{
	_frames.clear();
	if (!_texture.loadFromFile(texturePath))
	{
		return false;
	}

	std::ifstream in(jsonPath.c_str());
	if (!in.is_open())
	{
		return false;
	}

	std::stringstream buffer;
	buffer << in.rdbuf();
	const std::string data = buffer.str();

	const std::regex framePattern(
		R"REGEX("([^"]+\.png)"\s*:\s*\{\s*"frame"\s*:\s*\{\s*"x"\s*:\s*(\d+)\s*,\s*"y"\s*:\s*(\d+)\s*,\s*"w"\s*:\s*(\d+)\s*,\s*"h"\s*:\s*(\d+))REGEX"
	);

	for (std::sregex_iterator i(data.begin(), data.end(), framePattern); i != std::sregex_iterator(); ++i)
	{
		const std::smatch m = *i;
		const std::string name = m[1].str();
		const int x = std::stoi(m[2].str());
		const int y = std::stoi(m[3].str());
		const int w = std::stoi(m[4].str());
		const int h = std::stoi(m[5].str());
		_frames[name] = sf::IntRect(x, y, w, h);
	}

	return !_frames.empty();
}

bool SpriteAtlas::hasFrame(const std::string& frameName) const
{
	return _frames.find(frameName) != _frames.end();
}

sf::IntRect SpriteAtlas::getFrame(const std::string& frameName) const
{
	std::map<std::string, sf::IntRect>::const_iterator it = _frames.find(frameName);
	if (it == _frames.end())
	{
		throw std::runtime_error("Missing atlas frame: " + frameName);
	}
	return it->second;
}

const sf::Texture& SpriteAtlas::getTexture() const
{
	return _texture;
}
