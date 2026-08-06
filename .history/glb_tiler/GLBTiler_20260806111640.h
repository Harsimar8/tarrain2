#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "tiny_gltf.h"

struct BuildingCluster
{
    std::vector<uint32_t> triangleIndices;

    double centerX = 0.0;
    double centerZ = 0.0;

    double minX = 0.0;
    double minZ = 0.0;
    double maxX = 0.0;
    double maxZ = 0.0;
};

struct TileBounds
{
    double minX, minY, minZ;
    double maxX, maxY, maxZ;
};


class GLBTiler
{
public:

    bool load(const std::string& filename);

    void printStats();

    void findNearbyBuildings(double threshold);
    
    bool writeTileGLB(
        int tileX,
        int tileZ,
        double tileSize,
        const std::string& outputFile
    );

    void exportAllTiles(double tileSize, const std::string& outputFolder);

private:

    tinygltf::Model model;

    std::vector<BuildingCluster> buildings;

     void writeTilesetJson(
        int minTileX,
        int maxTileX,
        int minTileZ,
        int maxTileZ,
        double tileSize,
        const std::string& outputFolder
    );
};


