#include "MeshBuilder.h"
#include <cmath>
#include <iostream>
#include <algorithm>



static bool samePoint(const Point2D& a, const Point2D& b)
{
return std::fabs(a.x - b.x) < 0.001 &&
std::fabs(a.y - b.y) < 0.001;
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

// Remove nearly-collinear points
std::vector<Point2D> simplified;

for (size_t i = 0; i < clean.size(); i++)
{
    const Point2D& prev = clean[(i + clean.size() - 1) % clean.size()];
    const Point2D& curr = clean[i];
    const Point2D& next = clean[(i + 1) % clean.size()];

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
{
    std::reverse(clean.begin(), clean.end());
}

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

// Roof triangulation using centroid (prevents roof spikes)

// Roof triangulation using triangle fan
for (int i = 1; i < n - 1; i++)
{
    triangles.push_back({
        offset + n + 0,
        offset + n + i,
        offset + n + i + 1
    });
}


// Vertex center = {0, 0, (float)height};

// for (const auto& p : clean)
// {
//     center.x += (float)p.x;
//     center.y += (float)p.y;
// }

// center.x /= n;
// center.y /= n;

// int centerIndex = (int)vertices.size();
// vertices.push_back(center);

// for (int i = 0; i < n; i++)
// {
//     int next = (i + 1) % n;

//     triangles.push_back({
//         centerIndex,
//         offset + n + i,
//         offset + n + next
//     });
// }


}

const std::vector<Vertex>& MeshBuilder::getVertices() const
{
return vertices;
}

const std::vector<Triangle>& MeshBuilder::getTriangles() const
{
return triangles;
}
