#include "Graph.hpp"
#include "GraphRenderer.hpp"
#include <iostream>

void connectPoints(Graph& graph, const std::vector<Vector2>& points, int start, int end)
{
    for (int i = start; i <= end; i++)
    {
        if (i > start)
        {
            graph.addLine(points[i-1], points[i]);
        }
    }
}

int main()
{
    // Create graph with size 5x5
    Graph graph(15, 15);
    
    // Add points from subject example
    Vector2 p1 = {0, 0};
    Vector2 p2 = {2, 2};
    Vector2 p3 = {4, 2};
    Vector2 p4 = {2, 4};
    Vector2 p5 = {10.5, 12.7};
    
    graph.addPoint(p1);
    graph.addPoint(p2);
    graph.addPoint(p3);
    graph.addPoint(p4);
    graph.addPoint(p5);
    
    std::cout << "=== Display graph ===" << std::endl;
    GraphRenderer::display(graph);
    
    std::cout << "\n=== Adding lines ===" << std::endl;
    graph.addLine(p1, p2);
    graph.addLine(p2, p3);
    graph.addLine(p2, p4);
    
    GraphRenderer::display(graph);
    
    std::cout << "\n=== Export to PNG ===" << std::endl;
    GraphRenderer::exportToPNG(graph, "graph.png");
    
    std::cout << "\n=== Load from file ===" << std::endl;
    Graph graph2(10, 10);
    graph2.loadFromFile("points.input");
    if (!graph2.loadFromFile("points.input"))
    {
        std::cerr << "Failed to load fileo" << std::endl;
        return 1;
    }
    GraphRenderer::display(graph2);
    GraphRenderer::exportToPNG(graph2, "graph2.png");
    
    std::cout << "=== Loading Sword from file ===" << std::endl;
    Graph sword(100, 100);
    if (!sword.loadFromFile("sword_points.input"))
    {
        std::cerr << "Failed to load sword file" << std::endl;
        return 1;
    }
    
    const std::vector<Vector2>& points = sword.getPoints();
    
    // Blade center line (0-10)
    connectPoints(sword, points, 0, 10);
    
    // Blade left edge
    connectPoints(sword, points, 11, 23);
    
    // Blade right edge  
    connectPoints(sword, points, 12, 24);
    
    // Crossguard main
    connectPoints(sword, points, 25, 31);
    
    // Crossguard detail
    connectPoints(sword, points, 32, 43);
    
    // Handle center
    connectPoints(sword, points, 44, 50);
    
    // Handle left side
    connectPoints(sword, points, 51, 61);
    
    // Handle right side
    connectPoints(sword, points, 52, 62);
    
    // Pommel rows
    connectPoints(sword, points, 63, 68);
    connectPoints(sword, points, 69, 75);
    connectPoints(sword, points, 76, 80);
    
    GraphRenderer::display(sword);
    GraphRenderer::exportToPNG(sword, "sword.png");
    
    return 0;
}