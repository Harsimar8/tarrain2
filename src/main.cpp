// #include <iostream>
// #include <vector>
// #include "OsmReader.h"
// #include "OBJWriter.h"
// #include "CoordinateConverter.h"
// #include "MeshBuilder.h"
// #include "B3DMWriter.h"
// // #include "B3DMWriter.h"
//  #include "TilesetWriter.h"


// int main()
// {
//     OsmReader reader;

//     if (!reader.load("../data/in.osm"))
//     {
//         std::cout << "Cannot open OSM file" << std::endl;
//         return 1;
//     }

//     const auto &nodes = reader.getNodes();
//     const auto &buildings = reader.getBuildings();

//     if (buildings.empty())
//     {
//         std::cout << "No buildings found" << std::endl;
//         return 0;
//     }

//     // Use the first building's first node as the global origin
//     // Compute the center of all OSM nodes (better global origin)

// // Compute bounding box center (Method 2)

// double minLat =  1e9;
// double maxLat = -1e9;
// double minLon =  1e9;
// double maxLon = -1e9;

// for (const auto &pair : nodes)
// {
//     const Node &n = pair.second;

//     if (n.lat < minLat) minLat = n.lat;
//     if (n.lat > maxLat) maxLat = n.lat;

//     if (n.lon < minLon) minLon = n.lon;
//     if (n.lon > maxLon) maxLon = n.lon;
// }

// Node globalOrigin;
// globalOrigin.lat = (minLat + maxLat) / 2.0;
// globalOrigin.lon = (minLon + maxLon) / 2.0;

// CoordinateConverter converter(
//     globalOrigin.lat,
//     globalOrigin.lon
// );

// std::cout << "Bounding box origin: "
//           << globalOrigin.lat << ", "
//           << globalOrigin.lon << std::endl;

// std::cout << "Buildings found: "
//           << buildings.size()
//           << std::endl;

// for (int i = 0; i < 5 && i < buildings.size(); i++)
// {
//     std::cout << "Building " << i
//               << " height=" << buildings[i].height
//               << " levels=" << buildings[i].levels
//               << std::endl;
// }


    

//     MeshBuilder mesh;
   
//     for (const Building &b : buildings)
//     {

//         if (b.nodeIds.size() < 3)
//             continue;

//         std::vector<Point2D> polygon;

//         for (long long id : b.nodeIds)
//         {
//             auto it = nodes.find(id);

//             if (it == nodes.end())
//                 continue;

//             polygon.push_back(
//                 converter.toLocal(it->second.lat, it->second.lon));
//         }

//         if (polygon.size() < 3)
//         {
//             std::cout << "Skipping invalid building with "
//                       << polygon.size()
//                       << " points"
//                       << std::endl;
//             continue;
//         }

//         std::cout << "Building polygon points: "
//                   << polygon.size()
//                   << std::endl;

//         double buildingHeight;

// if (b.height > 0)
// {
//     // Use the exact OSM height
//     buildingHeight = b.height;
// }
// else if (b.levels > 0)
// {
//     // Estimate from number of floors
//     buildingHeight = b.levels * 3.0;
// }
// else
// {
//     // Default when no information exists
//     buildingHeight = 10.0;
// }

// mesh.appendExtrudedBuilding(
//     polygon,
//     buildingHeight);

// std::cout << "Building height used: "
//           << buildingHeight
//           << " m"
//           << std::endl;

//         std::cout << "Total vertices: "
//                   << mesh.getVertices().size()
//                   << std::endl;

//         std::cout << "Total triangles: "
//                   << mesh.getTriangles().size()
//                   << std::endl;
//     }

//     float minX = 1e9f;
//     float minY = 1e9f;
//     float minZ = 1e9f;
 

//     float maxX = -1e9f;
// float maxY = -1e9f;
// float maxZ = -1e9f;



//     for (const auto &v : mesh.getVertices())
//     {
//         if (v.x < minX)
//             minX = v.x;
//         if (v.y < minY)
//             minY = v.y;
//         if (v.z < minZ)
//             minZ = v.z;


