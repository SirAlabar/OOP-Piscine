#ifndef WORLDGENERATOR_HPP
#define WORLDGENERATOR_HPP

#include "world/World.hpp"
#include "utils/SeededRNG.hpp"

class WorldGenerator
{
private:
	SeededRNG _rng;
	int _width;
	int _height;

	float perlinNoise(int x, int y, float scale) const;
	float smoothNoise(int x, int y) const;
	float interpolate(float a, float b, float t) const;
	
	void generateHeightMap(World& world);
	void generateTemperatureMap(World& world, std::vector<float>& tempMap);
	void classifyBiomes(World& world, const std::vector<float>& tempMap);
	void applyCellularSmoothing(World& world, int passes);
	void applyAdjacencyRules(World& world);
	BiomeType getMajorityBiome(World& world, int x, int y) const;
	int countNeighborBiome(World& world, int x, int y, BiomeType biome) const;

public:
	WorldGenerator(unsigned int seed, int width, int height);

	void generate(World& world);
};

#endif