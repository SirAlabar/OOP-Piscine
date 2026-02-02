#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "io/CLI.hpp"

// Orchestrates the entire railway simulation application
class Application
{
public:
	Application(int argc, char* argv[]);
	Application(const Application&) = default;
	Application& operator=(const Application&) = default;
	~Application() = default;

	int run();

private:
	CLI _cli;
};

#endif