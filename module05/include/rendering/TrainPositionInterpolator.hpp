#ifndef TRAINPOSITIONINTERPOLATOR_HPP
#define TRAINPOSITIONINTERPOLATOR_HPP

#include "rendering/RenderTypes.hpp"
#include <SFML/System/Vector2.hpp>
#include <map>

class Train;
class Node;
class Rail;
class CameraManager;

// Pure geometry helper: converts a train's scalar progress along its current
// rail into an isometric screen position, using the stored L-shaped rail paths.
//
// Extracted from RenderManager so that the interpolation logic can be tested
// without a render window and does not contribute to RenderManager's SRP burden.
class TrainPositionInterpolator
{
public:
    // Compute the isometric screen position of train.
    //
    // nodePositions: world-grid positions of each node (built by SFMLRenderer)
    // railPaths    : L-shaped path geometry for each rail (built by SFMLRenderer)
    // camera       : supplies offset for projectIsometric
    // movingRight  : set to true if the train is moving along the horizontal
    //                segment (used by TrainRenderer to pick sprite direction)
    static sf::Vector2f compute(
        const Train*                               train,
        const CameraManager&                       camera,
        const std::map<const Node*, sf::Vector2f>& nodePositions,
        const std::map<const Rail*, RailPath>&     railPaths,
        bool&                                      movingRight
    );
};

#endif