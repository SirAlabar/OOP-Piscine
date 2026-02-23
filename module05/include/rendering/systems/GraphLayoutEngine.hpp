#ifndef GRAPHLAYOUTENGINE_HPP
#define GRAPHLAYOUTENGINE_HPP

#include <SFML/System/Vector2.hpp>
#include <map>

class Graph;
class Node;

// Computes an isometric-friendly 2D grid layout for a railway graph using
// a force-directed algorithm (repulsion + spring attraction + alignment).
//
// Usage:
//   GraphLayoutEngine engine;
//   auto positions = engine.compute(graph);
//   // positions: Node* -> grid cell (integer x, y)
class GraphLayoutEngine
{
public:
    // Layout tuning knobs — public so callers can override before calling compute().
    int   iterations        = 300;
    float repulsionStrength = 80.0f;
    float attractionStrength = 0.08f;
    float damping           = 0.75f;
    float minDistance       = 6.0f;
    float targetSize        = 40.0f;

    // Run the force-directed layout and return grid positions for every node.
    // Returns an empty map if graph is null or has no nodes.
    std::map<const Node*, sf::Vector2i> compute(const Graph* graph) const;
};

#endif