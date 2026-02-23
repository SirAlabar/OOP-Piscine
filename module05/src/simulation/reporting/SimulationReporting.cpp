#include "simulation/reporting/SimulationReporting.hpp"
#include "simulation/core/SimulationContext.hpp"
#include "simulation/core/SimConstants.hpp"
#include "event_system/EventScheduler.hpp"
#include "patterns/behavioral/states/ITrainState.hpp"
#include "io/ISimulationOutput.hpp"
#include "io/FileOutputWriter.hpp"
#include "core/Train.hpp"
#include "utils/Time.hpp"

SimulationReporting::SimulationReporting(
    ISimulationOutput*& simulationWriter,
    std::map<Train*, FileOutputWriter*>& outputWriters,
    std::vector<Train*>&                 trains,
    SimulationContext*&                  context,
    EventScheduler&                      eventScheduler,
    double&                              currentTime,
    std::map<Train*, ITrainState*>&      previousStates,
    int&                lastSnapshotMinute,
    int&                lastDashboardMinute)
    : _simulationWriter(simulationWriter),
      _outputWriters(outputWriters),
      _trains(trains),
      _context(context),
      _eventScheduler(eventScheduler),
      _currentTime(currentTime),
      _previousStates(previousStates),
      _lastSnapshotMinute(lastSnapshotMinute),
      _lastDashboardMinute(lastDashboardMinute)
{
}

void SimulationReporting::writeSnapshots()
{
    int  currentMinute = static_cast<int>(_currentTime / SimConfig::SECONDS_PER_MINUTE);
    bool periodicWrite =
        (currentMinute % 2 == 0) && (currentMinute != _lastSnapshotMinute);

    if (periodicWrite)
    {
        _lastSnapshotMinute = currentMinute;
    }

    for (auto& pair : _outputWriters)
    {
        Train*            train  = pair.first;
        FileOutputWriter* writer = pair.second;

        if (!writer || !(_context && _context->isTrainActive(train)))
        {
            continue;
        }

        ITrainState* currentState = train->getCurrentState();
        bool         stateChanged = false;

        auto prevIt = _previousStates.find(train);
        if (prevIt != _previousStates.end())
        {
            stateChanged = (prevIt->second != currentState);
        }
        else
        {
            _previousStates[train] = currentState;
            stateChanged           = true;
        }

        if (stateChanged || periodicWrite)
        {
            writer->writeSnapshot(_currentTime);
            _previousStates[train] = currentState;
        }
    }
}

void SimulationReporting::updateDashboard()
{
    if (!_simulationWriter)
    {
        return;
    }

    int currentMinute =
        static_cast<int>(_currentTime / SimConfig::SECONDS_PER_MINUTE);

    bool shouldUpdate =
        (currentMinute % 5 == 0)
        && (currentMinute != _lastDashboardMinute)
        && (currentMinute > 0);

    if (!shouldUpdate)
    {
        return;
    }

    int activeTrains    = 0;
    int completedTrains = 0;

    for (Train* train : _trains)
    {
        if (!train)
        {
            continue;
        }

        if (train->isFinished())
        {
            completedTrains++;
        }
        else if (_context && _context->isTrainActive(train))
        {
            activeTrains++;
        }
    }

    if (activeTrains == 0 && completedTrains == 0)
    {
        return;
    }

    _lastDashboardMinute = currentMinute;

    int activeEvents =
        static_cast<int>(_eventScheduler.getActiveEvents().size());

    Time t = Time::fromSeconds(_currentTime);

    _simulationWriter->writeDashboard(
        t,
        activeTrains,
        static_cast<int>(_trains.size()),
        completedTrains,
        activeEvents);
}