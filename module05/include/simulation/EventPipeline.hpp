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

// Owns the event update cycle 
//   update() — per-tick entry point:
//     1. Snapshot pre-update active events (to detect expirations).
//     2. Advance the scheduler (activates and expires events).
//     3. For newly activated events: log to console writer, record command,
//        update per-train file writers and stats.
//     4. For newly expired events: call writeEventEnded on console writer.
//     5. Every 60 simulated seconds: ask EventFactory to try generating events.
class EventPipeline
{
private:
    EventFactory*&      _eventFactory;
    EventScheduler&     _eventScheduler;
    TrainList&          _trains;
    SimulationContext*& _context;
    ISimulationOutput*& _simulationWriter;
    WriterMap&          _outputWriters;
    StatsCollector*&    _statsCollector;
    double&             _currentTime;
    double&             _lastEventGenerationTime;
    ICommandRecorder*   _recorder;

    bool isTrainActive(const Train* train) const;
    bool hasAnyActiveTrain()               const;

    void notifyNewEvent(Event* event);
    void notifyEndedEvents(
        const std::map<std::string, int>& preCounts,
        const std::map<std::string, int>& postCounts);
    void logEventForAffectedTrains(Event* event, const std::string& action);

public:
    using TrainList    = std::vector<Train*>;
    using WriterMap    = std::map<Train*, FileOutputWriter*>;

    EventPipeline(
        EventFactory*&     eventFactory,
        EventScheduler&    eventScheduler,
        TrainList&         trains,
        SimulationContext*& context,
        ISimulationOutput*& simulationWriter,
        WriterMap&         outputWriters,
        StatsCollector*&   statsCollector,
        double&            currentTime,
        double&            lastEventGenerationTime
    );

    void setCommandRecorder(ICommandRecorder* recorder);

    void update();
};

#endif