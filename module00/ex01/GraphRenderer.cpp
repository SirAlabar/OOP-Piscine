#include "GraphRenderer.hpp"
#include <iostream>
#include <vector>
#include <cmath>

// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include "stb_image_write.h"

void GraphRenderer::display(const Graph& graph)
{
    const Vector2& size = graph.getSize();
    const std::vector<Vector2>& points = graph.getPoints();
    const std::vector<std::pair<Vector2, Vector2> >& lines = graph.getLines();

    int width = static_cast<int>(size.X) + 1;
    int height = static_cast<int>(size.Y) + 1;

    std::vector<std::vector<char> > grid(height, std::vector<char>(width, '.'));

    //place points
    for (size_t i = 0; i < points.size(); i++)
    {
        int x = static_cast<int>(points[i].X);
        int y = static_cast<int>(points[i].Y);
        if(x >= 0 && x < width && y >= 0 && y < height)
        {
            grid[y][x] = 'X';
        }
    }
    //draw lines
    for (size_t i = 0; i < lines.size(); i++)
    {
        drawLine(grid, lines[i].first, lines[i].second);
    }

    //display grid top to bottom
    for (int y = height - 1; y >= 0; y--)
    {
        std::cout << ">& ";
        if (y < 10)
            std::cout << " ";
        std::cout << y << " ";
        
        for (int x = 0; x < width; x++)
        {
            std::cout << grid[y][x] << " ";
        }
        std::cout << std::endl;
    }
    
    // X-axis
    std::cout << ">&   ";
    for (int x = 0; x < width; x++)
    {
        if (x < 10)
            std::cout << " " << x;
        else
            std::cout << x;
    }
    std::cout << std::endl;
}

void GraphRenderer::drawLine(std::vector<std::vector<char> >& grid, const Vector2& start, const Vector2& end)
{
    int x0 = static_cast<int>(start.X);
    int y0 = static_cast<int>(start.Y);
    int x1 = static_cast<int>(end.X);
    int y1 = static_cast<int>(end.Y);
    
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    
    while (true)
    {
        if (x0 >= 0 && x0 < static_cast<int>(grid[0].size()) && 
            y0 >= 0 && y0 < static_cast<int>(grid.size()) && 
            grid[y0][x0] == '.')
        {
            grid[y0][x0] = '-';
        }
        
        if (x0 == x1 && y0 == y1)
        {
            break;
        }
        
        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}


// void exportToPNG(const Graph& graph, const std::string& filename);