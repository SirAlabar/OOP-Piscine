#include "Workshop.hpp"
#include <algorithm>
#include <iostream>
#include <cstddef>

#define YELLOW "\033[33m"
#define RESET "\033[0m"

Workshop::Workshop()
{
	std::cout << YELLOW << "Workshop created" << RESET << std::endl;
}

Workshop::Workshop(const Workshop&)
{
	// Don't copy workers (association, not composition)
	std::cout << YELLOW << "Workshop copied" << RESET << std::endl;
}

Workshop& Workshop::operator=(const Workshop& other)
{
	if (this != &other)
	{
		// Don't copy workers (association, not composition)
	}
	return *this;
}

Workshop::~Workshop()
{
    std::cout << YELLOW << "Workshop destroyed (releasing " << workers.size() << " workers)" << RESET << std::endl;

    // Unregister all workers
    std::vector<Worker*> workersCopy = workers;
    for (std::size_t i = 0; i < workersCopy.size(); ++i)
    {
        if (workersCopy[i])
            workersCopy[i]->unregisterWorkshop(this);
    }
    workers.clear();
}



void Workshop::registerWorker(Worker* worker)
{
	if (!worker)
		return;
	
	// Avoid duplicates
	if (std::find(workers.begin(), workers.end(), worker) == workers.end())
	{
		workers.push_back(worker);
		worker->registerWorkshop(this);
		std::cout << YELLOW << "Workshop accepted worker '" << worker->getName() << "'" << RESET << std::endl;
	}
}

void Workshop::releaseWorker(Worker* worker)
{
	if (!worker)
		return;
	
	std::vector<Worker*>::iterator it = std::find(workers.begin(), workers.end(), worker);
	if (it != workers.end())
	{
		workers.erase(it);
		worker->unregisterWorkshop(this);
		std::cout << YELLOW << "Workshop released worker '" << worker->getName() << "'" << RESET << std::endl;
	}
}

void Workshop::executeWorkDay()
{
	std::cout << YELLOW << "Workshop executing work day with " << workers.size() << " workers" << RESET << std::endl;
	
	for (std::size_t i = 0; i < workers.size(); ++i)
	{
		workers[i]->work();
	}
}

void Workshop::checkWorkerTools(Worker* worker)
{
    (void)worker;
	// Base implementation does nothing
	// Override in WorkshopWithTool for tool-specific checking
}