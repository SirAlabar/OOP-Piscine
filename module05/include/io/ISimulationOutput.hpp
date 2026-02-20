#ifndef ISIMULATIONOUTPUT_HPP
#define ISIMULATIONOUTPUT_HPP

#include <string>
#include "utils/Time.hpp"

// Narrow interface for simulation-domain output.
// These are the only two methods SimulationManager actually calls on its
// output writer — it should not be forced to depend on the 16-method
// IOutputWriter interface.
class ISimulationOutput
{
public:
    virtual ~ISimulationOutput() = default;

    // Called when a simulation event activates (weather, signal failure, etc.).
    virtual void writeEventActivated(const Time&        currentTime,
                                     const std::string& eventType,
                                     const std::string& description) = 0;

    // Called every N simulation-minutes to emit a status dashboard line.
    virtual void writeDashboard(const Time& currentTime,
                                int         activeTrains,
                                int         totalTrains,
                                int         completedTrains,
                                int         activeEvents) = 0;
};

#endif