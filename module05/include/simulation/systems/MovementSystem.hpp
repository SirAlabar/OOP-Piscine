#ifndef MOVEMENTSYSTEM_HPP
#define MOVEMENTSYSTEM_HPP

#include <vector>

class Train;
class SimulationContext;
class Node;
class Event;

// Responsible for managing logical movement of trains along their path.
// No direct dependency on EventManager — callers supply the active event list.
class MovementSystem
{
public:
    // Checks for signal failures affecting the train and forces a stop if found.
    // activeEvents: the current snapshot from IEventScheduler::getActiveEvents().
    static void checkSignalFailures(Train*                     train,
                                    SimulationContext*         ctx,
                                    const std::vector<Event*>& activeEvents);

    // Resolves train progression after the physics update.
    // activeEvents: the current snapshot from IEventScheduler::getActiveEvents().
    static void resolveProgress(Train*                     train,
                                SimulationContext*         ctx,
                                const std::vector<Event*>& activeEvents);

private:
    static bool hasReachedEndOfRail(const Train* train, SimulationContext* ctx);
    static void advanceToNextSegment(Train* train);
    static void handleArrivalAtNode(Train*                     train,
                                    SimulationContext*         ctx,
                                    Node*                      arrivalNode,
                                    const std::vector<Event*>& activeEvents);
    static bool isJourneyComplete(const Train* train);
};

#endif