#include "simulation/MovementSystem.hpp"
#include "simulation/SimulationContext.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "core/Node.hpp"
#include "patterns/events/Event.hpp"
#include "patterns/observers/EventManager.hpp"
#include "patterns/events/StationDelayEvent.hpp"
#include "patterns/events/SignalFailureEvent.hpp"
#include "patterns/states/StateRegistry.hpp"
#include <iostream>

void MovementSystem::checkSignalFailures(Train* train, SimulationContext* ctx)
{
    if (!train || !ctx)
    {
        return;
    }
    
    // Check for active signal failures
    EventManager& eventManager = EventManager::getInstance();
    const auto& activeEvents = eventManager.getActiveEvents();
    
    for (Event* event : activeEvents)
    {
        if (event && event->getType() == EventType::SIGNAL_FAILURE)
        {
            // SignalFailureEvent checks if train is on rail approaching the node
            if (event->isApplicableToTrain(train))
            {
                SignalFailureEvent* signalEvent = dynamic_cast<SignalFailureEvent*>(event);
                if (signalEvent)
                {
                    // Force emergency stop and set wait duration
                    double stopSeconds = signalEvent->getStopDuration().toMinutes() * 60.0;
                    ctx->setStopDuration(train, stopSeconds);
                    return;
                }
            }
        }
    }
}

void MovementSystem::resolveProgress(Train* train, SimulationContext* ctx)
{
    if (!train || !ctx)
        return;

    Rail* currentRail = train->getCurrentRail();

    // Train has already finished its journey
    if (!currentRail)
    {
        return;
    }

    // Ensure position is never negative
    if (train->getPosition() < 0.0)
    {
        train->setPosition(0.0);
    }

    // If still inside the current rail, nothing to do
    if (!hasReachedEndOfRail(train, ctx))
    {
        return;
    }

    // Train reached or passed the end of the current rail
    Node* arrivalNode = ctx->getCurrentArrivalNode(train);

    // Handle arrival logic (city stop or junction pass)
    handleArrivalAtNode(train, ctx, arrivalNode);
}

bool MovementSystem::hasReachedEndOfRail(const Train* train, SimulationContext* ctx)
{
    if (!train || !ctx)
    {
        return false;
    }

    return train->getPosition() >= ctx->getCurrentRailLength(train);
}

void MovementSystem::advanceToNextSegment(Train* train)
{
    if (!train)
    {
        return;
    }

    train->advanceToNextRail();
    train->setPosition(0.0);
}


void MovementSystem::handleArrivalAtNode(Train* train, SimulationContext* ctx, Node* arrivalNode)
{
    if (!train || !ctx)
    {
        return;
    }

    // If no more rails, journey is complete
    if (isJourneyComplete(train))
    {
        train->setVelocity(0.0);
        train->setState(ctx->states().stopped());
        
        // Set stop duration at final destination (needed for round-trip mode)
        double stopSeconds = train->getStopDuration().toMinutes() * 60.0;
        ctx->setStopDuration(train, stopSeconds);
        
        train->advanceToNextRail();  // Sets currentRail to nullptr
        train->markFinished();
        return;
    }

    // If arriving at a city, train must stop UNLESS it's at the departure station
    if (arrivalNode && arrivalNode->getType() == NodeType::CITY)
    {
        if (train->getCurrentRailIndex() == 0)
        {
            // This is the departure station - advance without stopping
            advanceToNextSegment(train);
            return;
        }

        // This is an intermediate or final city station - apply stop duration
        train->setVelocity(0.0);

        double stopSeconds = train->getStopDuration().toMinutes() * 60.0;
        
        // Check for active Station Delay events at this station
        EventManager& eventManager = EventManager::getInstance();
        const auto& activeEvents = eventManager.getActiveEvents();
        
        for (Event* event : activeEvents)
        {
            if (event && event->getType() == EventType::STATION_DELAY)
            {
                if (event->isApplicableToTrain(train))
                {
                    StationDelayEvent* stationEvent = dynamic_cast<StationDelayEvent*>(event);
                    if (stationEvent)
                    {
                        // Add additional delay time
                        double additionalSeconds = stationEvent->getAdditionalDelay().toMinutes() * 60.0;
                        stopSeconds += additionalSeconds;
                        break;
                    }
                }
            }
        }
        
        ctx->setStopDuration(train, stopSeconds);

        // Advance to next segment so train waits at position 0 of next rail
        advanceToNextSegment(train);

        return;
    }

    // Otherwise, proceed to next segment
    advanceToNextSegment(train);
}

bool MovementSystem::isJourneyComplete(const Train* train)
{
    if (!train)
    {
        return true;
    }

    const auto& path = train->getPath();
    if (path.empty())
    {
        return true;
    }

    return train->getCurrentRailIndex() == path.size() - 1;
}