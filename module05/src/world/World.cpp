#include "world/World.hpp"
#include <stdexcept>

World::World(int width, int height)
	: _width(width),
	  _height(height),
	  _tiles(width * height, BiomeType::Grass),
	  _heightMap(width * height, 0.5f),
	  _railOccupied(width * height, false),
	  _stationOccupied(width * height, false)
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

int World::computeSnowPathBitmask(int x, int y) const
{
	int mask = 0;
	
	// North
	if (isInBounds(x, y - 1) && getTile(x, y - 1) == BiomeType::Snow)
	{
		mask |= 1;
	}
	// East
	if (isInBounds(x + 1, y) && getTile(x + 1, y) == BiomeType::Snow)
	{
		mask |= 2;
	}
	// South
	if (isInBounds(x, y + 1) && getTile(x, y + 1) == BiomeType::Snow)
	{
		mask |= 4;
	}
	// West
	if (isInBounds(x - 1, y) && getTile(x - 1, y) == BiomeType::Snow)
	{
		mask |= 8;
	}
	
	if (mask == 0)
	{
		mask = 5; // Default vertical
	}
	
	return mask;
}

int World::computeDesertPathBitmask(int x, int y) const
{
	int mask = 0;
	
	// North
	if (isInBounds(x, y - 1) && getTile(x, y - 1) == BiomeType::Desert)
	{
		mask |= 1;
	}
	// East
	if (isInBounds(x + 1, y) && getTile(x + 1, y) == BiomeType::Desert)
	{
		mask |= 2;
	}
	// South
	if (isInBounds(x, y + 1) && getTile(x, y + 1) == BiomeType::Desert)
	{
		mask |= 4;
	}
	// West
	if (isInBounds(x - 1, y) && getTile(x - 1, y) == BiomeType::Desert)
	{
		mask |= 8;
	}
	
	if (mask == 0)
	{
		mask = 5; // Default vertical
	}
	
	return mask;
}

int World::computeForestPathBitmask(int x, int y) const
{
	int mask = 0;
	
	// North
	if (isInBounds(x, y - 1) && getTile(x, y - 1) == BiomeType::Forest)
	{
		mask |= 1;
	}
	// East
	if (isInBounds(x + 1, y) && getTile(x + 1, y) == BiomeType::Forest)
	{
		mask |= 2;
	}
	// South
	if (isInBounds(x, y + 1) && getTile(x, y + 1) == BiomeType::Forest)
	{
		mask |= 4;
	}
	// West
	if (isInBounds(x - 1, y) && getTile(x - 1, y) == BiomeType::Forest)
	{
		mask |= 8;
	}
	
	if (mask == 0)
	{
		mask = 5; // Default vertical
	}
	
	return mask;
}

std::string World::getBiomeSpriteName(BiomeType biome, int x, int y, unsigned int seed) const
{
	unsigned int hash = static_cast<unsigned int>(x + y * 57 + seed * 131);
	hash = (hash << 13) ^ hash;
	int variant = static_cast<int>((hash & 0x7fffffff) % 3);
	
	switch (biome)
	{
		case BiomeType::Snow:
		{
			int mask = computeSnowPathBitmask(x, y);
			return "snow_path_" + std::to_string(mask) + ".png";
		}
			
		case BiomeType::Grass:
			if (variant == 0) return "grass_01.png";
			else if (variant == 1) return "grass_02.png";
			else return "grass_03.png";
			
		case BiomeType::Forest:
		{
			int mask = computeForestPathBitmask(x, y);
			return "forest_path_" + std::to_string(mask) + ".png";
		}
			
		case BiomeType::Desert:
		{
			int mask = computeDesertPathBitmask(x, y);
			return "desert_path_" + std::to_string(mask) + ".png";
		}
			
		case BiomeType::Mountain:
			if (variant == 0) return "mountain_01.png";
			else if (variant == 1) return "mountain_02.png";
			else return "mountain_01.png";
			
		case BiomeType::Water:
			if (variant == 0) return "water_01.png";
			else if (variant == 1) return "water_02.png";
			else return "water_01.png";
			
		default:
			return "grass_01.png";
	}
}