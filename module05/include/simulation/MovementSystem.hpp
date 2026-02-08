#ifndef MOVEMENTSYSTEM_HPP
#define MOVEMENTSYSTEM_HPP

class Train;
class SimulationContext;
class Node;

// Responsible for managing logical movement of trains along their path
class MovementSystem
{
public:
    // Main entry point: resolves train progression after physics update
    static void resolveProgress(Train* train, SimulationContext* ctx);
    
    // Checks for signal failures and forces trains to stop if needed
    static void checkSignalFailures(Train* train, SimulationContext* ctx);

private:
    // Checks if train has reached or passed end of current rail
    static bool hasReachedEndOfRail(const Train* train, SimulationContext* ctx);

    // Advances train to next rail in path
    static void advanceToNextSegment(Train* train);

    // Handles arrival logic at a node (city or junction)
    static void handleArrivalAtNode(Train* train, SimulationContext* ctx, Node* arrivalNode);

    // Checks if train completed its entire journey
    static bool isJourneyComplete(const Train* train);
};

#endif