#include "Graph.hpp"
#include <fstream>
#include <sstream>

Graph::Graph(float width, float height)
{
    _size.X = width;
    _size.Y = height;
}

void Graph::addPoint(const Vector2& point)
{
    _points.push_back(point);
}

void Graph::addLine(const Vector2& start, const Vector2& end)
{
    _lines.push_back(std::make_pair(start, end));
}

void Graph::loadFromFile(const std::string& filename)
{
    std::ifstream file(filename.c_str());
    if (!file.is_open())
    {
        return;
    }
    
    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        Vector2 point;
        if (iss >> point.X >> point.Y)
        {
            addPoint(point);
        }
    }
    file.close();
}

const Vector2& Graph::getSize() const
{
    return _size;
}

const std::vector<Vector2>& Graph::getPoints() const
{
    return _points;
}

const std::vector<std::pair<Vector2, Vector2> >& Graph::getLines() const
{
    return _lines;
}