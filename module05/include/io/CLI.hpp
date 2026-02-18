#ifndef CLI_HPP
#define CLI_HPP

#include <map>
#include <string>

// Handles command-line argument parsing and help messages
class CLI
{
public:
    CLI(int argc, char* argv[]);
    CLI(const CLI&)            = default;
    CLI& operator=(const CLI&) = default;
    ~CLI()                     = default;

    bool shouldShowHelp()    const;
    bool hasValidArguments() const;

    std::string getNetworkFile() const;
    std::string getTrainFile()   const;

    // Optional flags
    bool         hasSeed()           const;
    unsigned int getSeed()           const;

    std::string  getPathfinding()    const;  // "dijkstra" or "astar"
    bool         hasRender()         const;
    bool         hasHotReload()      const;
    bool         hasRoundTrip()      const;

    bool         hasMonteCarloRuns() const;
    unsigned int getMonteCarloRuns() const;

    // Command Pattern / Replay
    bool         hasRecord()         const;  // --record
    bool         hasReplay()         const;  // --replay=file
    std::string  getReplayFile()     const;  // Path provided to --replay=

    void printUsage(const std::string& programName) const;
    void printHelp()                                const;

    bool validateFlags(std::string& errorMsg) const;

private:
    int   _argc;
    char** _argv;
    std::map<std::string, std::string> _flags;

    void parseFlags();
    bool parseFlag(const std::string& arg, std::string& key, std::string& value);
};

#endif