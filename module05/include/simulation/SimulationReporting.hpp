#ifndef SIMULATIONREPORTING_HPP
#define SIMULATIONREPORTING_HPP

#include <vector>
#include <map>

class Train;
class SimulationContext;
class EventScheduler;
class ISimulationOutput;
class FileOutputWriter;
class ITrainState;

// Owns the two periodic output steps
//   - writeSnapshots  : emit per-train snapshot to .result files on state
//                       change or every 2 simulated minutes.
//   - updateDashboard : emit the console status table every 5 minutes.
class SimulationReporting
{
private:
    ISimulationOutput*& _simulationWriter;
    WriterMap&          _outputWriters;
    TrainList&          _trains;
    SimulationContext*& _context;
    EventScheduler&     _eventScheduler;
    double&             _currentTime;
    StateMap&           _previousStates;
    int&                _lastSnapshotMinute;
    int&                _lastDashboardMinute;

    bool isTrainActive(const Train* train) const;

public:
    using TrainList = std::vector<Train*>;
    using WriterMap = std::map<Train*, FileOutputWriter*>;
    using StateMap  = std::map<Train*, ITrainState*>;

    SimulationReporting(
        ISimulationOutput*& simulationWriter,
        WriterMap&          outputWriters,
        TrainList&          trains,
        SimulationContext*& context,
        EventScheduler&     eventScheduler,
        double&             currentTime,
        StateMap&           previousStates,
        int&                lastSnapshotMinute,
        int&                lastDashboardMinute
    );

    void writeSnapshots();
    void updateDashboard();
};

#endif