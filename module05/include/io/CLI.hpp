#ifndef CLI_HPP
#define CLI_HPP

#include <string>
#include <map>

// Handles command-line argument parsing and help messages
class CLI
{
public:
	CLI(int argc, char* argv[]);
	CLI(const CLI&) = default;
	CLI& operator=(const CLI&) = default;
	~CLI() = default;

	bool shouldShowHelp() const;
	bool hasValidArguments() const;

	std::string getNetworkFile() const;
	std::string getTrainFile() const;

	// Optional flags
	bool hasSeed() const;
	unsigned int getSeed() const;
	
	std::string getPathfinding() const;  // Returns "dijkstra" or "astar"
	bool hasRender() const;
	bool hasHotReload() const;
	bool hasRoundTrip() const;
	
	bool hasMonteCarloRuns() const;
	unsigned int getMonteCarloRuns() const;

	void printUsage(const std::string& programName) const;
	void printHelp() const;
	
	// Validate all parsed flags
	bool validateFlags(std::string& errorMsg) const;

private:
	int _argc;
	char** _argv;
	std::map<std::string, std::string> _flags;
	
	void parseFlags();
	bool parseFlag(const std::string& arg, std::string& key, std::string& value);
};

#endif