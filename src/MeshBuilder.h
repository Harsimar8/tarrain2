#pragma once

#include <vector>
#include "CoordinateConverter.h"

struct Vertex
{
    float x;
    float y;
    float z;
};

struct Triangle
{
    int a;
    int b;
    int c;
};

class MeshBuilder
{
public:
    void buildExtrudedBuilding(
        const std::vector<Point2D>& polygon,
        double height
    );
    void appendExtrudedBuilding(
    const std::vector<Point2D>& polygon,
    double height
);

    const std::vector<Vertex>& getVertices() const;
    const std::vector<Triangle>& getTriangles() const;

private:
    std::vector<Vertex> vertices;
    std::vector<Triangle> triangles;
};