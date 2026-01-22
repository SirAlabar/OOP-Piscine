#include <iostream>
#include "Worker.hpp"
#include "ATool.hpp"
#include "Shovel.hpp"
#include "Hammer.hpp"
#include "Workshop.hpp"

#define CYAN "\033[36m"
#define MAGENTA "\033[35m"
#define RESET "\033[0m"

int main()
{
	std::cout << CYAN << "\n=== Testing Composition ===" << RESET << std::endl;
	Position pos(10, 20, 30);
	Statistic stat(5, 100);
	Worker worker1("Alice", pos, stat);
	std::cout << std::endl;

	std::cout << CYAN << "=== Testing Aggregation ===" << RESET << std::endl;
	Shovel shovel1;
	Hammer hammer1;
	
	std::cout << "Giving shovel to Alice..." << std::endl;
	worker1.takeTool(&shovel1);
	
	std::cout << "Giving hammer to Alice..." << std::endl;
	worker1.takeTool(&hammer1);
	std::cout << std::endl;

	std::cout << CYAN << "=== Testing Inheritance & Polymorphism ===" << RESET << std::endl;
	ATool* tool = &shovel1;
	std::cout << "Using tool polymorphically: ";
	tool->use();
	std::cout << std::endl;

	std::cout << CYAN << "=== Testing GetTool Bonus ===" << RESET << std::endl;
	Shovel* foundShovel = worker1.GetTool<Shovel>();
	if (foundShovel)
		std::cout << "Found Shovel in Alice's tools!" << std::endl;
	
	Hammer* foundHammer = worker1.GetTool<Hammer>();
	if (foundHammer)
		std::cout << "Found Hammer in Alice's tools!" << std::endl;
	std::cout << std::endl;

	std::cout << CYAN << "=== Testing Association: Worker <-> Workshop ===" << RESET << std::endl;
	Workshop workshop1;
	workshop1.registerWorker(&worker1);
	std::cout << std::endl;

	std::cout << CYAN << "=== Testing executeWorkDay ===" << RESET << std::endl;
	workshop1.executeWorkDay();
	std::cout << std::endl;

	std::cout << CYAN << "=== Testing Tool Transfer (Aggregation) ===" << RESET << std::endl;
	Worker worker2("Bob");
	std::cout << "Transferring shovel from Alice to Bob..." << std::endl;
	worker2.takeTool(&shovel1);
	std::cout << "Bob now has the shovel" << std::endl;
	std::cout << std::endl;

	std::cout << CYAN << "=== Testing WorkshopWithTool Bonus ===" << RESET << std::endl;
	WorkshopWithTool<Shovel> shovelWorkshop;
	Worker worker3("Charlie");
	Hammer hammer2;
	
	worker3.takeTool(&hammer2);
	std::cout << "Charlie has only Hammer, trying to join Shovel workshop..." << std::endl;
	shovelWorkshop.registerWorker(&worker3);
	
	Shovel shovel2;
	worker3.takeTool(&shovel2);
	std::cout << "Charlie now has Shovel, trying to join again..." << std::endl;
	shovelWorkshop.registerWorker(&worker3);
	std::cout << std::endl;

	std::cout << CYAN << "=== Testing Auto-Release Bonus ===" << RESET << std::endl;
	std::cout << "Charlie releases shovel..." << std::endl;
	worker3.releaseTool(&shovel2);
	std::cout << "Charlie should be auto-released from shovel workshop" << std::endl;
	std::cout << std::endl;

	std::cout << CYAN << "=== Testing Worker Destruction with Tools ===" << RESET << std::endl;
	Shovel shovel3;
	Hammer hammer3;
	{
		Worker tempWorker("David");
		tempWorker.takeTool(&shovel3);
		tempWorker.takeTool(&hammer3);
		std::cout << "David has 2 tools, going out of scope..." << std::endl;
	}
	std::cout << "David destroyed but tools survive:" << std::endl;
	std::cout << "Testing tools:" << std::endl;
	shovel3.use();
	hammer3.use();
	std::cout << std::endl;

	std::cout << MAGENTA << "=== All tests completed ===" << RESET << std::endl;
	std::cout << MAGENTA << "=== Destroying remaining objects ===" << RESET << std::endl;
	
	return 0;
}