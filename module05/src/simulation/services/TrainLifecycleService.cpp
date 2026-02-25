#include "simulation/services/TrainLifecycleService.hpp"
#include "simulation/core/SimulationContext.hpp"
#include "simulation/core/SimConstants.hpp"
#include "simulation/systems/MovementSystem.hpp"
#include "patterns/behavioral/mediator/TrafficController.hpp"
#include "event_system/EventScheduler.hpp"
#include "patterns/behavioral/command/ICommandRecorder.hpp"
#include "patterns/behavioral/command/TrainDepartureCommand.hpp"
#include "patterns/behavioral/command/TrainStateChangeCommand.hpp"
#include "patterns/behavioral/command/TrainAdvanceRailCommand.hpp"
#include "patterns/behavioral/states/ITrainState.hpp"
#include "core/Train.hpp"

TrainLifecycleService::TrainLifecycleService(
    std::vector<Train*>&                    trains,
    std::unique_ptr<SimulationContext>&     context,
    std::unique_ptr<TrafficController>&     trafficCtrl,
    double&                                 currentTime,
    double&                                 timestep,
    bool&                                   roundTripEnabled,
    EventScheduler&                         eventScheduler)
    : _trains(trains),
      _context(context),
      _trafficCtrl(trafficCtrl),
      _currentTime(currentTime),
      _timestep(timestep),
      _roundTripEnabled(roundTripEnabled),
      _eventScheduler(eventScheduler),
      _recorder(nullptr)
{
}

void TrainLifecycleService::setCommandRecorder(ICommandRecorder* recorder)
{
    _recorder = recorder;
}

Time TrainLifecycleService::getCurrentTimeFormatted() const
{
    return Time::fromSeconds(_currentTime);
}

void TrainLifecycleService::checkDepartures()
{
    if (!_context)
    {
        return;
    }

    Time currentTimeFormatted = getCurrentTimeFormatted();

    for (Train* train : _trains)
    {
        if (!train || !train->getCurrentState() || train->isFinished())
        {
            continue;
        }

        if (train->getCurrentState() != _context->states().idle())
        {
            continue;
        }

        if (currentTimeFormatted < train->getDepartureTime())
        {
            continue;
        }

        const auto& path = train->getPath();
        if (path.empty() || !path[0].rail)
        {
            continue;
        }

        if (!_trafficCtrl)
        {
            continue;
        }

        TrafficController::AccessDecision decision =
            _trafficCtrl->requestRailAccess(train, path[0].rail);

        if (decision == TrafficController::GRANT)
        {
            train->setState(_context->states().accelerating());

            if (_recorder)
            {
                _recorder->record(
                    new TrainDepartureCommand(_currentTime, train->getName()));
            }
        }
    }
}

void TrainLifecycleService::handleStateTransitions()
{
    if (!_context)
    {
        return;
    }

    for (Train* train : _trains)
    {
        if (!_context->isTrainActive(train))
        {
            continue;
        }

        std::string  prevStateName = train->getCurrentState()->getName();
        ITrainState* newState      =
            train->getCurrentState()->checkTransition(train, _context.get());

        if (!newState)
        {
            continue;
        }

        train->setState(newState);

        if (_recorder)
        {
            _recorder->record(
                new TrainStateChangeCommand(
                    _currentTime,
                    train->getName(),
                    prevStateName,
                    newState->getName()));
        }
    }
}

void TrainLifecycleService::updateTrainStates(double dt)
{
    if (!_context)
    {
        return;
    }

    const std::vector<Event*>& activeEvents = _eventScheduler.getActiveEvents();

    for (Train* train : _trains)
    {
        if (!train)
        {
            continue;
        }

        if (train->isFinished() && !_roundTripEnabled)
        {
            continue;
        }

        train->update(dt);

        if (train->getCurrentState() == _context->states().stopped())
        {
            bool expired = _context->decrementStopDuration(train, dt);

            if (expired)
            {
                _context->clearStopDuration(train);

                if (train->isFinished() && _roundTripEnabled)
                {
                    train->reverseJourney();

                    int currentMinutes   = static_cast<int>(
                        _currentTime / SimConfig::SECONDS_PER_MINUTE);
                    int departureMinutes = train->getDepartureTime().toMinutes();
                    int nextDeparture    =
                        departureMinutes + SimConfig::MINUTES_PER_HALF_DAY;

                    while (nextDeparture <= currentMinutes)
                    {
                        nextDeparture += SimConfig::MINUTES_PER_HALF_DAY;
                    }

                    train->setDepartureTime(
                        Time(nextDeparture / 60, nextDeparture % 60));
                    train->setState(_context->states().idle());
                }
            }
        }

        MovementSystem::checkSignalFailures(train, _context.get(), activeEvents);

        std::size_t prevRailIndex = train->getCurrentRailIndex();
        MovementSystem::resolveProgress(train, _context.get(), activeEvents);
        std::size_t newRailIndex = train->getCurrentRailIndex();

        if (newRailIndex != prevRailIndex && _recorder)
        {
            _recorder->record(
                new TrainAdvanceRailCommand(
                    _currentTime, train->getName(), newRailIndex));
        }
    }
}