#ifndef SIMULATIONCONFIG_HPP
#define SIMULATIONCONFIG_HPP

class Graph;
class ISimulationOutput;

// Aggregates all one-time configuration values that Application passes to
// SimulationManager before calling run().
// writer is ISimulationOutput* — SimulationManager only needs
// writeEventActivated and writeDashboard, not the full IOutputWriter.
struct SimulationConfig
{
    Graph*             network   = nullptr;
    unsigned int       seed      = 0;
    bool               roundTrip = false;
    ISimulationOutput* writer    = nullptr;
};

#endif