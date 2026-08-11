#pragma once

#include <vector>
#include "CoordinateConverter.h"

extern int gCurrentTileIndex;

struct Vertex
{
    float x;
    float y;
    float z;

    float u;
    float v;
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
        double height,
        double terrainHeight = 0.0
    );

    void appendExtrudedBuilding(
        const std::vector<Point2D>& polygon,
        double height,
        double terrainHeight = 0.0
    );

    const std::vector<Vertex>& getVertices() const;
    const std::vector<Triangle>& getTriangles() const;

private:
    std::vector<Vertex> vertices;
    std::vector<Triangle> wallTriangles;
std::vector<Triangle> roofTriangles;
};