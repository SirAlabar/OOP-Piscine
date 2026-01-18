#include "GraphRenderer.hpp"
#include <iostream>
#include <vector>
#include <cmath>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#pragma GCC diagnostic pop

static void drawLinePNG(std::vector<unsigned char>& image, int width, int height,
                        int x0, int y0, int x1, int y1,
                        unsigned char r, unsigned char g, unsigned char b)
{
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    
    while (true)
    {
        if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height)
        {
            int idx = (y0 * width + x0) * 3;
            image[idx] = r;
            image[idx + 1] = g;
            image[idx + 2] = b;
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

void GraphRenderer::exportToPNG(const Graph& graph, const std::string& filename)
{
    const Vector2& size = graph.getSize();
    const std::vector<Vector2>& points = graph.getPoints();
    const std::vector<std::pair<Vector2, Vector2> >& lines = graph.getLines();

    int gridWidth = static_cast<int>(size.X) + 1;
    int gridHeight = static_cast<int>(size.Y) + 1;

    // 50px per point
    int scale = 50;
    int imgWidth = gridWidth * scale;
    int imgHeight = gridHeight * scale;

    // Image buffer 3 bytes RGB
    std::vector<unsigned char> image(imgWidth * imgHeight * 3, 255);

    // draw grid
    for (int y = 0; y < imgHeight; y++)
    {
        for (int x = 0; x < imgWidth; x++)
        {
            if (x % scale == 0 || y % scale == 0)
            {
                int idx = (y * imgWidth + x) * 3;
                image[idx] = 200;     // R
                image[idx + 1] = 200; // G
                image[idx + 2] = 200; // B
            }
        }
    }

    //draw lines
    for (size_t i = 0; i < lines.size(); i++)
    {
        int x0 = static_cast<int>(lines[i].first.X * scale + scale / 2);
        int y0 = static_cast<int>((gridHeight - 1 - lines[i].first.Y) * scale + scale / 2);
        int x1 = static_cast<int>(lines[i].second.X * scale + scale / 2);
        int y1 = static_cast<int>((gridHeight - 1 - lines[i].second.Y) * scale + scale / 2);
        
        drawLinePNG(image, imgWidth, imgHeight, x0, y0, x1, y1, 0, 0, 255);
    }

    // draw points
    for (size_t i = 0; i < points.size(); i++)
    {
        int x = static_cast<int>(points[i].X * scale + scale / 2);
        int y = static_cast<int>((gridHeight - 1 - points[i].Y) * scale + scale / 2);
        
        int radius = scale / 4;
        for (int dy = -radius; dy <= radius; dy++)
        {
            for (int dx = -radius; dx <= radius; dx++)
            {
                if (dx * dx + dy * dy <= radius * radius)
                {
                    int px = x + dx;
                    int py = y + dy;
                    if (px >= 0 && px < imgWidth && py >= 0 && py < imgHeight)
                    {
                        int idx = (py * imgWidth + px) * 3;
                        image[idx] = 255;
                        image[idx + 1] = 0;
                        image[idx + 2] = 0;
                    }
                }
            }
        }
    }
    
    // Write PNG
    stbi_write_png(filename.c_str(), imgWidth, imgHeight, 3, &image[0], imgWidth * 3);
    std::cout << "PNG exported to " << filename << std::endl;
}

