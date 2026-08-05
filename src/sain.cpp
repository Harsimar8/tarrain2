#include <iostream>
#include <vector>
#include <iomanip>
#include <map>
#include <cmath>

#include "B3DMWriter.h"
#include "TilesetWriter.h"
#include "OsmReader.h"
#include "CoordinateConverter.h"
#include "MeshBuilder.h"
#include "OBJWriter.h"

struct TileKey
{
    int x;
    int y;

    bool operator<(const TileKey& other) const
    {
        if (x != other.x) return x < other.x;
        return y < other.y;
    }
};

int main()
{
    OsmReader reader;

    if (!reader.load("../data/de.osm"))
    {
        std::cout << "Cannot open OSM file" << std::endl;
        return 1;
    }

    const auto& nodes = reader.getNodes();
    const auto& buildings = reader.getBuildings();

    if (buildings.empty())
    {
        std::cout << "No buildings found" << std::endl;
        return 0;
    }

    std::cout << std::fixed << std::setprecision(8);

    long long firstNodeId = buildings[0].nodeIds[0];
    auto firstNode = nodes.find(firstNodeId);

    if (firstNode == nodes.end())
    {
        std::cout << "First node not found" << std::endl;
        return 1;
    }

    CoordinateConverter converter(
        firstNode->second.lat,
        firstNode->second.lon);

    std::cout << "Origin: "
              << firstNode->second.lat
              << ", "
              << firstNode->second.lon
              << std::endl;

    // --------------------------------------------------
    // Split buildings into 100m x 100m tiles
    // --------------------------------------------------

    const double TILE_SIZE = 100.0;

    std::map<TileKey, MeshBuilder> tileMeshes;

    int exportedCount = 0;

    for (const Building& b : buildings)
    {
        if (exportedCount >= 1000)
            break;

        std::vector<Point2D> polygon;

        double sumX = 0.0;
        double sumY = 0.0;

        for (long long id : b.nodeIds)
        {
            auto it = nodes.find(id);

            if (it == nodes.end())
                continue;

            Point2D p = converter.toLocal(
                it->second.lat,
                it->second.lon);

            polygon.push_back(p);

            sumX += p.x;
            sumY += p.y;
        }

        if (polygon.size() < 3)
            continue;

        double centerX = sumX / polygon.size();
        double centerY = sumY / polygon.size();

        int tileX = static_cast<int>(std::floor(centerX / TILE_SIZE));
        int tileY = static_cast<int>(std::floor(centerY / TILE_SIZE));

        double buildingHeight = 10.0;

        if (b.height > 0)
            buildingHeight = b.height;
        else if (b.levels > 0)
            buildingHeight = b.levels * 3.0;

        TileKey key{tileX, tileY};

        tileMeshes[key].appendExtrudedBuilding(
            polygon,
            buildingHeight);

        exportedCount++;
    }

    std::cout << "Total exported buildings: "
              << exportedCount
              << std::endl;

    std::cout << "Generated tiles: "
              << tileMeshes.size()
              << std::endl;

    // --------------------------------------------------
    // Temporary OBJ export (first tile only)
    // --------------------------------------------------

    if (!tileMeshes.empty())
    {
        OBJWriter writer;

        writer.writeOBJ(
            tileMeshes.begin()->second,
            "../output/dehra_exact.obj");
    }

    // --------------------------------------------------
    // Tileset generation will be implemented next
    // --------------------------------------------------

    return 0;
}