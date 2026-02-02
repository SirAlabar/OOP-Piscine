#ifndef CLI_HPP
#define CLI_HPP

#include <string>

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

	void printUsage(const std::string& programName) const;
	void printHelp() const;

private:
	int _argc;
	char** _argv;
};

#endif