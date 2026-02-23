#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "io/CLI.hpp"
#include "simulation/core/SimulationBuilder.hpp"
#include "simulation/systems/CollisionAvoidance.hpp"
#include "simulation/core/SimulationManager.hpp"
#include <memory>
#include <string>
#include <vector>

class IOutputWriter;
class IRunModeHandler;

// Orchestrates run-mode selection and simulation lifecycle at a high level.
class Application
{
private:
    CLI                _cli;
    IOutputWriter*     _consoleWriter;
    CollisionAvoidance _collision;
    SimulationManager  _sim;
    SimulationBuilder  _builder;

    std::vector<std::unique_ptr<IRunModeHandler> > _modeHandlers;

    void registerModeHandlers();
    void printConfiguration(const std::string& netFile, const std::string& trainFile) const;

public:
    Application(int argc, char* argv[]);
    ~Application();
    Application(const Application&)            = delete;
    Application& operator=(const Application&) = delete;

    int run();
};

#endif
