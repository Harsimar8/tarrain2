#include "MeshBuilder.h"
#include <cmath>
#include <iostream>

static bool samePoint(const Point2D& a, const Point2D& b)
{
return std::fabs(a.x - b.x) < 0.001 &&
std::fabs(a.y - b.y) < 0.001;
}

void MeshBuilder::buildExtrudedBuilding(
const std::vector<Point2D>& polygon,
double height
)
{
vertices.clear();
triangles.clear();
appendExtrudedBuilding(polygon, height);
}

void MeshBuilder::appendExtrudedBuilding(
const std::vector<Point2D>& polygon,
double height
)
{
std::vector<Point2D> clean;

for (const auto& p : polygon)
{
    if (clean.empty() || !samePoint(clean.back(), p))
        clean.push_back(p);
}

if (clean.size() >= 2 && samePoint(clean.front(), clean.back()))
    clean.pop_back();

int n = static_cast<int>(clean.size());

if (n < 3)
    return;

int offset = static_cast<int>(vertices.size());

for (const auto& p : clean)
{
    vertices.push_back({
        (float)p.x,
        (float)p.y,
        0.0f
    });
}

for (const auto& p : clean)
{
    vertices.push_back({
        (float)p.x,
        (float)p.y,
        (float)height
    });
}

for (int i = 0; i < n; i++)
{
    int next = (i + 1) % n;

    int g1 = offset + i;
    int g2 = offset + next;
    int r1 = offset + i + n;
    int r2 = offset + next + n;

    triangles.push_back({g1, g2, r1});
    triangles.push_back({g2, r2, r1});
}

for (int i = 1; i < n - 1; i++)
{
    triangles.push_back({
        offset + n,
        offset + n + i,
        offset + n + i + 1
    });
}


}

const std::vector<Vertex>& MeshBuilder::getVertices() const
{
return vertices;
}

const std::vector<Triangle>& MeshBuilder::getTriangles() const
{
return triangles;
}
