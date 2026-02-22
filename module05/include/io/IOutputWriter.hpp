#ifndef IOUTPUTWRITER_HPP
#define IOUTPUTWRITER_HPP

#include <string>
#include <vector>
#include "utils/Time.hpp"
#include "io/ILogger.hpp"
#include "io/ISimulationOutput.hpp"

class Train;
class Graph;
class Event;
class Node;
class Rail;

class IOutputWriter : public ILogger, public ISimulationOutput
{
public:
    virtual ~IOutputWriter() = default;

    // Startup phase
    virtual void writeStartupHeader() = 0;
    virtual void writeConfiguration(const std::string& key,
                                    const std::string& value) = 0;
    virtual void writeNetworkSummary(int nodeCount, int railCount) = 0;
    virtual void writeGraphDetails(const std::vector<Node*>& nodes,
                                   const std::vector<Rail*>& rails) = 0;
    virtual void writeTrainCreated(const std::string& trainName, int trainId,
                                   const std::string& from,
                                   const std::string& to,
                                   int segments) = 0;
    virtual void writePathDebug(const Train* train) = 0;

    // Simulation phase
    // writeEventActivated, writeEventEnded, writeDashboard inherited from ISimulationOutput.
    virtual void writeSimulationStart() = 0;
    virtual void writeTrainSchedule(const std::string& trainName,
                                    const Time& departureTime) = 0;

    // Completion phase
    virtual void writeSimulationComplete() = 0;
    virtual void writeOutputFileListing(const std::string& filename) = 0;

    // writeProgress and writeError inherited from ILogger
};

#endif