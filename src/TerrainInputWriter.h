#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "TileBuilder.h"
#include "OsmReader.h"

void writeTileTerrainInput(
    const Tile& tile,
    const std::vector<Building>& buildings,
    const std::unordered_map<long long, Node>& nodes,
    const std::string& filename
);