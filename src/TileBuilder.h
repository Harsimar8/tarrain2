#pragma once

#include <vector>
#include "OsmReader.h"

struct Tile
{
    std::vector<int> buildingIndices;

    double centerLat = 0.0;
    double centerLon = 0.0;

    double minLat = 0.0;
    double minLon = 0.0;
    double maxLat = 0.0;
    double maxLon = 0.0;

    // NEW: terrain elevation at the tile center
    double terrainElevation = 0.0;
};

class TileBuilder
{
public:
    std::vector<Tile> buildTiles(
        const std::vector<Building>& buildings,
        const std::unordered_map<long long, Node>& nodes,
        double tileSizeMeters = 250.0
    );
};