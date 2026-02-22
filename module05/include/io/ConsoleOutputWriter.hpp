#ifndef CONSOLEOUTPUTWRITER_HPP
#define CONSOLEOUTPUTWRITER_HPP

#include "io/IOutputWriter.hpp"
#include "utils/ConsoleColors.hpp"
#include <iostream>

// Console output implementation with colors and formatting.
// Implements the full IOutputWriter interface, which transitively satisfies
// both ILogger and ISimulationOutput.
class ConsoleOutputWriter : public IOutputWriter
{
public:
    ConsoleOutputWriter()          = default;
    ~ConsoleOutputWriter() override = default;

    // ILogger
    void writeProgress(const std::string& message)  override;
    void writeError(const std::string& message)      override;

    // ISimulationOutput
    void writeEventActivated(const Time&        currentTime,
                             const std::string& eventType,
                             const std::string& description) override;
    void writeDashboard(const Time& currentTime, int activeTrains,
                        int totalTrains, int completedTrains,
                        int activeEvents)                     override;

    // IOutputWriter — startup
    void writeStartupHeader()                                           override;
    void writeConfiguration(const std::string& key,
                            const std::string& value)                  override;
    void writeNetworkSummary(int nodeCount, int railCount)              override;
    void writeGraphDetails(const std::vector<Node*>& nodes,
                           const std::vector<Rail*>& rails)            override;
    void writeTrainCreated(const std::string& trainName, int trainId,
                           const std::string& from, const std::string& to,
                           int segments)                                override;
    void writePathDebug(const Train* train)                             override;

    // IOutputWriter — simulation
    void writeSimulationStart()                                         override;
    void writeTrainSchedule(const std::string& trainName,
                            const Time& departureTime)                  override;
    void writeEventEnded(const Time& currentTime,
                         const std::string& eventType)                  override;

    // IOutputWriter — completion
    void writeSimulationComplete()                                      override;
    void writeOutputFileListing(const std::string& filename)            override;

private:
};

#endif