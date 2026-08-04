#include <iostream>
#include <vector>
#include <iomanip>

#include "OsmReader.h"
#include "CoordinateConverter.h"
#include "MeshBuilder.h"
#include "OBJWriter.h"

int main()
{
    OsmReader reader;

    if (!reader.load("../data/in.osm"))
    {
        std::cout << "Cannot open OSM file" << std::endl;
        return 1;
    }

    const auto &nodes = reader.getNodes();
    const auto &buildings = reader.getBuildings();

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

    // ---------- Use first node as origin ----------
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

    std::cout << "\nOrigin (first node): "
              << firstNode->second.lat
              << ", "
              << firstNode->second.lon
              << std::endl;

    // ---------- Build mesh for ONLY Building 0 ----------
    // ---------- Build mesh for first 30 buildings ----------
MeshBuilder mesh;

int exportedCount = 0;

for (const Building &b : buildings)
{
    if (exportedCount >= 100)
        break;


    std::vector<Point2D> polygon;


    for (long long id : b.nodeIds)
    {
        auto it = nodes.find(id);

        if (it == nodes.end())
            continue;


        Point2D p = converter.toLocal(
            it->second.lat,
            it->second.lon
        );


        polygon.push_back(p);
    }


    // invalid footprint
    if (polygon.size() < 3)
        continue;


    double buildingHeight = 10.0;


    if (b.height > 0)
        buildingHeight = b.height;
    else if (b.levels > 0)
        buildingHeight = b.levels * 3.0;


    mesh.appendExtrudedBuilding(
        polygon,
        buildingHeight
    );


    std::cout
        << "Added building "
        << exportedCount
        << std::endl;


    exportedCount++;
}


std::cout
    << "Total exported buildings: "
    << exportedCount
    << std::endl;

    // ---------- Export OBJ ----------
    OBJWriter writer;

    if (writer.writeOBJ(mesh, "../output/building30_exact.obj"))
    {
        std::cout << "\nOBJ written successfully!" << std::endl;
        std::cout << "File: ../output/building0_exact.obj" << std::endl;
    }
    else
    {
        std::cout << "\nFailed to write OBJ!" << std::endl;
    }

    return 0;
}