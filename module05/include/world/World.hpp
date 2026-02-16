#ifndef WORLD_HPP
#define WORLD_HPP

#include <vector>
#include <string>
#include <cstdint>

enum class BiomeType
{
	Snow = 0,
	Grass = 1,
	Forest = 2,
	Desert = 3,
	Mountain = 4,
	Water = 5
};

enum class RailDir : uint8_t
{
	None  = 0,
	North = 1 << 0,  // 1
	East  = 1 << 1,  // 2
	South = 1 << 2,  // 4
	West  = 1 << 3   // 8
};

// Bitwise operators for RailDir
inline RailDir operator|(RailDir a, RailDir b)
{
	return static_cast<RailDir>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline RailDir operator&(RailDir a, RailDir b)
{
	return static_cast<RailDir>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

inline RailDir& operator|=(RailDir& a, RailDir b)
{
	a = a | b;
	return a;
}

inline uint8_t toMask(RailDir dir)
{
	return static_cast<uint8_t>(dir);
}

class World
{
private:
	int _width;
	int _height;
	std::vector<BiomeType> _tiles;
	std::vector<float> _heightMap;
	std::vector<bool> _railOccupied;
	std::vector<uint8_t> _railMask;  // Explicit rail connectivity using RailDir
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
	void setRailConnections(int x, int y, RailDir directions);
	void addRailConnection(int x, int y, RailDir direction);
	RailDir getRailConnections(int x, int y) const;
	uint8_t getRailMask(int x, int y) const;
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