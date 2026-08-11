#include "MeshBuilder.h"
#include <iostream>
#include <cmath>
#include <algorithm>

int gCurrentTileIndex = -1;
static bool samePoint(const Point2D &a, const Point2D &b)
{
    return std::fabs(a.x - b.x) < 0.001 &&
           std::fabs(a.y - b.y) < 0.001;
}

void MeshBuilder::buildExtrudedBuilding(
    const std::vector<Point2D> &polygon,
    double height,
    double terrainHeight)
{
    vertices.clear();
    wallTriangles.clear();
    roofTriangles.clear();
    appendExtrudedBuilding(polygon, height, terrainHeight);
}

void MeshBuilder::appendExtrudedBuilding(
    const std::vector<Point2D> &polygon,
    double height,
    double terrainHeight)
{
    std::vector<Point2D> clean;

    for (const auto &p : polygon)
    {
        if (clean.empty() || !samePoint(clean.back(), p))
            clean.push_back(p);
    }

    if (clean.size() >= 2 && samePoint(clean.front(), clean.back()))
        clean.pop_back();

    // Remove nearly-collinear points
    std::vector<Point2D> simplified;

    for (size_t i = 0; i < clean.size(); i++)
    {
        const Point2D &prev = clean[(i + clean.size() - 1) % clean.size()];
        const Point2D &curr = clean[i];
        const Point2D &next = clean[(i + 1) % clean.size()];

        double cross =
            (curr.x - prev.x) * (next.y - curr.y) -
            (curr.y - prev.y) * (next.x - curr.x);

        if (std::fabs(cross) > 0.01)
            simplified.push_back(curr);
    }

    clean = simplified;

    if (clean.size() < 3)
        return;

    // Ensure polygon is counter-clockwise
    double area = 0.0;

    for (size_t i = 0; i < clean.size(); i++)
    {
        size_t j = (i + 1) % clean.size();
        area += clean[i].x * clean[j].y - clean[j].x * clean[i].y;
    }

    if (area < 0.0)
        std::reverse(clean.begin(), clean.end());

    int n = static_cast<int>(clean.size());

    if (n < 3)
        return;

    int offset = static_cast<int>(vertices.size());

    // Ground vertices
    for (const auto &p : clean)
    {
        vertices.push_back({
    (float)p.x,
    (float)p.y,
    (float)terrainHeight,
    0.0f,
    0.0f
});
    }

    // Roof vertices
    for (const auto &p : clean)
    {
        vertices.push_back({
    (float)p.x,
    (float)p.y,
    (float)(terrainHeight + height),
    0.0f,
    0.0f
});
    }

    // Walls
    // Walls with duplicated vertices and UVs
for (int i = 0; i < n; i++)
{
    int next = (i + 1) % n;

    const Point2D& p1 = clean[i];
    const Point2D& p2 = clean[next];

    // Length of this wall
    float wallLength = std::sqrt(
        (float)((p2.x - p1.x) * (p2.x - p1.x) +
                (p2.y - p1.y) * (p2.y - p1.y)));

    int base = (int)vertices.size();

    // Ground left
    vertices.push_back({
        (float)p1.x,
        (float)p1.y,
        (float)terrainHeight,
        0.0f,
        0.0f
    });

    // Ground right
vertices.push_back({
    (float)p2.x,
    (float)p2.y,
    (float)terrainHeight,
    wallLength / 8.0f,
    0.0f
});

// Roof left
vertices.push_back({
    (float)p1.x,
    (float)p1.y,
    (float)(terrainHeight + height),
    0.0f,
    (float)(height / 8.0f)
});

// Roof right
vertices.push_back({
    (float)p2.x,
    (float)p2.y,
    (float)(terrainHeight + height),
    wallLength / 8.0f,
    (float)(height / 8.0f)
});

    // Two triangles
    wallTriangles.push_back({base + 0, base + 1, base + 2});
wallTriangles.push_back({base + 1, base + 3, base + 2});
}


    // Roof
    for (int i = 1; i < n - 1; i++)
    {
        roofTriangles.push_back({offset + n + 0,
                         offset + n + i,
                         offset + n + i + 1});
    }

    
}

const std::vector<Vertex> &MeshBuilder::getVertices() const
{
    return vertices;
}

const std::vector<Triangle>& MeshBuilder::getWallTriangles() const
{
    return wallTriangles;
}

const std::vector<Triangle>& MeshBuilder::getRoofTriangles() const
{
    return roofTriangles;
}