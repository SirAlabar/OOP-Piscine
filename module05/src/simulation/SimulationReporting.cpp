#include "simulation/SimulationReporting.hpp"
#include "simulation/SimulationContext.hpp"
#include "simulation/SimConstants.hpp"
#include "patterns/observers/EventScheduler.hpp"
#include "patterns/states/ITrainState.hpp"
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

bool SimulationReporting::isTrainActive(const Train* train) const
{
    return train
        && _context
        && train->getCurrentState()
        && !train->isFinished()
        && train->getCurrentState() != _context->states().idle();
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

        if (!writer || !isTrainActive(train))
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
        else if (isTrainActive(train))
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

    int totalMinutes = static_cast<int>(_currentTime / SimConfig::SECONDS_PER_MINUTE);
    Time t(totalMinutes / 60, totalMinutes % 60);

    _simulationWriter->writeDashboard(
        t,
        activeTrains,
        static_cast<int>(_trains.size()),
        completedTrains,
        activeEvents);
}