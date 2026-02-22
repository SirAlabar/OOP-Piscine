#ifndef ISIMULATIONOUTPUT_HPP
#define ISIMULATIONOUTPUT_HPP

#include <string>
#include "utils/Time.hpp"

// Narrow interface for simulation-domain output.
class ISimulationOutput
{
public:
    virtual ~ISimulationOutput() = default;

    // Called when a simulation event activates (weather, signal failure, etc.).
    virtual void writeEventActivated(const Time&        currentTime,
                                     const std::string& eventType,
                                     const std::string& description) = 0;

    // Called when an active event expires.
    virtual void writeEventEnded(const Time&        currentTime,
                                 const std::string& eventType)
    {
        (void)currentTime;
        (void)eventType;
    }

    // Called every N simulation-minutes to emit a status dashboard line.
    virtual void writeDashboard(const Time& currentTime,
                                int         activeTrains,
                                int         totalTrains,
                                int         completedTrains,
                                int         activeEvents) = 0;
};

#endif