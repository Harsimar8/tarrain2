#include <iostream>
#include <vector>
#include <iomanip>
#include "B3DMWriter.h"
#include "TilesetWriter.h"
#include "OsmReader.h"
#include "CoordinateConverter.h"
#include "MeshBuilder.h"
#include "OBJWriter.h"
#include <map>
#include <cmath>


int main()
{
    OsmReader reader;

    if (!reader.load("../data/de.osm"))
    {
        std::cout << "Cannot open OSM file" << std::endl;
        return 1;
    }

    const auto &nodes = reader.getNodes();
    const auto &buildings = reader.getBuildings();



TileBuilder tileBuilder;
auto tiles = tileBuilder.buildTiles(buildings, nodes, 250.0); // 250 m tiles

std::cout << "Total buildings: " << buildings.size() << std::endl;
std::cout << "Total tiles: " << tiles.size() << std::endl;

size_t totalAssigned = 0;

for (size_t i = 0; i < tiles.size(); ++i)
{
    const Tile& t = tiles[i];

    totalAssigned += t.buildingIndices.size();

    std::cout
        << "Tile " << i
        << " | center ("
        << t.centerLat
        << ", "
        << t.centerLon
        << ") | buildings: "
        << t.buildingIndices.size()
        << std::endl;
}

std::cout
    << "Total assigned buildings: "
    << totalAssigned
    << std::endl;



    if (buildings.empty())
    {
        std::cout << "No buildings found" << std::endl;
        return 0;
    }

    // ---------- Print exact OSM footprint ----------
    std::cout << std::fixed << std::setprecision(8);

    std::cout << "Building 0 footprint (exact OSM coordinates)" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;

    for (size_t i = 0; i < buildings[0].nodeIds.size(); i++)
    {
        long long id = buildings[0].nodeIds[i];
        auto it = nodes.find(id);

        if (it == nodes.end())
            continue;

        std::cout
            << i
            << " | lat=" << it->second.lat
            << " | lon=" << it->second.lon
            << std::endl;
    }

    // ---------- Compute OSM center as origin ----------
double minLat =  1e30;
double maxLat = -1e30;
double minLon =  1e30;
double maxLon = -1e30;

for (const auto& kv : nodes)
{
    minLat = std::min(minLat, kv.second.lat);
    maxLat = std::max(maxLat, kv.second.lat);
    minLon = std::min(minLon, kv.second.lon);
    maxLon = std::max(maxLon, kv.second.lon);
}

double originLat = (minLat + maxLat) * 0.5;
double originLon = (minLon + maxLon) * 0.5;


return 0;
}

// CoordinateConverter converter(originLat, originLon);

// std::cout << "\nOSM center origin: "
//           << originLat
//           << ", "
//           << originLon
//           << std::endl;
//     // ---------- Build mesh for ONLY Building 0 ----------
//     // ---------- Build mesh for first 30 buildings ----------
// MeshBuilder mesh;

// int exportedCount = 0;

// for (const Building &b : buildings)
// {
//     if (exportedCount >= 1000)
//         break;


//     std::vector<Point2D> polygon;


//     for (long long id : b.nodeIds)
//     {
//         auto it = nodes.find(id);

//         if (it == nodes.end())
//             continue;


//         Point2D p = converter.toLocal(
//             it->second.lat,
//             it->second.lon
//         );


//         polygon.push_back(p);
//     }


//     // invalid footprint
//     if (polygon.size() < 3)
//         continue;


//     double buildingHeight = 10.0;


//     if (b.height > 0)
//         buildingHeight = b.height;
//     else if (b.levels > 0)
//         buildingHeight = b.levels * 3.0;


//     mesh.appendExtrudedBuilding(
//         polygon,
//         buildingHeight
//     );


//     std::cout
//         << "Added building "
//         << exportedCount
//         << std::endl;


//     exportedCount++;
// }


// std::cout
//     << "Total exported buildings: "
//     << exportedCount
//     << std::endl;



// B3DMWriter b3dmWriter;


// bool b3dmOK =
// b3dmWriter.writeB3DM(
//     "../data/DD3.glb",
//     "../output/tile.b3dm"
// );


// if(b3dmOK)
// {
//     std::cout
//     << "B3DM created\n";
// }
// else
// {
//     std::cout
//     << "B3DM failed\n";
// }



// // ---------- Generate tileset.json ----------

// TilesetWriter tilesWriter;


// bool tilesOK =
// tilesWriter.writeTileset(
//     mesh,
//     originLat,
//     originLon,
//     "../output/tileset.json"
// );



// if(tilesOK)
// {
//     std::cout
//     << "tileset.json created\n";
// }
// else
// {
//     std::cout
//     << "tileset failed\n";
// }

//     // ---------- Export OBJ ----------
//     OBJWriter writer;

//     if (writer.writeOBJ(mesh, "../output/dehra_exact.obj"))
//     {
//         std::cout << "\nOBJ written successfully!" << std::endl;
//         std::cout << "File: ../output/dehra_exact.obj" << std::endl;
//     }
//     else
//     {
//         std::cout << "\nFailed to write OBJ!" << std::endl;
//     }

//     return 0;
// }