#ifndef TRAINLIFECYCLESERVICE_HPP
#define TRAINLIFECYCLESERVICE_HPP

#include <vector>
#include <memory>
#include "utils/Time.hpp"

class Train;
class SimulationContext;
class TrafficController;
class EventScheduler;
class ICommandRecorder;

// Owns the three per-tick train lifecycle steps:
//   - checkDepartures       : promote Idle trains when schedule time is met
//   - handleStateTransitions: ask each state for its next state
//   - updateTrainStates     : apply physics + resolve progress
class TrainLifecycleService
{
public:
    TrainLifecycleService(
        std::vector<Train*>&                    trains,
        std::unique_ptr<SimulationContext>&     context,
        std::unique_ptr<TrafficController>&     trafficCtrl,
        double&                                 currentTime,
        double&                                 timestep,
        bool&                                   roundTripEnabled,
        EventScheduler&                         eventScheduler
    );

    void setCommandRecorder(ICommandRecorder* recorder);

    void checkDepartures();
    void handleStateTransitions();
    void updateTrainStates(double dt);

private:
    std::vector<Train*>&                _trains;
    std::unique_ptr<SimulationContext>& _context;
    std::unique_ptr<TrafficController>& _trafficCtrl;
    double&                             _currentTime;
    double&                             _timestep;
    bool&                               _roundTripEnabled;
    EventScheduler&                     _eventScheduler;
    ICommandRecorder*                   _recorder;

    Time getCurrentTimeFormatted() const;
};

#endif