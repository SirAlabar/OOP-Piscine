#include "world/WorldGenerator.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>

WorldGenerator::WorldGenerator(unsigned int seed, int width, int height)
	: _rng(seed),
	  _width(width),
	  _height(height)
{
	std::cout << "[WorldGenerator] Initialized with seed: " << seed 
	          << " for " << width << "x" << height << " world" << std::endl;
}

void WorldGenerator::generate(World& world)
{
	std::cout << "[WorldGenerator] Starting generation with seed: " << _rng.getSeed() << std::endl;
	
	generateHeightMap(world);
	
	std::vector<float> tempMap(_width * _height, 0.5f);
	generateTemperatureMap(world, tempMap);
	
	classifyBiomes(world, tempMap);
	
	std::cout << "[WorldGenerator] Biome classification complete" << std::endl;
	
	applyCellularSmoothing(world, 3);
	
	std::cout << "[WorldGenerator] Cellular smoothing complete" << std::endl;
	
	applyAdjacencyRules(world);
	
	std::cout << "[WorldGenerator] World generation complete!" << std::endl;
}

float WorldGenerator::perlinNoise(int x, int y, float scale) const
{
	float nx = static_cast<float>(x) / scale;
	float ny = static_cast<float>(y) / scale;
	
	int x0 = static_cast<int>(std::floor(nx));
	int y0 = static_cast<int>(std::floor(ny));
	int x1 = x0 + 1;
	int y1 = y0 + 1;
	
	float sx = nx - static_cast<float>(x0);
	float sy = ny - static_cast<float>(y0);
	
	float n00 = smoothNoise(x0, y0);
	float n10 = smoothNoise(x1, y0);
	float n01 = smoothNoise(x0, y1);
	float n11 = smoothNoise(x1, y1);
	
	float nx0 = interpolate(n00, n10, sx);
	float nx1 = interpolate(n01, n11, sx);
	
	return interpolate(nx0, nx1, sy);
}

float WorldGenerator::smoothNoise(int x, int y) const
{
	unsigned int seed = _rng.getSeed();
	
	unsigned int h = seed;
	h ^= static_cast<unsigned int>(x) * 374761393U;
	h ^= static_cast<unsigned int>(y) * 668265263U;
	h = (h ^ (h >> 13)) * 1274126177U;
	h = h ^ (h >> 16);
	
	float normalized = static_cast<float>(h & 0x7FFFFFFF) / 2147483647.0f;
	return normalized * 2.0f - 1.0f;
}

float WorldGenerator::interpolate(float a, float b, float t) const
{
	float ft = t * 3.1415927f;
	float f = (1.0f - std::cos(ft)) * 0.5f;
	return a * (1.0f - f) + b * f;
}

void WorldGenerator::generateHeightMap(World& world)
{
	for (int y = 0; y < _height; ++y)
	{
		for (int x = 0; x < _width; ++x)
		{
			float height = 0.0f;
			height += perlinNoise(x, y, 16.0f) * 0.5f;
			height += perlinNoise(x, y, 8.0f) * 0.25f;
			height += perlinNoise(x, y, 4.0f) * 0.125f;
			height += perlinNoise(x, y, 2.0f) * 0.0625f;
			
			height = (height + 1.0f) * 0.5f;
			height = std::max(0.0f, std::min(1.0f, height));
			
			world.setHeight(x, y, height);
		}
	}
	
	std::cout << "[WorldGenerator] Sample heights: [0,0]=" << world.getHeight(0, 0) 
	          << " [10,10]=" << world.getHeight(10, 10)
	          << " [20,20]=" << world.getHeight(20, 20) << std::endl;
}

void WorldGenerator::generateTemperatureMap(World& world, std::vector<float>& tempMap)
{
	for (int y = 0; y < _height; ++y)
	{
		for (int x = 0; x < _width; ++x)
		{
			float temp = 0.0f;
			temp += perlinNoise(x + 1000, y + 1000, 24.0f) * 0.6f;
			temp += perlinNoise(x + 1000, y + 1000, 12.0f) * 0.3f;
			temp += perlinNoise(x + 1000, y + 1000, 6.0f) * 0.1f;
			
			temp = (temp + 1.0f) * 0.5f;
			temp = std::max(0.0f, std::min(1.0f, temp));
			
			float height = world.getHeight(x, y);
			temp -= (height - 0.5f) * 0.4f;
			temp = std::max(0.0f, std::min(1.0f, temp));
			
			tempMap[y * _width + x] = temp;
		}
	}
}

