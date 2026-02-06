#include "simulation/MovementSystem.hpp"
#include "simulation/SimulationContext.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "core/Node.hpp"
#include <iostream>

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

    // std::cout << "[MOVEMENT] Train " << train->getName()
    //     << " reached end of rail "
    //     << currentRail->getNodeA()->getName() << " -> "
    //     << currentRail->getNodeB()->getName()
    //     << std::endl;

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

    Rail* oldRail = train->getCurrentRail();

    train->advanceToNextRail();

    Rail* newRail = train->getCurrentRail();

    train->setPosition(0.0);

    if (oldRail && newRail)
    {
        // std::cout << "[MOVEMENT] Train " << train->getName()
        //           << " advancing to next rail "
        //           << newRail->getNodeA()->getName() << " -> "
        //           << newRail->getNodeB()->getName()
        //           << std::endl;
    }
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
        std::cout << "[MOVEMENT] " << train->getName()
                << " COMPLETED JOURNEY at "
                << arrivalNode->getName()
                << std::endl;

        train->setVelocity(0.0);
        train->advanceToNextRail();
        train->markFinished();

        return;
    }

    // If arriving at a city, train must stop
    if (arrivalNode && arrivalNode->getType() == NodeType::CITY)
    {
        train->setVelocity(0.0);

        double stopSeconds = train->getStopDuration().toMinutes() * 60.0;
        ctx->setStopDuration(train, stopSeconds);

        // std::cout << "[MOVEMENT] Train " << train->getName()
        //         << " arrived at station " << arrivalNode->getName()
        //         << " - stopping for " << stopSeconds << " seconds"
        //         << std::endl;

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
