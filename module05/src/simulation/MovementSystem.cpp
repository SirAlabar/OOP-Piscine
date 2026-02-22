#include "simulation/MovementSystem.hpp"
#include "simulation/SimulationContext.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "core/Node.hpp"
#include "patterns/events/Event.hpp"
#include "patterns/events/StationDelayEvent.hpp"
#include "patterns/events/SignalFailureEvent.hpp"
#include "patterns/states/StateRegistry.hpp"

namespace
{
    // Returns the first active event of the given EventType that is applicable
    // to the train.
    template<typename T>
    T* findApplicableEvent(const std::vector<Event*>& events,
                           EventType                  type,
                           Train*                     train)
    {
        for (Event* event : events)
        {
            if (event
                && event->getType() == type
                && event->isApplicableToTrain(train))
            {
                return static_cast<T*>(event);
            }
        }
        return nullptr;
    }
}

void MovementSystem::checkSignalFailures(Train*                     train,
                                         SimulationContext*         ctx,
                                         const std::vector<Event*>& activeEvents)
{
    if (!train || !ctx)
    {
        return;
    }

    SignalFailureEvent* signalEvent =
        findApplicableEvent<SignalFailureEvent>(activeEvents,
                                               EventType::SIGNAL_FAILURE,
                                               train);
    if (signalEvent)
    {
        double stopSeconds = signalEvent->getStopDuration().toSeconds();
        ctx->setStopDuration(train, stopSeconds);
    }
}

void MovementSystem::resolveProgress(Train*                     train,
                                     SimulationContext*         ctx,
                                     const std::vector<Event*>& activeEvents)
{
    if (!train || !ctx)
    {
        return;
    }

    if (!train->getCurrentRail())
    {
        return;
    }

    if (train->getPosition() < 0.0)
    {
        train->setPosition(0.0);
    }

    if (!hasReachedEndOfRail(train, ctx))
    {
        return;
    }

    Node* arrivalNode = ctx->getCurrentArrivalNode(train);
    handleArrivalAtNode(train, ctx, arrivalNode, activeEvents);
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

void MovementSystem::handleArrivalAtNode(Train*                     train,
                                         SimulationContext*         ctx,
                                         Node*                      arrivalNode,
                                         const std::vector<Event*>& activeEvents)
{
    if (!train || !ctx)
    {
        return;
    }

    if (isJourneyComplete(train))
    {
        train->setVelocity(0.0);
        train->setState(ctx->states().stopped());

        double stopSeconds = train->getStopDuration().toSeconds();
        ctx->setStopDuration(train, stopSeconds);

        train->advanceToNextRail();
        train->markFinished();
        return;
    }

    if (arrivalNode && arrivalNode->getType() == NodeType::CITY)
    {
        if (train->getCurrentRailIndex() == 0)
        {
            // Departure station — pass through without stopping.
            advanceToNextSegment(train);
            return;
        }

        train->setVelocity(0.0);

        double stopSeconds = train->getStopDuration().toSeconds();

        // Apply additional delay from an active StationDelayEvent, if present.
        StationDelayEvent* stationEvent =
            findApplicableEvent<StationDelayEvent>(activeEvents,
                                                   EventType::STATION_DELAY,
                                                   train);
        if (stationEvent)
        {
            stopSeconds += stationEvent->getAdditionalDelay().toSeconds();
        }

        ctx->setStopDuration(train, stopSeconds);
        advanceToNextSegment(train);
        return;
    }

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