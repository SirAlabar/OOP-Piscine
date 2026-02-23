#include "simulation/systems/EventPipeline.hpp"
#include "simulation/core/SimulationContext.hpp"
#include "simulation/core/SimConstants.hpp"
#include "event_system/EventScheduler.hpp"
#include "patterns/creational/factories/EventFactory.hpp"
#include "events/Event.hpp"
#include "patterns/behavioral/command/ICommandRecorder.hpp"
#include "patterns/behavioral/command/SimEventCommand.hpp"
#include "io/ISimulationOutput.hpp"
#include "io/FileOutputWriter.hpp"
#include "analysis/StatsCollector.hpp"
#include "core/Train.hpp"
#include "utils/Time.hpp"
#include <map>

EventPipeline::EventPipeline(
    EventFactory*&      eventFactory,
    EventScheduler&     eventScheduler,
    std::vector<Train*>&                 trains,
    SimulationContext*&                  context,
    ISimulationOutput*&                  simulationWriter,
    std::map<Train*, FileOutputWriter*>& outputWriters,
    StatsCollector*&    statsCollector,
    double&             currentTime,
    double&             lastEventGenerationTime)
    : _eventFactory(eventFactory),
      _eventScheduler(eventScheduler),
      _trains(trains),
      _context(context),
      _simulationWriter(simulationWriter),
      _outputWriters(outputWriters),
      _statsCollector(statsCollector),
      _currentTime(currentTime),
      _lastEventGenerationTime(lastEventGenerationTime),
      _recorder(nullptr)
{
}

void EventPipeline::setCommandRecorder(ICommandRecorder* recorder)
{
    _recorder = recorder;
}



void EventPipeline::update()
{
    if (!_eventFactory)
    {
        return;
    }

    Time currentTimeFormatted = Time::fromSeconds(_currentTime);

    // --- Snapshot pre-update active events (type counts) ---
    // We need counts, not pointers, because update() may delete expired events.
    const std::vector<Event*>& previousActive = _eventScheduler.getActiveEvents();
    std::map<std::string, int> preCounts;
    for (Event* e : previousActive)
    {
        preCounts[Event::typeToString(e->getType())]++;
    }

    // Remember which events were active before, to detect newly activated ones.
    std::vector<Event*> prevActiveSnapshot = previousActive;

    // --- Advance scheduler ---
    _eventScheduler.update(currentTimeFormatted);
    const std::vector<Event*>& currentActive = _eventScheduler.getActiveEvents();

    // --- Post-update type counts (for expiration detection) ---
    std::map<std::string, int> postCounts;
    for (Event* e : currentActive)
    {
        postCounts[Event::typeToString(e->getType())]++;
    }

    // --- Detect and handle newly activated events ---
    for (Event* event : currentActive)
    {
        bool isNew = true;
        for (Event* prev : prevActiveSnapshot)
        {
            if (prev == event)
            {
                isNew = false;
                break;
            }
        }

        if (isNew)
        {
            notifyNewEvent(event);
        }
    }

    // --- Detect and notify ended events ---
    notifyEndedEvents(preCounts, postCounts);

    // --- Periodic event generation (every 60 simulated seconds) ---
    double timeSinceLastGeneration = _currentTime - _lastEventGenerationTime;
    if (timeSinceLastGeneration >= SimConfig::SECONDS_PER_MINUTE)
    {
        std::vector<Event*> newEvents =
            _eventFactory->tryGenerateEvents(currentTimeFormatted,
                                             SimConfig::SECONDS_PER_MINUTE);

        for (Event* event : newEvents)
        {
            if (event)
            {
                _eventScheduler.scheduleEvent(event);
            }
        }

        _lastEventGenerationTime = _currentTime;
    }
}

void EventPipeline::notifyNewEvent(Event* event)
{
    std::string eventTypeStr = Event::typeToString(event->getType());

    logEventForAffectedTrains(event, "ACTIVATED");

    if (_recorder)
    {
        _recorder->record(
            new SimEventCommand(
                _currentTime,
                eventTypeStr,
                event->getDescription()));
    }

    if (_simulationWriter && _context && _context->hasAnyActiveTrain())
    {
        Time t = Time::fromSeconds(_currentTime);

        _simulationWriter->writeEventActivated(t, eventTypeStr,
                                               event->getDescription());
    }
}

void EventPipeline::notifyEndedEvents(
    const std::map<std::string, int>& preCounts,
    const std::map<std::string, int>& postCounts)
{
    if (!_simulationWriter)
    {
        return;
    }

    Time t = Time::fromSeconds(_currentTime);

    for (const auto& kv : preCounts)
    {
        const std::string& typeStr   = kv.first;
        int                prevCount = kv.second;
        int                currCount = 0;

        auto it = postCounts.find(typeStr);
        if (it != postCounts.end())
        {
            currCount = it->second;
        }

        for (int i = 0; i < prevCount - currCount; ++i)
        {
            _simulationWriter->writeEventEnded(t, typeStr);
        }
    }
}

void EventPipeline::logEventForAffectedTrains(Event* event,
                                               const std::string& action)
{
    if (!event)
    {
        return;
    }

    std::string eventTypeStr = Event::typeToString(event->getType());

    for (Train* train : _trains)
    {
        if (!_context || !_context->isTrainActive(train))
        {
            continue;
        }

        if (!event->isApplicableToTrain(train))
        {
            continue;
        }

        if (_statsCollector && action == "ACTIVATED")
        {
            _statsCollector->recordEventForTrain(train);
        }

        auto writerIt = _outputWriters.find(train);
        if (writerIt != _outputWriters.end() && writerIt->second)
        {
            writerIt->second->writeEventNotification(
                _currentTime,
                eventTypeStr,
                event->getDescription(),
                action);
        }
    }
}