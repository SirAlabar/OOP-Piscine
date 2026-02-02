#include "Application.hpp"
#include <iostream>
#include <exception>

int main(int argc, char* argv[])
{
	try
	{
		Application app(argc, argv);
		return app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Fatal error: " << e.what() << std::endl;
		return 1;
	}
}
