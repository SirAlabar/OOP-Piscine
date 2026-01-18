#include "Graph.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

Graph::Graph(float width, float height)
{
    if (width <= 0 || height <= 0)
    {
        throw std::invalid_argument("Graph size must be positive");
    }
    _size.X = width;
    _size.Y = height;
}

void Graph::addPoint(const Vector2& point)
{
    if (point.X < 0 || point.X > _size.X || point.Y < 0 || point.Y > _size.Y)
    {
        std::cerr << "Warning: Point (" << point.X << ", " << point.Y 
                  << ") outside graph bounds" << std::endl;
    }
    _points.push_back(point);
}

void Graph::addLine(const Vector2& start, const Vector2& end)
{
    _lines.push_back(std::make_pair(start, end));
}

bool Graph::loadFromFile(const std::string& filename)
{
    std::ifstream file(filename.c_str());
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }
    
    std::string line;
    int lineCount = 0;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        Vector2 point;
        if (iss >> point.X >> point.Y)
        {
            addPoint(point);
            lineCount++;
        }
    }
    file.close();
    
    std::cout << "Loaded " << lineCount << " points from " << filename << std::endl;
    return true;
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