#include <iostream>

#define RESET   "\033[0m"
#define CYAN    "\033[36m"
#define GREEN   "\033[32m"

int main()
{
	std::cout << CYAN << "=== Ex00: File Decomposition ===" << RESET << std::endl;
	std::cout << GREEN << "Files successfully decomposed from datas.hpp!" << RESET << std::endl;
	std::cout << "Check the file structure in each directory:" << std::endl;
	std::cout << "  - people/" << std::endl;
	std::cout << "  - academic/" << std::endl;
	std::cout << "  - rooms/" << std::endl;
	std::cout << "  - forms/" << std::endl;
	std::cout << "  - enums/" << std::endl;
	
	return 0;
}