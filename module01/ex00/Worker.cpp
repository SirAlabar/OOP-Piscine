#include "Worker.hpp"
#include "ATool.hpp"
#include "Workshop.hpp"
#include <iostream>
#include <algorithm>
#include <cstddef>

#define BLUE "\033[34m"
#define RESET "\033[0m"

Worker::Worker(std::string name) : name(name), coordonnee(), stat()
{
	std::cout << BLUE << "Worker '" << name << "' created" << RESET << std::endl;
}

Worker::Worker(std::string name, Position pos, Statistic st) : name(name), coordonnee(pos), stat(st)
{
	std::cout << BLUE << "Worker '" << name << "' created at position (" << pos.x << "," << pos.y << "," << pos.z << ")" << RESET << std::endl;
}

Worker::Worker(const Worker& other) : name(other.name), coordonnee(other.coordonnee), stat(other.stat)
{
	std::cout << BLUE << "Worker '" << name << "' copied" << RESET << std::endl;
}

Worker& Worker::operator=(const Worker& other)
{
	if (this != &other)
	{
		name = other.name;
		coordonnee = other.coordonnee;
		stat = other.stat;
	}
	return *this;
}

Worker::~Worker()
{
    std::cout << BLUE << "Worker '" << name << "' destroyed (releasing " << tools.size() << " tools)" << RESET << std::endl;

    // Leave from all workshops
    std::vector<Workshop*> workshopsCopy = workshops;
    for (std::size_t i = 0; i < workshopsCopy.size(); ++i)
    {
        if (workshopsCopy[i])
            workshopsCopy[i]->releaseWorker(this);
    }
    workshops.clear();

    // Release all tools (aggregation)
    for (std::size_t i = 0; i < tools.size(); ++i)
    {
        tools[i]->setOwner(0);
    }
    tools.clear();
}


void Worker::takeTool(ATool* tool)
{
	if (!tool)
		return;
	
	// If tool already has an owner, remove it from that owner first
	Worker* previousOwner = tool->getOwner();
	if (previousOwner)
	{
		previousOwner->releaseTool(tool);
	}
	
	// Add tool to this worker
	tools.push_back(tool);
	tool->setOwner(this);
	std::cout << BLUE << "Worker '" << name << "' received a tool" << RESET << std::endl;
}

void Worker::releaseTool(ATool* tool)
{
	if (!tool)
		return;
	
	// Remove tool from this worker's collection
	std::vector<ATool*>::iterator it = std::find(tools.begin(), tools.end(), tool);
	if (it != tools.end())
	{
		tools.erase(it);
		tool->setOwner(0);
		std::cout << BLUE << "Worker '" << name << "' released a tool" << RESET << std::endl;
		
		std::vector<Workshop*> workshopsCopy = workshops;
		for (std::size_t i = 0; i < workshopsCopy.size(); ++i)
		{
			workshopsCopy[i]->checkWorkerTools(this);
		}
	}
}

void Worker::registerWorkshop(Workshop* workshop)
{
	if (!workshop)
		return;
	
	if (std::find(workshops.begin(), workshops.end(), workshop) == workshops.end())
	{
		workshops.push_back(workshop);
		std::cout << BLUE << "Worker '" << name << "' joined a workshop" << RESET << std::endl;
	}
}

void Worker::unregisterWorkshop(Workshop* workshop)
{
	if (!workshop)
		return;
	
	std::vector<Workshop*>::iterator it = std::find(workshops.begin(), workshops.end(), workshop);
	if (it != workshops.end())
	{
		workshops.erase(it);
		std::cout << BLUE << "Worker '" << name << "' left a workshop" << RESET << std::endl;
	}
}

void Worker::work()
{
	if (workshops.empty())
	{
		std::cout << BLUE << "Worker '" << name << "' is not registered to any workshop" << RESET << std::endl;
		return;
	}
	
	std::cout << BLUE << "Worker '" << name << "' is working..." << RESET << std::endl;
	
	// Use tools if available
	for (std::size_t i = 0; i < tools.size(); ++i)
	{
		tools[i]->use();
	}
}

std::string Worker::getName() const
{
	return name;
}