#include "world/World.hpp"
#include <stdexcept>

World::World(int width, int height)
	: _width(width),
	  _height(height),
	  _tiles(width * height, BiomeType::Grass),
	  _heightMap(width * height, 0.5f),
	  _railOccupied(width * height, false),
	  _railMask(width * height, 0),
	  _stationOccupied(width * height, false),
	  _tileSprites(width * height, "grass_01.png")
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

void World::setRailConnections(int x, int y, RailDir directions)
{
	if (!isInBounds(x, y))
	{
		return;
	}
	_railMask[getIndex(x, y)] = static_cast<uint8_t>(directions);
}

void World::addRailConnection(int x, int y, RailDir direction)
{
	if (!isInBounds(x, y))
	{
		return;
	}
	RailDir current = static_cast<RailDir>(_railMask[getIndex(x, y)]);
	_railMask[getIndex(x, y)] = static_cast<uint8_t>(current | direction);
}

RailDir World::getRailConnections(int x, int y) const
{
	if (!isInBounds(x, y))
	{
		return RailDir::None;
	}
	return static_cast<RailDir>(_railMask[getIndex(x, y)]);
}

uint8_t World::getRailMask(int x, int y) const
{
	if (!isInBounds(x, y))
	{
		return 0;
	}
	return _railMask[getIndex(x, y)];
}

bool World::isRailOccupied(int x, int y) const
{
	if (!isInBounds(x, y))
	{
		return false;
	}
	return _railOccupied[getIndex(x, y)];
}

void World::markStationOccupied(int x, int y)
{
	if (!isInBounds(x, y))
	{
		return;
	}
	_stationOccupied[getIndex(x, y)] = true;
}

bool World::isStationOccupied(int x, int y) const
{
	if (!isInBounds(x, y))
	{
		return false;
	}
	return _stationOccupied[getIndex(x, y)];
}

bool World::isInBounds(int x, int y) const
{
	return x >= 0 && x < _width && y >= 0 && y < _height;
}

int World::getIndex(int x, int y) const
{
	return y * _width + x;
}

void World::cacheTileSprites(unsigned int seed)
{
	_tileSprites.resize(_width * _height);
	
	for (int y = 0; y < _height; ++y)
	{
		for (int x = 0; x < _width; ++x)
		{
			BiomeType biome = getTile(x, y);
			_tileSprites[getIndex(x, y)] = getBiomeSpriteName(biome, x, y, seed);
		}
	}
}

std::string World::getCachedSprite(int x, int y) const
{
	if (!isInBounds(x, y))
	{
		return "grass_01.png";
	}
	return _tileSprites[getIndex(x, y)];
}

int World::computeSnowPathBitmask(int x, int y) const
{
	int mask = 0;
	
	if (isInBounds(x, y - 1) && getTile(x, y - 1) == BiomeType::Snow)
	{
		mask |= 1;
	}
	if (isInBounds(x + 1, y) && getTile(x + 1, y) == BiomeType::Snow)
	{
		mask |= 2;
	}
	if (isInBounds(x, y + 1) && getTile(x, y + 1) == BiomeType::Snow)
	{
		mask |= 4;
	}
	if (isInBounds(x - 1, y) && getTile(x - 1, y) == BiomeType::Snow)
	{
		mask |= 8;
	}
	
	if (mask == 0)
	{
		mask = 5;
	}
	
	return mask;
}

int World::computeDesertPathBitmask(int x, int y) const
{
	int mask = 0;
	
	if (isInBounds(x, y - 1) && getTile(x, y - 1) == BiomeType::Desert)
	{
		mask |= 1;
	}
	if (isInBounds(x + 1, y) && getTile(x + 1, y) == BiomeType::Desert)
	{
		mask |= 2;
	}
	if (isInBounds(x, y + 1) && getTile(x, y + 1) == BiomeType::Desert)
	{
		mask |= 4;
	}
	if (isInBounds(x - 1, y) && getTile(x - 1, y) == BiomeType::Desert)
	{
		mask |= 8;
	}
	
	if (mask == 0)
	{
		mask = 5;
	}
	
	return mask;
}

int World::computeForestPathBitmask(int x, int y) const
{
	int mask = 0;
	
	if (isInBounds(x, y - 1) && getTile(x, y - 1) == BiomeType::Forest)
	{
		mask |= 1;
	}
	if (isInBounds(x + 1, y) && getTile(x + 1, y) == BiomeType::Forest)
	{
		mask |= 2;
	}
	if (isInBounds(x, y + 1) && getTile(x, y + 1) == BiomeType::Forest)
	{
		mask |= 4;
	}
	if (isInBounds(x - 1, y) && getTile(x - 1, y) == BiomeType::Forest)
	{
		mask |= 8;
	}
	
	if (mask == 0)
	{
		mask = 5;
	}
	
	return mask;
}

std::string World::getBiomeSpriteName(BiomeType biome, int x, int y, unsigned int seed) const
{
	unsigned int hash = static_cast<unsigned int>(x * 374761393U + y * 668265263U + seed * 1274126177U);
	hash = (hash ^ (hash >> 13)) * 1274126177U;
	hash = hash ^ (hash >> 16);
	int variant = static_cast<int>((hash & 0x7fffffff) % 3);
	
	switch (biome)
	{
		case BiomeType::Snow:
		{
			int mask = computeSnowPathBitmask(x, y);
			std::string sprite = "snow_path_" + std::to_string(mask) + ".png";
			return sprite;
		}
			
		case BiomeType::Grass:
		{
			std::string sprite;
			if (variant == 0) sprite = "grass_01.png";
			else if (variant == 1) sprite = "grass_02.png";
			else sprite = "grass_03.png";
			return sprite;
		}
			
		case BiomeType::Forest:
		{
			int mask = computeForestPathBitmask(x, y);
			std::string sprite;
			
			if (mask == 5)
			{
				if (variant == 0) sprite = "forest_path_5.png";
				else if (variant == 1) sprite = "forest_path_5.2.png";
				else sprite = "forest_path_5.3.png";
			}
			else if (mask == 10 || mask == 12 || mask == 14 || mask == 15)
			{
				if (variant == 0) sprite = "forest_path_" + std::to_string(mask) + ".png";
				else sprite = "forest_path_" + std::to_string(mask) + ".2.png";
			}
			else
			{
				sprite = "forest_path_" + std::to_string(mask) + ".png";
			}
			
			return sprite;
		}
			
		case BiomeType::Desert:
		{
			int mask = computeDesertPathBitmask(x, y);
			std::string sprite = "desert_path_" + std::to_string(mask) + ".png";
			return sprite;
		}
			
		case BiomeType::Mountain:
		{
			std::string sprite;
			if (variant == 0) sprite = "mountain_01.png";
			else if (variant == 1) sprite = "mountain_02.png";
			else sprite = "mountain_01.png";
			return sprite;
		}
			
		case BiomeType::Water:
		{
			std::string sprite;
			if (variant == 0) sprite = "water_01.png";
			else if (variant == 1) sprite = "water_02.png";
			else sprite = "water_01.png";
			return sprite;
		}
			
		default:
		{
			return "grass_01.png";
		}
	}
}