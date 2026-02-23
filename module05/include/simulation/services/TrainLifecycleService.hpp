#ifndef TRAINLIFECYCLESERVICE_HPP
#define TRAINLIFECYCLESERVICE_HPP

#include <vector>
#include "utils/Time.hpp"

class Train;
class SimulationContext;
class TrafficController;
class EventScheduler;
class ICommandRecorder;

// Owns the three per-tick train lifecycle steps that were previously
// private methods on SimulationManager:
//   - checkDepartures       : promote Idle trains when schedule time is met
//   - handleStateTransitions: ask each state for its next state
//   - updateTrainStates     : apply physics + resolve progress
//
// All data is held by reference back to SimulationManager's members so that
// setNetwork() re-assignments are automatically visible here.
class TrainLifecycleService
{
public:
    TrainLifecycleService(
        std::vector<Train*>& trains,
        SimulationContext*&  context,
        TrafficController*&  trafficCtrl,
        double&              currentTime,
        double&              timestep,
        bool&                roundTripEnabled,
        EventScheduler&      eventScheduler
    );

    // Set after construction (avoids passing `this` inside initializer list).
    void setCommandRecorder(ICommandRecorder* recorder);

    void checkDepartures();
    void handleStateTransitions();
    void updateTrainStates(double dt);

private:
    std::vector<Train*>& _trains;
    SimulationContext*& _context;
    TrafficController*& _trafficCtrl;
    double&             _currentTime;
    double&             _timestep;
    bool&               _roundTripEnabled;
    EventScheduler&     _eventScheduler;
    ICommandRecorder*   _recorder;

    Time getCurrentTimeFormatted() const;
};

#endif