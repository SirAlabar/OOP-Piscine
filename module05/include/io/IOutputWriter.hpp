#ifndef IOUTPUTWRITER_HPP
#define IOUTPUTWRITER_HPP

#include <string>
#include <vector>
#include "utils/Time.hpp"

class Train;
class Graph;
class Event;
class Node;
class Rail;

// Strategy pattern for different output formats
// Separates output logic from business logic (Single Responsibility)
class IOutputWriter
{
public:
	virtual ~IOutputWriter() = default;
	
	// Startup phase
	virtual void writeStartupHeader() = 0;
	virtual void writeConfiguration(const std::string& key, const std::string& value) = 0;
	virtual void writeNetworkSummary(int nodeCount, int railCount) = 0;
	virtual void writeGraphDetails(const std::vector<Node*>& nodes,
	                                const std::vector<Rail*>& rails) = 0;
	virtual void writeTrainCreated(const std::string& trainName, int trainId, 
	                                const std::string& from, const std::string& to, 
	                                int segments) = 0;
	virtual void writePathDebug(const Train* train) = 0;
	
	// Simulation phase
	virtual void writeSimulationStart() = 0;
	virtual void writeTrainSchedule(const std::string& trainName, const Time& departureTime) = 0;
	virtual void writeEventActivated(const Time& currentTime, const std::string& eventType, 
	                                  const std::string& description) = 0;
	virtual void writeEventEnded(const Time& currentTime, const std::string& eventType) = 0;
	virtual void writeDashboard(const Time& currentTime, int activeTrains, int totalTrains,
	                             int completedTrains, int activeEvents) = 0;
	
	// Completion phase
	virtual void writeSimulationComplete() = 0;
	virtual void writeOutputFileListing(const std::string& filename) = 0;
	
	// Progress/status
	virtual void writeProgress(const std::string& message) = 0;
	virtual void writeError(const std::string& message) = 0;
};

#endif