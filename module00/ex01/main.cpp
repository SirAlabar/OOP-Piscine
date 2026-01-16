#include "Graph.hpp"
#include "GraphRenderer.hpp"
#include <iostream>

int main()
{
    // Create graph with size 5x5
    Graph graph(35, 35);
    
    // Add points from subject example
    Vector2 p1 = {0, 0};
    Vector2 p2 = {2, 2};
    Vector2 p3 = {4, 2};
    Vector2 p4 = {2, 4};
    Vector2 p5 = {20.5, 24.7};
    
    graph.addPoint(p1);
    graph.addPoint(p2);
    graph.addPoint(p3);
    graph.addPoint(p4);
    graph.addPoint(p5);
    
    std::cout << "=== Display graph ===" << std::endl;
    GraphRenderer::display(graph);
    
    // std::cout << "\n=== Adding lines ===" << std::endl;
    // graph.addLine(p1, p2);
    // graph.addLine(p2, p3);
    // graph.addLine(p2, p4);
    
    // GraphRenderer::display(graph);
    
    // std::cout << "\n=== Export to PNG ===" << std::endl;
    // GraphRenderer::exportToPNG(graph, "graph.png");
    
    // std::cout << "\n=== Load from file ===" << std::endl;
    // Graph graph2(10, 10);
    // graph2.loadFromFile("points.input");
    // GraphRenderer::display(graph2);
    // GraphRenderer::exportToPNG(graph2, "graph2.png");
    
    return 0;
}