#ifndef GRAPHRENDERER_HPP
#define GRAPHRENDERER_HPP

#include "Graph.hpp"
#include <string>
#include <vector>

class GraphRenderer
{
private:
    static void drawLine(std::vector<std::vector<char> >& grid, const Vector2& start, const Vector2& end);

public:
    static void display(const Graph& graph);
    static void exportToPNG(const Graph& graph, const std::string& filename);
};

#endif