#include "TilesetWriter.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>

static constexpr double DEG_TO_RAD = 3.14159265358979323846 / 180.0;
static constexpr double EARTH_RADIUS = 6378137.0;

bool TilesetWriter::writeTileset(
const MeshBuilder& mesh,
double originLat,
double originLon,
const std::string& filename
)
{
const auto& vertices = mesh.getVertices();

if (vertices.empty())
    return false;

double minX = vertices[0].x;
double minY = vertices[0].y;
double minZ = vertices[0].z;

double maxX = vertices[0].x;
double maxY = vertices[0].y;
double maxZ = vertices[0].z;

for (const auto& v : vertices)
{
    if (v.x < minX) minX = v.x;
    if (v.y < minY) minY = v.y;
    if (v.z < minZ) minZ = v.z;

    if (v.x > maxX) maxX = v.x;
    if (v.y > maxY) maxY = v.y;
    if (v.z > maxZ) maxZ = v.z;
}

double halfWidth  = (maxX - minX) * 0.5;
double halfHeight = (maxY - minY) * 0.5;

double latDelta =
    halfHeight / EARTH_RADIUS;

double lonDelta =
    halfWidth /
    (EARTH_RADIUS * std::cos(originLat * DEG_TO_RAD));

double west  = (originLon * DEG_TO_RAD) - lonDelta;
double east  = (originLon * DEG_TO_RAD) + lonDelta;
double south = (originLat * DEG_TO_RAD) - latDelta;
double north = (originLat * DEG_TO_RAD) + latDelta;

std::ofstream out(filename);

if (!out)
    return false;

out << std::fixed << std::setprecision(10);

out << "{\n";
out << "  \"asset\": {\n";
out << "    \"version\": \"1.0\"\n";
out << "  },\n";
out << "  \"geometricError\": 500,\n";
out << "  \"root\": {\n";
out << "    \"boundingVolume\": {\n";
out << "      \"region\": ["
    << west << ", "
    << south << ", "
    << east << ", "
    << north << ", 0, "
    << (maxZ + 20.0)
    << "]\n";
out << "    },\n";
out << "    \"geometricError\": 0,\n";
out << "    \"refine\": \"ADD\",\n";
out << "    \"content\": {\n";
out << "      \"uri\": \"tile.b3dm\"\n";
out << "    }\n";
out << "  }\n";
out << "}\n";

out.close();
std::cout << "\n========== STEP 4 : TILESET ==========" << std::endl;
std::cout << "West  : " << west << std::endl;
std::cout << "South : " << south << std::endl;
std::cout << "East  : " << east << std::endl;
std::cout << "North : " << north << std::endl;
std::cout << "Max Z : " << maxZ << std::endl;
std::cout << "Tileset file : " << filename << std::endl;
std::cout << "=====================================\n" << std::endl;

return true;


}