//             if (v.x > maxX) maxX = v.x;
//     if (v.y > maxY) maxY = v.y;
//     if (v.z > maxZ) maxZ = v.z;
//     }

//     std::cout << "Minimum coordinates: "
//               << minX << " "
//               << minY << " "
//               << minZ
//               << std::endl;


//     return 0;          

// //     OBJWriter objWriter;

// //     if (objWriter.writeOBJ(mesh, "../output/delhi_block.obj"))
// // {
// //     std::cout << "Delhi OBJ written successfully!" << std::endl;
// // }
// // else
// // {
// //     std::cout << "Failed to write Delhi OBJ." << std::endl;
// // }




// }



#include <iostream>
#include <vector>
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

// -------- Bounding-box origin (Method 2) --------

double minLat = 1e9;
double maxLat = -1e9;
double minLon = 1e9;
double maxLon = -1e9;

for (const auto &pair : nodes)
{
    const Node &n = pair.second;

    if (n.lat < minLat) minLat = n.lat;
    if (n.lat > maxLat) maxLat = n.lat;

    if (n.lon < minLon) minLon = n.lon;
    if (n.lon > maxLon) maxLon = n.lon;
}

Node globalOrigin;
globalOrigin.lat = (minLat + maxLat) / 2.0;
globalOrigin.lon = (minLon + maxLon) / 2.0;

CoordinateConverter converter(globalOrigin.lat, globalOrigin.lon);

std::cout << "Bounding box origin: "
          << globalOrigin.lat << ", "
          << globalOrigin.lon << std::endl;

std::cout << "Buildings found: "
          << buildings.size()
          << std::endl;

// -------- Print only first 5 buildings --------

std::cout << "\\nFirst 5 buildings:" << std::endl;

for (int i = 0; i < 5 && i < buildings.size(); i++)
{
    double finalHeight;

    if (buildings[i].height > 0)
        finalHeight = buildings[i].height;
    else if (buildings[i].levels > 0)
        finalHeight = buildings[i].levels * 3.0;
    else
        finalHeight = 10.0;

    std::cout
        << "Building " << i
        << " | OSM height=" << buildings[i].height
        << " | levels=" << buildings[i].levels
        << " | final height=" << finalHeight << " m"
        << std::endl;
}

// -------- Build mesh for all buildings --------

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
        continue;

    double buildingHeight;

    if (b.height > 0)
        buildingHeight = b.height;
    else if (b.levels > 0)
        buildingHeight = b.levels * 3.0;
    else
        buildingHeight = 10.0;

    mesh.appendExtrudedBuilding(
        polygon,
        buildingHeight);
}

// -------- Mesh summary --------

float minX = 1e9f;
float minY = 1e9f;
float minZ = 1e9f;

float maxX = -1e9f;
float maxY = -1e9f;
float maxZ = -1e9f;

for (const auto &v : mesh.getVertices())
{
    if (v.x < minX) minX = v.x;
    if (v.y < minY) minY = v.y;
    if (v.z < minZ) minZ = v.z;

    if (v.x > maxX) maxX = v.x;
    if (v.y > maxY) maxY = v.y;
    if (v.z > maxZ) maxZ = v.z;
}



std::cout << "Minimum coordinates: "
          << minX << " "
          << minY << " "
          << minZ
          << std::endl;

OBJWriter objWriter;

if (objWriter.writeOBJ(mesh, "../output/delhi_block.obj"))
{
    std::cout << "\nOBJ file written successfully!" << std::endl;
    std::cout << "File: ../output/delhi_block.obj" << std::endl;
}
else
{
    std::cout << "\nFailed to write OBJ file!" << std::endl;
}

std::cout << "\nMesh generation complete" << std::endl;
std::cout << "Vertices  : " << mesh.getVertices().size() << std::endl;
std::cout << "Triangles : " << mesh.getTriangles().size() << std::endl;
std::cout << "Bounds X  : " << minX << " to " << maxX << std::endl;
std::cout << "Bounds Y  : " << minY << " to " << maxY << std::endl;
std::cout << "Bounds Z  : " << minZ << " to " << maxZ << std::endl;


return 0;




}

