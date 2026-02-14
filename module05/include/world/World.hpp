#ifndef WORLD_HPP
#define WORLD_HPP

#include <vector>
#include <string>

enum class BiomeType
{
	Snow = 0,
	Grass = 1,
	Forest = 2,
	Desert = 3,
	Mountain = 4,
	Water = 5
};

class World
{
private:
	int _width;
	int _height;
	std::vector<BiomeType> _tiles;
	std::vector<float> _heightMap;
	std::vector<bool> _railOccupied;
	std::vector<bool> _stationOccupied;
	std::vector<std::string> _tileSprites;


	int getIndex(int x, int y) const;
	int computeSnowPathBitmask(int x, int y) const;
	int computeDesertPathBitmask(int x, int y) const;
	int computeForestPathBitmask(int x, int y) const;
	
	std::string getBiomeSpriteName(BiomeType biome, int x, int y, unsigned int seed) const;

public:
	World(int width, int height);

	void setTile(int x, int y, BiomeType biome);
	BiomeType getTile(int x, int y) const;

	void setHeight(int x, int y, float height);
	float getHeight(int x, int y) const;

	void markRailOccupied(int x, int y);
	bool isRailOccupied(int x, int y) const;

	void markStationOccupied(int x, int y);
	bool isStationOccupied(int x, int y) const;

	bool isInBounds(int x, int y) const;
	int getWidth() const { return _width; }
	int getHeight() const { return _height; }

	void cacheTileSprites(unsigned int seed);
	std::string getCachedSprite(int x, int y) const;
};

#endif