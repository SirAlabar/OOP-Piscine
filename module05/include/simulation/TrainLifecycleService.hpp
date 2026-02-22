#ifndef TRAINLIFECYCLESERVICE_HPP
#define TRAINLIFECYCLESERVICE_HPP

#include <vector>
#include "utils/Time.hpp"

class Train;
class SimulationContext;
class TrafficController;
class EventScheduler;
class ICommandRecorder;

// Owns the three per-tick train lifecycle
//   - checkDepartures       : promote Idle trains when schedule time is met
//   - handleStateTransitions: ask each state for its next state
//   - updateTrainStates     : apply physics + resolve progress
//
class TrainLifecycleService
{
private:
    TrainList&          _trains;
    SimulationContext*& _context;
    TrafficController*& _trafficCtrl;
    double&             _currentTime;
    double&             _timestep;
    bool&               _roundTripEnabled;
    EventScheduler&     _eventScheduler;
    ICommandRecorder*   _recorder;

    bool isTrainActive(const Train* train) const;
    Time getCurrentTimeFormatted()         const;

public:
    using TrainList = std::vector<Train*>;

    TrainLifecycleService(
        TrainList&          trains,
        SimulationContext*& context,
        TrafficController*& trafficCtrl,
        double&             currentTime,
        double&             timestep,
        bool&               roundTripEnabled,
        EventScheduler&     eventScheduler
    );

    // Set after construction
    void setCommandRecorder(ICommandRecorder* recorder);

    void checkDepartures();
    void handleStateTransitions();
    void updateTrainStates(double dt);
};

#endif