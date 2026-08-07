#include "TileBuilder.h"
#include <iostream>
#include <cmath>
#include <unordered_map>

struct TileKey
{
    int x;
    int y;

    bool operator==(const TileKey& other) const
    {
        return x == other.x && y == other.y;
    }
};

struct TileKeyHash
{
    size_t operator()(const TileKey& k) const
    {
        return (std::hash<int>{}(k.x) << 1) ^ std::hash<int>{}(k.y);
    }
};

std::vector<Tile> TileBuilder::buildTiles(
    const std::vector<Building>& buildings,
    const std::unordered_map<long long, Node>& nodes,
    double tileSizeMeters)
{
    std::vector<Tile> result;

    if (buildings.empty() || nodes.empty())
        return result;

    // Global origin (south-west corner of the dataset)
    double minLat = 1e30;
    double minLon = 1e30;

    for (const auto& kv : nodes)
    {
        minLat = std::min(minLat, kv.second.lat);
        minLon = std::min(minLon, kv.second.lon);
    }

    const double metersPerDegLat = 111320.0;
    const double metersPerDegLon =
        111320.0 * std::cos(minLat * 3.14159265358979323846 / 180.0);

    std::unordered_map<TileKey, int, TileKeyHash> tileMap;

    for (size_t i = 0; i < buildings.size(); ++i)
    {
        if (buildings[i].nodeIds.empty())
{
    std::cout << "Building " << i << " has no nodes\n";
    continue;
}

auto it = nodes.find(buildings[i].nodeIds.front());

if (it == nodes.end())
{
    std::cout << "Building " << i
              << " first node missing: "
              << buildings[i].nodeIds.front()
              << std::endl;
    continue;
}

        double dx = (it->second.lon - minLon) * metersPerDegLon;
        double dy = (it->second.lat - minLat) * metersPerDegLat;

        int tx = static_cast<int>(std::floor(dx / tileSizeMeters));
        int ty = static_cast<int>(std::floor(dy / tileSizeMeters));

        TileKey key{tx, ty};

        auto found = tileMap.find(key);

        if (found == tileMap.end())
        {
            Tile tile;

            tile.minLon = minLon + (tx * tileSizeMeters) / metersPerDegLon;
            tile.maxLon = minLon + ((tx + 1) * tileSizeMeters) / metersPerDegLon;

            tile.minLat = minLat + (ty * tileSizeMeters) / metersPerDegLat;
            tile.maxLat = minLat + ((ty + 1) * tileSizeMeters) / metersPerDegLat;

            tile.centerLat = (tile.minLat + tile.maxLat) * 0.5;
            tile.centerLon = (tile.minLon + tile.maxLon) * 0.5;

            tile.buildingIndices.push_back((int)i);

            tileMap[key] = (int)result.size();
            result.push_back(tile);
        }
        else
        {
            result[found->second].buildingIndices.push_back((int)i);
        }
    }

    return result;
}