#include "rendering/GraphLayoutEngine.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <limits>
#include <algorithm>
#include <map>

std::map<const Node*, sf::Vector2i>
GraphLayoutEngine::compute(const Graph* graph) const
{
    std::map<const Node*, sf::Vector2i> result;

    if (!graph)
    {
        return result;
    }

    std::vector<Node*> nodes = graph->getNodes();

    if (nodes.empty())
    {
        return result;
    }

    const float initialRadius =
        std::max(10.0f, static_cast<float>(nodes.size()) * 2.0f);
    const float maxRadius =
        static_cast<float>(nodes.size()) * 4.0f;

    std::map<const Node*, sf::Vector2f> positions;
    std::map<const Node*, sf::Vector2f> velocities;

    const float angleStep =
        2.0f * 3.14159265f / static_cast<float>(nodes.size());

    for (std::size_t i = 0; i < nodes.size(); ++i)
    {
        const Node* node = nodes[i];
        float angle = static_cast<float>(i) * angleStep;

        positions[node]  = sf::Vector2f(std::cos(angle) * initialRadius,
                                        std::sin(angle) * initialRadius);
        velocities[node] = sf::Vector2f(0.0f, 0.0f);
    }

    for (int iter = 0; iter < iterations; ++iter)
    {
        std::map<const Node*, sf::Vector2f> forces;

        for (const Node* node : nodes)
        {
            forces[node] = sf::Vector2f(0.0f, 0.0f);
        }

        // Repulsion between every node pair.
        for (std::size_t i = 0; i < nodes.size(); ++i)
        {
            for (std::size_t j = i + 1; j < nodes.size(); ++j)
            {
                const Node* a = nodes[i];
                const Node* b = nodes[j];

                sf::Vector2f delta = positions.at(b) - positions.at(a);
                float distSq = delta.x * delta.x + delta.y * delta.y;
                float dist   = std::sqrt(distSq);

                if (dist < 0.1f)
                {
                    dist = 0.1f;
                }

                sf::Vector2f dir = delta / dist;

                int degreeA =
                    static_cast<int>(
                        graph->getNeighbors(const_cast<Node*>(a)).size());
                int degreeB =
                    static_cast<int>(
                        graph->getNeighbors(const_cast<Node*>(b)).size());

                float degreeFactor =
                    1.0f / std::sqrt(static_cast<float>(degreeA + degreeB));
                float force = repulsionStrength * degreeFactor / distSq;

                forces[a] -= dir * force;
                forces[b] += dir * force;
            }
        }

        // Spring attraction along rails.
        for (const Rail* rail : graph->getRails())
        {
            const Node* a = rail->getNodeA();
            const Node* b = rail->getNodeB();

            sf::Vector2f delta = positions.at(b) - positions.at(a);
            float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);

            if (dist < 0.1f)
            {
                continue;
            }

            sf::Vector2f dir = delta / dist;

            float desiredLength = minDistance;

            if (a->getType() != NodeType::CITY &&
                b->getType() != NodeType::CITY)
            {
                desiredLength *= 0.7f;
            }

            float force = (dist - desiredLength) * attractionStrength;

            forces[a] += dir * force;
            forces[b] -= dir * force;
        }

        // Alignment: pull junctions toward the midpoint of their two neighbours.
        for (const Node* node : nodes)
        {
            if (node->getType() == NodeType::CITY)
            {
                continue;
            }

            auto neighbours = graph->getNeighbors(const_cast<Node*>(node));

            if (neighbours.size() == 2)
            {
                sf::Vector2f midpoint =
                    (positions.at(neighbours[0]) + positions.at(neighbours[1]))
                    * 0.5f;

                sf::Vector2f delta = midpoint - positions.at(node);
                forces[node] += delta * 0.2f;
            }
        }

        // Integrate and clamp to maxRadius.
        for (const Node* node : nodes)
        {
            velocities[node] =
                (velocities.at(node) + forces.at(node)) * damping;
            positions[node] += velocities.at(node);

            float len = std::sqrt(positions.at(node).x * positions.at(node).x +
                                  positions.at(node).y * positions.at(node).y);

            if (len > maxRadius)
            {
                positions[node] = (positions.at(node) / len) * maxRadius;
            }
        }
    }

    // Centre the layout.
    sf::Vector2f centre(0.0f, 0.0f);

    for (const Node* node : nodes)
    {
        centre += positions.at(node);
    }

    centre /= static_cast<float>(nodes.size());

    for (const Node* node : nodes)
    {
        positions[node] -= centre;
    }

    // Compute bounding box.
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();

    for (const Node* node : nodes)
    {
        const auto& p = positions.at(node);
        if (p.x < minX) minX = p.x;
        if (p.x > maxX) maxX = p.x;
        if (p.y < minY) minY = p.y;
        if (p.y > maxY) maxY = p.y;
    }

    float width  = maxX - minX;
    float height = maxY - minY;
    float maxDim = std::max(width, height);

    if (maxDim < 1.0f)
    {
        maxDim = 1.0f;
    }

    float scale = targetSize / maxDim;

    // Convert to integer grid cells.
    for (const Node* node : nodes)
    {
        sf::Vector2f p = positions.at(node);

        p.x = (p.x - minX) * scale;
        p.y = (p.y - minY) * scale;

        result[node] = sf::Vector2i(
            static_cast<int>(std::round(p.x)),
            static_cast<int>(std::round(p.y)));
    }

    return result;
}