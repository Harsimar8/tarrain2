#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <queue>
#include <set>

#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE


#include "tiny_gltf.h"


struct BuildingComponent
{
    std::vector<uint32_t> triangleIndices;

    double centerX = 0.0;
    double centerZ = 0.0;
};

class GLBTiler
{
public:

    bool load(const std::string& filename);

    void printStats();

    void findConnectedBuildings();

    bool writeTileGLB(
        int tileX,
        int tileZ,
        double tileSize,
        const std::string& outputFile
    );

private:

    tinygltf::Model model;

    std::vector<BuildingComponent> buildings;
};