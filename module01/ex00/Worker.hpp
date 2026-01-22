#ifndef WORKER_HPP
#define WORKER_HPP

#include <vector>
#include <cstddef>
#include <string>

class ATool;
class Workshop;

struct Position
{
	int x;
	int y;
	int z;

	Position(int x = 0, int y = 0, int z = 0) : x(x), y(y), z(z)
	{
	}
};

struct Statistic
{
	int level;
	int exp;

	Statistic(int level = 0, int exp = 0) : level(level), exp(exp)
	{
	}
};

class Worker
{
private:
	std::string name;
	Position coordonnee; //Composition
	Statistic stat; //Composition
	std::vector<ATool*> tools; //Aggregation
	std::vector<Workshop*> workshops; //Association

public:
	Worker(std::string name = "Unnamed");
	Worker(std::string name, Position pos, Statistic st);
	Worker(const Worker& other);
	Worker& operator=(const Worker& other);
	~Worker();

	void takeTool(ATool* tool);
	void releaseTool(ATool* tool);
	
	void registerWorkshop(Workshop* workshop);
	void unregisterWorkshop(Workshop* workshop);
	
	void work();
	
	std::string getName() const;
	
	// GetTool template method
	template<typename ToolType>
	ToolType* GetTool()
	{
		for (std::size_t i = 0; i < tools.size(); ++i)
		{
			ToolType* tool = dynamic_cast<ToolType*>(tools[i]);
			if (tool)
				return tool;
		}
		return 0;
	}
};

#endif