#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "io/CLI.hpp"

class IOutputWriter;

// Orchestrates the entire railway simulation application
class Application
{
public:
	Application(int argc, char* argv[]);
	~Application();
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	int run();

private:
	CLI _cli;
	IOutputWriter* _consoleWriter;  // Dependency injection for output strategy
};

#endif