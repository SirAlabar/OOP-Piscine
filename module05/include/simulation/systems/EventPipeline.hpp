#ifndef EVENTPIPELINE_HPP
#define EVENTPIPELINE_HPP

#include <vector>
#include <map>
#include <memory>
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

// Owns the event update cycle per tick:
//   1. Snapshot pre-update active events.
//   2. Advance the scheduler.
//   3. Notify newly activated / expired events.
//   4. Every 60 simulated seconds: try generating new events.
class EventPipeline
{
public:
    EventPipeline(
        std::unique_ptr<EventFactory>&           eventFactory,
        EventScheduler&                          eventScheduler,
        std::vector<Train*>&                     trains,
        std::unique_ptr<SimulationContext>&      context,
        ISimulationOutput*&                      simulationWriter,
        std::map<Train*, FileOutputWriter*>&     outputWriters,
        StatsCollector*&                         statsCollector,
        double&                                  currentTime,
        double&                                  lastEventGenerationTime
    );

    void setCommandRecorder(ICommandRecorder* recorder);

    void update();

private:
    std::unique_ptr<EventFactory>&           _eventFactory;
    EventScheduler&                          _eventScheduler;
    std::vector<Train*>&                     _trains;
    std::unique_ptr<SimulationContext>&      _context;
    ISimulationOutput*&                      _simulationWriter;
    std::map<Train*, FileOutputWriter*>&     _outputWriters;
    StatsCollector*&                         _statsCollector;
    double&                                  _currentTime;
    double&                                  _lastEventGenerationTime;
    ICommandRecorder*                        _recorder;

    void notifyNewEvent(Event* event);
    void notifyEndedEvents(
        const std::map<std::string, int>& preCounts,
        const std::map<std::string, int>& postCounts);
    void logEventForAffectedTrains(Event* event, const std::string& action);
};

#endif