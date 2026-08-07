#include "TileBuilder.h"

std::vector<Tile> TileBuilder::buildTiles(
    const std::vector<Building>& buildings,
    const std::unordered_map<long long, Node>& nodes,
    double tileSizeMeters)
{
    Tile tile;

    double sumLat = 0.0;
    double sumLon = 0.0;
    size_t count = 0;

    for (size_t i = 0; i < buildings.size(); ++i)
    {
        tile.buildingIndices.push_back((int)i);

        for (long long id : buildings[i].nodeIds)
        {
            auto it = nodes.find(id);
            if (it == nodes.end()) continue;

            sumLat += it->second.lat;
            sumLon += it->second.lon;
            count++;
        }
    }

    if (count > 0)
    {
        tile.centerLat = sumLat / count;
        tile.centerLon = sumLon / count;
    }

    return { tile };
}