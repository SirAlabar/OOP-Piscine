#include "world/World.hpp"
#include <stdexcept>

World::World(int width, int height)
	: _width(width),
	  _height(height),
	  _tiles(width * height, BiomeType::Grass),
	  _heightMap(width * height, 0.5f),
	  _railOccupied(width * height, false)
{
}

void World::setTile(int x, int y, BiomeType biome)
{
	if (!isInBounds(x, y))
	{
		return;
	}
	_tiles[getIndex(x, y)] = biome;
}

BiomeType World::getTile(int x, int y) const
{
	if (!isInBounds(x, y))
	{
		return BiomeType::Grass;
	}
	return _tiles[getIndex(x, y)];
}

void World::setHeight(int x, int y, float height)
{
	if (!isInBounds(x, y))
	{
		return;
	}
	_heightMap[getIndex(x, y)] = height;
}

float World::getHeight(int x, int y) const
{
	if (!isInBounds(x, y))
	{
		return 0.5f;
	}
	return _heightMap[getIndex(x, y)];
}

void World::markRailOccupied(int x, int y)
{
	if (!isInBounds(x, y))
	{
		return;
	}
	_railOccupied[getIndex(x, y)] = true;
}

bool World::isRailOccupied(int x, int y) const
{
	if (!isInBounds(x, y))
	{
		return false;
	}
	return _railOccupied[getIndex(x, y)];
}

bool World::isInBounds(int x, int y) const
{
	return x >= 0 && x < _width && y >= 0 && y < _height;
}

int World::getIndex(int x, int y) const
{
	return y * _width + x;
}

std::string World::getBiomeSpriteName(BiomeType biome, int x, int y, unsigned int seed) const
{
	unsigned int hash = static_cast<unsigned int>(x + y * 57 + seed * 131);
	hash = (hash << 13) ^ hash;
	int variant = static_cast<int>((hash & 0x7fffffff) % 3);
	
	switch (biome)
	{
		case BiomeType::Snow:
			return "snow.png";
			
		case BiomeType::Grass:
			if (variant == 0) return "grass_01.png";
			else if (variant == 1) return "grass_02.png";
			else return "grass_03.png";
			
		case BiomeType::Forest:
			if (variant == 0) return "forest1.png";
			else if (variant == 1) return "forest3.png";
			else return "forest4.png";
			
		case BiomeType::Desert:
			return "desert.png";
			
		case BiomeType::Mountain:
			return (variant == 0) ? "mountain_01.png" : "mountain_02.png";
			
		case BiomeType::Water:
			return (variant == 0) ? "water_01.png" : "water_02.png";
			
		default:
			return "grass_01.png";
	}
}