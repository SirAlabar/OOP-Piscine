#ifndef EVENTPIPELINE_HPP
#define EVENTPIPELINE_HPP

#include <vector>
#include <map>
#include <string>

class Train;
class SimulationContext;
class EventFactory;
class EventScheduler;
class ISimulationOutput;
class FileOutputWriter;
class StatsCollector;
class ICommandRecorder;
class Event;

// Owns the event update cycle that was previously split across
// updateEvents() and logEventForAffectedTrains() in SimulationManager:
//
//   update() — per-tick entry point:
//     1. Snapshot pre-update active events (to detect expirations).
//     2. Advance the scheduler (activates and expires events).
//     3. For newly activated events: log to console writer, record command,
//        update per-train file writers and stats.
//     4. For newly expired events: call writeEventEnded on console writer.
//     5. Every 60 simulated seconds: ask EventFactory to try generating events.
class EventPipeline
{
public:
    EventPipeline(
        EventFactory*&                        eventFactory,
        EventScheduler&                       eventScheduler,
        std::vector<Train*>&                  trains,
        SimulationContext*&                   context,
        ISimulationOutput*&                   simulationWriter,
        std::map<Train*, FileOutputWriter*>&  outputWriters,
        StatsCollector*&                      statsCollector,
        double&                               currentTime,
        double&                               lastEventGenerationTime
    );

    void setCommandRecorder(ICommandRecorder* recorder);

    void update();

private:
    EventFactory*&                       _eventFactory;
    EventScheduler&                      _eventScheduler;
    std::vector<Train*>&                 _trains;
    SimulationContext*&                  _context;
    ISimulationOutput*&                  _simulationWriter;
    std::map<Train*, FileOutputWriter*>& _outputWriters;
    StatsCollector*&                     _statsCollector;
    double&             _currentTime;
    double&             _lastEventGenerationTime;
    ICommandRecorder*   _recorder;

    void notifyNewEvent(Event* event);
    void notifyEndedEvents(
        const std::map<std::string, int>& preCounts,
        const std::map<std::string, int>& postCounts);
    void logEventForAffectedTrains(Event* event, const std::string& action);
};

#endif