#include <iostream>
#include <vector>
#include "OsmReader.h"
#include "OBJWriter.h"
#include "CoordinateConverter.h"
#include "MeshBuilder.h"
#include "B3DMWriter.h"
// #include "B3DMWriter.h"
 #include "TilesetWriter.h"


int main()
{
    OsmReader reader;

    if (!reader.load("E:/C/osm_to_glb_cpp/data/map.osm"))
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

    // Use the first building's first node as the global origin
    const Building &firstBuilding = buildings[0];
    const Node &globalOrigin = nodes.at(firstBuilding.nodeIds[0]);

    CoordinateConverter converter(
        globalOrigin.lat,
        globalOrigin.lon);

    std::cout << "Buildings: "
              << buildings.size()
              << std::endl;

    

    MeshBuilder mesh;
   
    for (const Building &b : buildings)
    {

        if (b.nodeIds.size() < 3)
            continue;

        std::vector<Point2D> polygon;

        for (long long id : b.nodeIds)
        {
            auto it = nodes.find(id);

            if (it == nodes.end())
                continue;

            polygon.push_back(
                converter.toLocal(it->second.lat, it->second.lon));
        }

        if (polygon.size() < 3)
        {
            std::cout << "Skipping invalid building with "
                      << polygon.size()
                      << " points"
                      << std::endl;
            continue;
        }

        std::cout << "Building polygon points: "
                  << polygon.size()
                  << std::endl;

        mesh.appendExtrudedBuilding(
            polygon,
            12.0);

        std::cout << "Total vertices: "
                  << mesh.getVertices().size()
                  << std::endl;

        std::cout << "Total triangles: "
                  << mesh.getTriangles().size()
                  << std::endl;
    }

    float minX = 1e9f;
    float minY = 1e9f;
    float minZ = 1e9f;

    for (const auto &v : mesh.getVertices())
    {
        if (v.x < minX)
            minX = v.x;
        if (v.y < minY)
            minY = v.y;
        if (v.z < minZ)
            minZ = v.z;
    }

    std::cout << "Minimum coordinates: "
              << minX << " "
              << minY << " "
              << minZ
              << std::endl;

    OBJWriter objWriter;

    if (objWriter.writeOBJ(mesh, "E:/C/osm_to_glb_cpp/output/delhi_block.obj"))
    {
        std::cout << "Delhi OBJ written successfully!" << std::endl;
    }
    else
    {
        std::cout << "Failed to write Delhi OBJ." << std::endl;
    }

    std::cout << "Vertices: "
              << mesh.getVertices().size()
              << std::endl;

    std::cout << "Triangles: "
              << mesh.getTriangles().size()
              << std::endl;

    int i = 0;

    for (const auto &v : mesh.getVertices())
    {
        std::cout
            << "V "
            << i++
            << " : "
            << v.x
            << " "
            << v.y
            << " "
            << v.z
            << std::endl;

        if (i == 10)
            break;
    }






// // Write B3DM
// B3DMWriter b3dm;

// if (b3dm.writeB3DM(
//         "E:/C/osm_to_glb_cpp/output/delhi_block.glb",
//         "E:/C/osm_to_glb_cpp/output/tile.b3dm"
//     ))
// {
//     std::cout << "B3DM written successfully!" << std::endl;
// }
// else
// {
//     std::cout << "Failed to write B3DM." << std::endl;
// }

// TilesetWriter tileset;

// if (tileset.writeTileset(
//         mesh,
//         globalOrigin.lat,
//         globalOrigin.lon,
//         "E:/C/osm_to_glb_cpp/output/tileset.json"
//     ))
// {
//     std::cout << "Tileset written successfully!" << std::endl;
// }
// else
// {
//     std::cout << "Failed to write tileset." << std::endl;
// }


B3DMWriter b3dm;

if (b3dm.writeB3DM(
        "E:/C/osm_to_glb_cpp/output/one.glb",
        "E:/C/osm_to_glb_cpp/output/tile.b3dm"
    ))
{
    std::cout << "B3DM written successfully!" << std::endl;
}
else
{
    std::cout << "Failed to write B3DM." << std::endl;
}

TilesetWriter tileset;

if (tileset.writeTileset(
        mesh,
        globalOrigin.lat,
        globalOrigin.lon,
        "E:/C/osm_to_glb_cpp/output/tileset.json"
    ))
{
    std::cout << "Tileset written successfully!" << std::endl;
}
else
{
    std::cout << "Failed to write tileset." << std::endl;
}

return 0;

   
}
