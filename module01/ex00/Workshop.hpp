#ifndef WORKSHOP_HPP
#define WORKSHOP_HPP

#include <vector>
#include <iostream>
#include "Worker.hpp"

class Workshop
{
private:
	std::vector<Worker*> workers; //Association

public:
	Workshop();
	Workshop(const Workshop& other);
	Workshop& operator=(const Workshop& other);
	~Workshop();

	void registerWorker(Worker* worker);
	void releaseWorker(Worker* worker);
	void executeWorkDay();
	
	// Check if worker still has required tools
	void checkWorkerTools(Worker* worker);
};

// Template Workshop that requires specific tool type
template<typename RequiredTool>
class WorkshopWithTool : public Workshop
{
public:
	void registerWorker(Worker* worker)
	{
		if (!worker)
        {
			return;
        }
		
		// Check if worker has the required tool
		RequiredTool* tool = worker->GetTool<RequiredTool>();
		if (!tool)
		{
            std::cout << "\033[33mWorker '" << worker->getName() << "' lost required tool, releasing from workshop\033[0m" << std::endl;
            releaseWorker(worker);
            return;
		}
		
		// Use base class registration
		Workshop::registerWorker(worker);
		std::cout << "\033[33mWorker '" << worker->getName() << "' successfully registered to specialized workshop\033[0m" << std::endl;
	}
	
	void checkWorkerTools(Worker* worker)
	{
		if (!worker)
        {
			return;
        }
		
		// Check if worker still has the required tool
		RequiredTool* tool = worker->GetTool<RequiredTool>();
		if (!tool)
		{
			// Worker lost the required tool, release them
			releaseWorker(worker);
		}
	}
};

#endif