#include "rendering/animation/TrainPositionInterpolator.hpp"
#include "rendering/systems/CameraManager.hpp"
#include "rendering/core/RenderTypes.hpp"  // RailPath struct
#include "utils/IsometricUtils.hpp"
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "core/Node.hpp"
#include <cmath>
#include <algorithm>

sf::Vector2f TrainPositionInterpolator::compute(
    const Train*                               train,
    const CameraManager&                       camera,
    const std::map<const Node*, sf::Vector2f>& nodePositions,
    const std::map<const Rail*, RailPath>&     railPaths,
    bool&                                      movingRight)
{
    movingRight = true;

    if (!train)
    {
        return camera.getOffset();
    }

    const PathSegment* segment = train->getCurrentPathSegment();

    if (!segment || !segment->rail)
    {
        return camera.getOffset();
    }

    const Node* fromNode = segment->from;
    const Node* toNode   = segment->to;

    if (nodePositions.count(fromNode) == 0 ||
        nodePositions.count(toNode)   == 0)
    {
        return camera.getOffset();
    }

    const Rail* rail = segment->rail;

    // Fallback: straight-line interpolation when no L-path is stored.
    if (railPaths.count(rail) == 0)
    {
        const sf::Vector2f start = nodePositions.at(fromNode);
        const sf::Vector2f end   = nodePositions.at(toNode);

        double railLengthMeters = rail->getLength() * 1000.0;
        double t = (railLengthMeters > 0.0)
            ? (train->getPosition() / railLengthMeters)
            : 0.0;
        t = std::max(0.0, std::min(1.0, t));

        sf::Vector2f world(
            start.x + static_cast<float>((end.x - start.x) * t),
            start.y + static_cast<float>((end.y - start.y) * t)
        );

        return IsometricUtils::project(world, camera);
    }

    // Use stored L-path.
    const RailPath& path = railPaths.at(rail);

    // Orient path relative to travel direction.
    sf::Vector2f pathStart, pathCorner, pathEnd;

    if (fromNode == rail->getNodeA())
    {
        pathStart  = path.start;
        pathCorner = path.corner;
        pathEnd    = path.end;
    }
    else
    {
        pathStart  = path.end;
        pathCorner = path.corner;
        pathEnd    = path.start;
    }

    const float dx1 = pathCorner.x - pathStart.x;
    const float dy1 = pathCorner.y - pathStart.y;
    const float seg1Len = std::sqrt(dx1 * dx1 + dy1 * dy1);

    const float dx2 = pathEnd.x - pathCorner.x;
    const float dy2 = pathEnd.y - pathCorner.y;
    const float seg2Len = std::sqrt(dx2 * dx2 + dy2 * dy2);

    const float totalLength = seg1Len + seg2Len;

    double railLengthMeters = rail->getLength() * 1000.0;
    double t = (railLengthMeters > 0.0)
        ? (train->getPosition() / railLengthMeters)
        : 0.0;
    t = std::max(0.0, std::min(1.0, t));

    const float distAlongPath = static_cast<float>(t) * totalLength;

    sf::Vector2f worldPos;

    if (distAlongPath <= seg1Len)
    {
        float segT = (seg1Len > 0.0f) ? (distAlongPath / seg1Len) : 0.0f;
        worldPos.x = pathStart.x + (pathCorner.x - pathStart.x) * segT;
        worldPos.y = pathStart.y + (pathCorner.y - pathStart.y) * segT;
        movingRight = (std::abs(dx1) > std::abs(dy1));
    }
    else
    {
        float segT = (seg2Len > 0.0f)
            ? ((distAlongPath - seg1Len) / seg2Len)
            : 0.0f;
        worldPos.x = pathCorner.x + (pathEnd.x - pathCorner.x) * segT;
        worldPos.y = pathCorner.y + (pathEnd.y - pathCorner.y) * segT;
        movingRight = (std::abs(dx2) > std::abs(dy2));
    }

    return IsometricUtils::project(worldPos, camera);
}