#include "TilesetWriter.h"

#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>


static constexpr double DEG_TO_RAD =
    3.14159265358979323846 / 180.0;

static constexpr double EARTH_RADIUS =
    6378137.0;



bool TilesetWriter::writeTileset(
    const MeshBuilder& mesh,
    double originLat,
    double originLon,
    const std::string& filename
)
{

    const auto& vertices = mesh.getVertices();


    if(vertices.empty())
        return false;



    // -----------------------------
    // Local bounding box
    // -----------------------------

    double minX = vertices[0].x;
    double minY = vertices[0].y;
    double minZ = vertices[0].z;

    double maxX = vertices[0].x;
    double maxY = vertices[0].y;
    double maxZ = vertices[0].z;



    for(const auto& v : vertices)
    {

        minX = std::min(minX,v.x);
        minY = std::min(minY,v.y);
        minZ = std::min(minZ,v.z);

        maxX = std::max(maxX,v.x);
        maxY = std::max(maxY,v.y);
        maxZ = std::max(maxZ,v.z);

    }



    double centerX =
        (minX + maxX) * 0.5;

    double centerY =
        (minY + maxY) * 0.5;

    double centerZ =
        (minZ + maxZ) * 0.5;



    double radius =
        std::sqrt(
            (maxX-minX)*(maxX-minX)
            +
            (maxY-minY)*(maxY-minY)
            +
            (maxZ-minZ)*(maxZ-minZ)
        );



    std::ofstream out(filename);


    if(!out)
        return false;



    out
    << std::fixed
    << std::setprecision(12);



    out << "{\n";



    out <<
    "  \"asset\": {\n"
    "    \"version\": \"1.0\"\n"
    "  },\n";



    out <<
    "  \"geometricError\": 500,\n";



    out <<
    "  \"root\": {\n";



    /*
        Root transform

        This replaces:

        Cesium Matrix4 ENU
        +
        Z rotation -90

        Later we can make this full
        ECEF transform.
    */


    double angle =
        -90.0 * DEG_TO_RAD;



    double c = cos(angle);
    double s = sin(angle);



    out <<
    "    \"transform\": [\n"
    << "      "
    << c << ", "
    << s << ", "
    << "0, 0,\n"

    << "      "
    << -s << ", "
    << c << ", "
    << "0, 0,\n"

    << "      "
    << "0, 0, 1, 0,\n"

    << "      "
    << "0, 0, 0, 1\n"

    << "    ],\n";



    // Bounding sphere

    out <<
    "    \"boundingVolume\": {\n"
    "      \"sphere\": [\n"
    << "        "
    << centerX << ", "
    << centerY << ", "
    << centerZ << ", "
    << radius
    << "\n"
    "      ]\n"
    "    },\n";



    out <<
    "    \"geometricError\": 0,\n"
    "    \"refine\": \"ADD\",\n";



    out <<
    "    \"content\": {\n"
    "      \"uri\": \"tile.b3dm\"\n"
    "    }\n";



    out <<
    "  }\n"
    "}\n";



    out.close();



    std::cout
    << "\n========== STEP 4 : TILESET ==========\n"
    << "Origin latitude : "
    << originLat
    << "\nOrigin longitude : "
    << originLon
    << "\nCenter X : "
    << centerX
    << "\nCenter Y : "
    << centerY
    << "\nCenter Z : "
    << centerZ
    << "\nRadius : "
    << radius
    << "\nTileset : "
    << filename
    << "\n=====================================\n";



    return true;
}