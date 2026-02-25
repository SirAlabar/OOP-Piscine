#ifndef SIMULATIONREPORTING_HPP
#define SIMULATIONREPORTING_HPP

#include <vector>
#include <map>
#include <memory>

class Train;
class SimulationContext;
class EventScheduler;
class ISimulationOutput;
class FileOutputWriter;
class ITrainState;

// Owns the two periodic output steps:
//   - writeSnapshots  : emit per-train snapshot on state change or every 2 minutes
//   - updateDashboard : emit console status table every 5 minutes
class SimulationReporting
{
public:
    SimulationReporting(
        ISimulationOutput*&                     simulationWriter,
        std::map<Train*, FileOutputWriter*>&    outputWriters,
        std::vector<Train*>&                    trains,
        std::unique_ptr<SimulationContext>&     context,
        EventScheduler&                         eventScheduler,
        double&                                 currentTime,
        std::map<Train*, ITrainState*>&         previousStates,
        int&                                    lastSnapshotMinute,
        int&                                    lastDashboardMinute
    );

    void writeSnapshots();
    void updateDashboard();

private:
    ISimulationOutput*&                     _simulationWriter;
    std::map<Train*, FileOutputWriter*>&    _outputWriters;
    std::vector<Train*>&                    _trains;
    std::unique_ptr<SimulationContext>&     _context;
    EventScheduler&                         _eventScheduler;
    double&                                 _currentTime;
    std::map<Train*, ITrainState*>&         _previousStates;
    int&                                    _lastSnapshotMinute;
    int&                                    _lastDashboardMinute;
};

#endif