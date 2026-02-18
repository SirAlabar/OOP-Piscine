#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "io/CLI.hpp"
#include <string>
#include <vector>

class IOutputWriter;
class Graph;
class Train;
class FileOutputWriter;

// Orchestrates the entire railway simulation application.
class Application
{
public:
    Application(int argc, char* argv[]);
    ~Application();
    Application(const Application&)            = delete;
    Application& operator=(const Application&) = delete;

    int run();

private:
    CLI            _cli;
    IOutputWriter* _consoleWriter;

    // Parse files, create all objects, configure SimulationManager.
    // seedOverride >= 0 forces a specific seed (used in replay mode).
    // Returns true on success; cleans up and returns false on failure.
    bool _buildSimulation(
        const std::string&              netFile,
        const std::string&              trainFile,
        Graph*&                         outGraph,
        std::vector<Train*>&            outTrains,
        std::vector<FileOutputWriter*>& outWriters,
        int                             seedOverride = -1
    );

    // Close writers, delete trains and graph, reset SimulationManager.
    void _teardownSimulation(
        Graph*&                         graph,
        std::vector<Train*>&            trains,
        std::vector<FileOutputWriter*>& writers
    );

    // Read an entire file into a string (for hot-reload undo snapshot).
    static std::string _readFile(const std::string& path);
};

#endif