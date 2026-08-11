#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <regex>
#include "GLTFWriter.h"
#include "OsmReader.h"
#include "TileBuilder.h"
#include "TerrainInputWriter.h"
#include "CoordinateConverter.h"
#include "MeshBuilder.h"
#include "B3DMWriter.h"
#include "TilesetWriter.h"
#include "iomanip"


// --------------------------------------------------
// Terrain data
// --------------------------------------------------
struct TerrainData
{
    double tileCenterElevation = 0.0;
    std::unordered_map<int, double> buildingElevations;
};


TerrainData loadTerrainData(const std::string &filename)
{
    TerrainData data;

    std::ifstream file(filename);

    if (!file.is_open())
    {
        std::cout << "Cannot open " << filename << std::endl;
        return data;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    std::string text = buffer.str();

    std::regex centerRegex(
        R"REGEX("tileCenterElevation"\s*:\s*([-0-9.]+))REGEX"
    );

    std::smatch match;

    if (std::regex_search(text, match, centerRegex))
    {
        data.tileCenterElevation = std::stod(match[1]);
    }

    // FIXED REGEX
    std::regex entryRegex(
        R"REGEX("([0-9]+)"\s*:\s*([-0-9.]+))REGEX"
    );

    auto begin =
        std::sregex_iterator(
            text.begin(),
            text.end(),
            entryRegex
        );

    auto end = std::sregex_iterator();

    for (auto it = begin; it != end; ++it)
    {
        int id = std::stoi((*it)[1]);
        double h = std::stod((*it)[2]);

        data.buildingElevations[id] = h;
    }

    return data;
}


int main()
{
    OsmReader reader;

    if (!reader.load("../data/dehradun_buildings.osm"))
    {
        std::cout << "Cannot open OSM file" << std::endl;
        return 1;
    }

    const auto &nodes = reader.getNodes();
    const auto &buildings = reader.getBuildings();

    TileBuilder tileBuilder;
    auto tiles = tileBuilder.buildTiles(buildings, nodes, 250.0);

    std::cout << "Total buildings: " << buildings.size() << std::endl;
    std::cout << "Total tiles: " << tiles.size() << std::endl;

    
    size_t totalAssigned = 0;
    std::vector<double> tileElevations(tiles.size(), 0.0);


    for (size_t tileIndex = 0; tileIndex < tiles.size(); ++tileIndex)
    {
        const Tile &tile = tiles[tileIndex];

        totalAssigned += tile.buildingIndices.size();

        std::string elevationFile =
            "../output/elevations/tile_" +
            std::to_string(tileIndex) +
            "_elevations.json";

        TerrainData terrain =
            loadTerrainData(elevationFile);


         tileElevations[tileIndex] = terrain.tileCenterElevation;   
          

        CoordinateConverter converter(
            tile.centerLat,
            tile.centerLon);

       
        MeshBuilder mesh;
        gCurrentTileIndex = static_cast<int>(tileIndex);

        for (int buildingIndex : tile.buildingIndices)
        {
            const Building &b = buildings[buildingIndex];

            std::vector<Point2D> polygon;

            for (long long nodeId : b.nodeIds)
            {
                auto it = nodes.find(nodeId);

                if (it == nodes.end())
                    continue;

            Point2D pt = converter.toLocal(
    it->second.lat,
    it->second.lon);

polygon.push_back(pt);



            }

            if (polygon.size() < 3)
                continue;

            double h = 10.0;

            if (b.height > 0)
                h = b.height;
            else if (b.levels > 0)
                h = b.levels * 3.0;


            double buildingTerrain = terrain.tileCenterElevation;

            auto e = terrain.buildingElevations.find(b.id);


            

if (e != terrain.buildingElevations.end())
    buildingTerrain = e->second;

            double shiftedTerrain =
                buildingTerrain -
                terrain.tileCenterElevation;

                

            mesh.appendExtrudedBuilding(
                polygon,
                h,
                shiftedTerrain);
        }

        
        if (mesh.getVertices().empty())
    continue;

// -----------------------------
// GLB
// -----------------------------

GLTFWriter gltfWriter;

std::string glbFile =
    "../output/glb/tile_" +
    std::to_string(tileIndex) +
    ".glb";

if (!gltfWriter.writeGLB(mesh, glbFile))
{
    std::cout
        << "Failed GLB: "
        << glbFile
        << std::endl;

    continue;
}

// -----------------------------
// B3DM
// -----------------------------

B3DMWriter b3dmWriter;

std::string b3dmFile =
    "../output/b3dm/tile_" +
    std::to_string(tileIndex) +
    ".b3dm";

if (!b3dmWriter.writeB3DM(glbFile, b3dmFile))
{
    std::cout
        << "Failed B3DM: "
        << b3dmFile
        << std::endl;

    continue;
}





    }

    std::cout
        << "Total assigned buildings: "
        << totalAssigned
        << std::endl;

        TilesetWriter tilesetWriter;

tilesetWriter.writeTileset(
    tiles,
    tileElevations,
    "../output/tileset.json"
);

    return 0;
}