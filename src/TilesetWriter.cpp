
#include "TilesetWriter.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <limits>
#include <vector>

// --------------------------------------------------
// WGS84 constants
// --------------------------------------------------
static constexpr double PI = 3.14159265358979323846;

static constexpr double WGS84_A = 6378137.0;
static constexpr double WGS84_B = 6356752.314245;

static constexpr double WGS84_E2 =
    (WGS84_A * WGS84_A - WGS84_B * WGS84_B) /
    (WGS84_A * WGS84_A);

// --------------------------------------------------
// ECEF position for one tile
// --------------------------------------------------
struct ECEF
{
    double x;
    double y;
    double z;
};

// --------------------------------------------------
// Write tileset
// --------------------------------------------------
bool TilesetWriter::writeTileset(
    const std::vector<Tile>& tiles,
    const std::vector<double>& tileElevations,
    const std::string& filename)
{
    if (tiles.empty())
    {
        std::cerr << "ERROR: No tiles to write.\n";
        return false;
    }

    if (tileElevations.size() != tiles.size())
    {
        std::cerr
            << "ERROR: tiles.size() = "
            << tiles.size()
            << ", tileElevations.size() = "
            << tileElevations.size()
            << "\n";

        return false;
    }

    // --------------------------------------------------
    // FIRST PASS:
    // Calculate ECEF position for every tile.
    // --------------------------------------------------

    std::vector<ECEF> ecefPositions;
    ecefPositions.reserve(tiles.size());

    double minX =  std::numeric_limits<double>::max();
    double minY =  std::numeric_limits<double>::max();
    double minZ =  std::numeric_limits<double>::max();

    double maxX = -std::numeric_limits<double>::max();
    double maxY = -std::numeric_limits<double>::max();
    double maxZ = -std::numeric_limits<double>::max();

    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "        TILE ECEF DEBUG INFORMATION\n";
    std::cout << "========================================\n";

    for (size_t i = 0; i < tiles.size(); ++i)
    {
        const Tile& tile = tiles[i];

        double latDeg = tile.centerLat;
        double lonDeg = tile.centerLon;
        double h      = tileElevations[i];

        double lat = latDeg * PI / 180.0;
        double lon = lonDeg * PI / 180.0;

        double sinLat = std::sin(lat);
        double cosLat = std::cos(lat);
        double sinLon = std::sin(lon);
        double cosLon = std::cos(lon);

        double N =
            WGS84_A /
            std::sqrt(1.0 - WGS84_E2 * sinLat * sinLat);

        double x =
            (N + h) *
            cosLat *
            cosLon;

        double y =
            (N + h) *
            cosLat *
            sinLon;

        double z =
            (N * (1.0 - WGS84_E2) + h) *
            sinLat;

        ecefPositions.push_back({x, y, z});

        // --------------------------------------------------
        // Update overall ECEF bounds
        // --------------------------------------------------

        minX = std::min(minX, x);
        minY = std::min(minY, y);
        minZ = std::min(minZ, z);

        maxX = std::max(maxX, x);
        maxY = std::max(maxY, y);
        maxZ = std::max(maxZ, z);

        // --------------------------------------------------
        // DEBUG LOG
        // --------------------------------------------------

        
    }

    // --------------------------------------------------
    // Calculate root ECEF bounding box
    // --------------------------------------------------

    double rootCenterX = (minX + maxX) * 0.5;
    double rootCenterY = (minY + maxY) * 0.5;
    double rootCenterZ = (minZ + maxZ) * 0.5;

    // Add safety margin around all tiles.
    constexpr double ROOT_MARGIN = 2000.0;

    double rootHalfX =
        (maxX - minX) * 0.5 + ROOT_MARGIN;

    double rootHalfY =
        (maxY - minY) * 0.5 + ROOT_MARGIN;

    double rootHalfZ =
        (maxZ - minZ) * 0.5 + ROOT_MARGIN;

    

    // --------------------------------------------------
    // Open output file
    // --------------------------------------------------

    std::ofstream out(filename);

    if (!out)
    {
        std::cerr
            << "ERROR: Could not open tileset file: "
            << filename
            << "\n";

        return false;
    }

    out << std::fixed << std::setprecision(10);

    // --------------------------------------------------
    // JSON HEADER
    // --------------------------------------------------

    out << "{\n";

    out << "  \"asset\": {\n";
    out << "    \"version\": \"1.1\"\n";
    out << "  },\n";

    out << "  \"geometricError\": 1000,\n";

    out << "  \"root\": {\n";

    // --------------------------------------------------
    // IMPORTANT:
    //
    // Root is now an ECEF box instead of the old:
    //
    // [0,0,0,0,0,1000]
    // --------------------------------------------------

    out << "    \"boundingVolume\": {\n";

    out << "      \"box\": [\n";

    out << "        "
        << rootCenterX << ","
        << rootCenterY << ","
        << rootCenterZ << ",\n";

    out << "        "
        << rootHalfX << ",0,0,\n";

    out << "        0,"
        << rootHalfY
        << ",0,\n";

    out << "        0,0,"
        << rootHalfZ
        << "\n";

    out << "      ]\n";
    out << "    },\n";

    out << "    \"geometricError\": 500,\n";
    out << "    \"refine\": \"ADD\",\n";

    out << "    \"children\": [\n";

    // --------------------------------------------------
    // CHILD TILES
    // --------------------------------------------------

    for (size_t i = 0; i < tiles.size(); ++i)
    {
        const Tile& tile = tiles[i];

        double lat = tile.centerLat * PI / 180.0;
        double lon = tile.centerLon * PI / 180.0;
        double h   = tileElevations[i];

        double sinLat = std::sin(lat);
        double cosLat = std::cos(lat);
        double sinLon = std::sin(lon);
        double cosLon = std::cos(lon);

        double N =
            WGS84_A /
            std::sqrt(1.0 - WGS84_E2 * sinLat * sinLat);

        double x =
            (N + h) *
            cosLat *
            cosLon;

        double y =
            (N + h) *
            cosLat *
            sinLon;

        double z =
            (N * (1.0 - WGS84_E2) + h) *
            sinLat;

        // --------------------------------------------------
        // ENU basis
        // --------------------------------------------------

        double eastX  = -sinLon;
        double eastY  =  cosLon;
        double eastZ  =  0.0;

        double northX = -sinLat * cosLon;
        double northY = -sinLat * sinLon;
        double northZ =  cosLat;

        double upX    =  cosLat * cosLon;
        double upY    =  cosLat * sinLon;
        double upZ    =  sinLat;


        if (i == 538)
{
    std::cout << "==============================" << std::endl;
    std::cout << "TilesetWriter - Tile 538" << std::endl;
    std::cout << "Latitude : " << tile.centerLat << std::endl;
    std::cout << "Longitude: " << tile.centerLon << std::endl;
    std::cout << "Elevation: " << h << std::endl;
    std::cout << "ECEF X: " << x << std::endl;
    std::cout << "ECEF Y: " << y << std::endl;
    std::cout << "ECEF Z: " << z << std::endl;
    std::cout << "==============================" << std::endl;
}
        out << "      {\n";

        // --------------------------------------------------
        // TEMPORARY CHILD BOUNDING BOX
        //
        // We will fix this in STEP 2 after checking
        // the actual GLB vertex bounds.
        // --------------------------------------------------

        // Compute tile bounds from geographic tile size
double metersPerDegLat = 111320.0;
double metersPerDegLon = 111320.0 * std::cos(tile.centerLat * PI / 180.0);

double tileWidth  = (tile.maxLon - tile.minLon) * metersPerDegLon;
double tileHeight = (tile.maxLat - tile.minLat) * metersPerDegLat;

double halfX = tileWidth  * 0.5;
double halfY = tileHeight * 0.5;
double halfZ = 500.0;// enough for buildings

out << "        \"boundingVolume\": {\n";
out << "          \"box\": [";
out << "0,0," << halfZ << ",";
out << halfX << ",0,0,";
out << "0," << halfY << ",0,";
out << "0,0," << halfZ;
out << "]\n";
out << "        },\n";

        out << "        \"geometricError\": 0,\n";

        // --------------------------------------------------
        // Geographic/ECEF transform
        // --------------------------------------------------

        out << "        \"transform\": [\n";

        out << "          "
            << eastX << ","
            << eastY << ","
            << eastZ << ",0,\n";

        out << "          "
            << northX << ","
            << northY << ","
            << northZ << ",0,\n";

        out << "          "
            << upX << ","
            << upY << ","
            << upZ << ",0,\n";

        out << "          "
            << x << ","
            << y << ","
            << z << ",1\n";

        out << "        ],\n";

        // --------------------------------------------------
        // B3DM
        // --------------------------------------------------

        out << "        \"content\": {\n";

        out << "          \"uri\": \"b3dm/tile_"
            << i
            << ".b3dm\"\n";

        out << "        }\n";

        out << "      }";

        if (i + 1 != tiles.size())
            out << ",";

        out << "\n";
    }

    // --------------------------------------------------
    // Close JSON
    // --------------------------------------------------

    out << "    ]\n";
    out << "  }\n";
    out << "}\n";

    out.close();

    std::cout
        << "Root tileset written: "
        << filename
        << std::endl;

    return true;
}

