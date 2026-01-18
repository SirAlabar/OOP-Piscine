#ifndef GRAPH_HPP
#define GRAPH_HPP

#include "Vector2.hpp"
#include <vector>
#include <string>
#include <utility>

class Graph
{
private:
    Vector2 _size;
    std::vector<Vector2> _points;
    std::vector<std::pair<Vector2, Vector2> > _lines;
    // private for integrity, user cant add invalid points
    // prevent direct manipulations
    // can add bounds checking

public:
    Graph(float width, float height);
    
    void addPoint(const Vector2& point);
    void addLine(const Vector2& start, const Vector2& end);
    bool loadFromFile(const std::string& filename);

    // Getters
    const Vector2& getSize() const;
    const std::vector<Vector2>& getPoints() const;
    const std::vector<std::pair<Vector2, Vector2> >& getLines() const;
};

#endif