void WorldGenerator::classifyBiomes(World& world, const std::vector<float>& tempMap)
{
	int biomeCounts[6] = {0};
	
	for (int y = 0; y < _height; ++y)
	{
		for (int x = 0; x < _width; ++x)
		{
			if (world.isRailOccupied(x, y))
			{
				continue;
			}
			
			float height = world.getHeight(x, y);
			float temp = tempMap[y * _width + x];
			
			BiomeType biome;
			
			if (height < 0.30f)
			{
				biome = BiomeType::Water;
			}
			else if (height > 0.70f)
			{
				biome = BiomeType::Mountain;
			}
			else if (temp < 0.25f)
			{
				biome = BiomeType::Snow;
			}
			else if (temp < 0.40f)
			{
				biome = BiomeType::Grass;
			}
			else if (temp < 0.65f)
			{
				biome = BiomeType::Forest;
			}
			else
			{
				biome = BiomeType::Desert;
			}
			
			world.setTile(x, y, biome);
			biomeCounts[static_cast<int>(biome)]++;
		}
	}
	
	std::cout << "[WorldGenerator] Biome distribution:" << std::endl;
	std::cout << "  Snow: " << biomeCounts[0] << " tiles" << std::endl;
	std::cout << "  Grass: " << biomeCounts[1] << " tiles" << std::endl;
	std::cout << "  Forest: " << biomeCounts[2] << " tiles" << std::endl;
	std::cout << "  Desert: " << biomeCounts[3] << " tiles" << std::endl;
	std::cout << "  Mountain: " << biomeCounts[4] << " tiles" << std::endl;
	std::cout << "  Water: " << biomeCounts[5] << " tiles" << std::endl;
}

void WorldGenerator::applyCellularSmoothing(World& world, int passes)
{
	for (int pass = 0; pass < passes; ++pass)
	{
		std::vector<BiomeType> nextGen(_width * _height);
		
		for (int y = 0; y < _height; ++y)
		{
			for (int x = 0; x < _width; ++x)
			{
				if (world.isRailOccupied(x, y))
				{
					nextGen[y * _width + x] = world.getTile(x, y);
					continue;
				}
				
				BiomeType majority = getMajorityBiome(world, x, y);
				nextGen[y * _width + x] = majority;
			}
		}
		
		for (int y = 0; y < _height; ++y)
		{
			for (int x = 0; x < _width; ++x)
			{
				world.setTile(x, y, nextGen[y * _width + x]);
			}
		}
	}
}

void WorldGenerator::applyAdjacencyRules(World& world)
{
	std::vector<BiomeType> adjusted(_width * _height);
	
	for (int y = 0; y < _height; ++y)
	{
		for (int x = 0; x < _width; ++x)
		{
			adjusted[y * _width + x] = world.getTile(x, y);
			
			if (world.isRailOccupied(x, y))
			{
				continue;
			}
			
			BiomeType current = world.getTile(x, y);
			
			if (current == BiomeType::Desert)
			{
				int snowCount = countNeighborBiome(world, x, y, BiomeType::Snow);
				if (snowCount > 0)
				{
					adjusted[y * _width + x] = BiomeType::Grass;
				}
			}
			else if (current == BiomeType::Forest)
			{
				int grassCount = countNeighborBiome(world, x, y, BiomeType::Grass);
				int forestCount = countNeighborBiome(world, x, y, BiomeType::Forest);
				if (grassCount + forestCount < 3)
				{
					adjusted[y * _width + x] = BiomeType::Grass;
				}
			}
		}
	}
	
	for (int y = 0; y < _height; ++y)
	{
		for (int x = 0; x < _width; ++x)
		{
			world.setTile(x, y, adjusted[y * _width + x]);
		}
	}
}

BiomeType WorldGenerator::getMajorityBiome(World& world, int x, int y) const
{
	int counts[6] = {0};
	
	BiomeType center = world.getTile(x, y);
	counts[static_cast<int>(center)]++;
	
	for (int dy = -1; dy <= 1; ++dy)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			if (dx == 0 && dy == 0)
			{
				continue;
			}
			
			int nx = x + dx;
			int ny = y + dy;
			
			if (world.isInBounds(nx, ny))
			{
				BiomeType neighbor = world.getTile(nx, ny);
				counts[static_cast<int>(neighbor)]++;
			}
		}
	}
	
	int maxCount = 0;
	BiomeType majority = center;
	for (int i = 0; i < 6; ++i)
	{
		if (counts[i] > maxCount)
		{
			maxCount = counts[i];
			majority = static_cast<BiomeType>(i);
		}
	}
	
	return majority;
}

int WorldGenerator::countNeighborBiome(World& world, int x, int y, BiomeType biome) const
{
	int count = 0;
	
	for (int dy = -1; dy <= 1; ++dy)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			if (dx == 0 && dy == 0)
			{
				continue;
			}
			
			int nx = x + dx;
			int ny = y + dy;
			
			if (world.isInBounds(nx, ny) && world.getTile(nx, ny) == biome)
			{
				count++;
			}
		}
	}
	
	return count;
}