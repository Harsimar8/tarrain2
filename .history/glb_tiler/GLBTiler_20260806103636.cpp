#include "tiny_gltf.h"
#include "GLBTiler.h"

#include <iostream>
#include <limits>
#include <queue>
#include <cmath>
#include <algorithm>
#include <unordered_map>




bool GLBTiler::load(const std::string& filename)
{
    tinygltf::TinyGLTF loader;

    std::string err;
    std::string warn;

    bool ok = loader.LoadBinaryFromFile(
        &model,
        &err,
        &warn,
        filename
    );

    if (!warn.empty())
        std::cout << warn << std::endl;

    if (!err.empty())
        std::cout << err << std::endl;

    return ok;
}

void GLBTiler::printStats()
{
    if (model.meshes.empty())
    {
        std::cout << "No meshes found" << std::endl;
        return;
    }

    const auto& mesh = model.meshes[0];
    const auto& prim = mesh.primitives[0];

    auto posIt = prim.attributes.find("POSITION");

    if (posIt == prim.attributes.end())
    {
        std::cout << "No POSITION attribute" << std::endl;
        return;
    }

    const auto& posAccessor = model.accessors[posIt->second];
    const auto& posView     = model.bufferViews[posAccessor.bufferView];
    const auto& posBuffer   = model.buffers[posView.buffer];

    const float* positions = reinterpret_cast<const float*>(
        &posBuffer.data[posView.byteOffset + posAccessor.byteOffset]);

    double minX = std::numeric_limits<double>::max();
    double minY = std::numeric_limits<double>::max();
    double minZ = std::numeric_limits<double>::max();

    double maxX = -std::numeric_limits<double>::max();
    double maxY = -std::numeric_limits<double>::max();
    double maxZ = -std::numeric_limits<double>::max();

    for (size_t i = 0; i < posAccessor.count; i++)
    {
        double x = positions[i*3+0];
        double y = positions[i*3+1];
        double z = positions[i*3+2];

        minX = std::min(minX, x);
        minY = std::min(minY, y);
        minZ = std::min(minZ, z);

        maxX = std::max(maxX, x);
        maxY = std::max(maxY, y);
        maxZ = std::max(maxZ, z);
    }

    std::cout << "\n========== GLB STATS ==========\n";
    std::cout << "Meshes   : " << model.meshes.size() << std::endl;
    std::cout << "Vertices : " << posAccessor.count << std::endl;
    std::cout << "Min      : "
              << minX << ", "
              << minY << ", "
              << minZ << std::endl;
    std::cout << "Max      : "
              << maxX << ", "
              << maxY << ", "
              << maxZ << std::endl;
    std::cout << "===============================\n";
}

void GLBTiler::findNearbyBuildings(double threshold)
{
    buildings.clear();

    if (model.meshes.empty())
        return;

    const auto& mesh = model.meshes[0];
    if (mesh.primitives.empty())
        return;

    const auto& primitive = mesh.primitives[0];

    auto posIt = primitive.attributes.find("POSITION");
    if (posIt == primitive.attributes.end())
        return;

    const auto& posAccessor = model.accessors[posIt->second];
    const auto& posView     = model.bufferViews[posAccessor.bufferView];
    const auto& posBuffer   = model.buffers[posView.buffer];

    const float* positions = reinterpret_cast<const float*>(
        &posBuffer.data[posView.byteOffset + posAccessor.byteOffset]);

    const auto& idxAccessor = model.accessors[primitive.indices];
    const auto& idxView     = model.bufferViews[idxAccessor.bufferView];
    const auto& idxBuffer   = model.buffers[idxView.buffer];

    std::vector<uint32_t> indices(idxAccessor.count);

    if (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
    {
        const uint16_t* src = reinterpret_cast<const uint16_t*>(
            &idxBuffer.data[idxView.byteOffset + idxAccessor.byteOffset]);

        for (size_t i = 0; i < idxAccessor.count; i++)
            indices[i] = src[i];
    }
    else
    {
        const uint32_t* src = reinterpret_cast<const uint32_t*>(
            &idxBuffer.data[idxView.byteOffset + idxAccessor.byteOffset]);

        for (size_t i = 0; i < idxAccessor.count; i++)
            indices[i] = src[i];
    }

    const size_t triangleCount = indices.size() / 3;

    struct TriCenter
    {
        double x;
        double z;
    };

    std::vector<TriCenter> centers(triangleCount);

    for (size_t t = 0; t < triangleCount; t++)
    {
        uint32_t i0 = indices[t*3+0];
        uint32_t i1 = indices[t*3+1];
        uint32_t i2 = indices[t*3+2];

        centers[t].x =
            (positions[i0*3+0] +
             positions[i1*3+0] +
             positions[i2*3+0]) / 3.0;

        centers[t].z =
            (positions[i0*3+2] +
             positions[i1*3+2] +
             positions[i2*3+2]) / 3.0;
    }

    std::vector<bool> visited(triangleCount, false);

    for (size_t start = 0; start < triangleCount; start++)
    {
        if (visited[start])
            continue;

        BuildingCluster cluster;

        std::queue<size_t> q;
        q.push(start);
        visited[start] = true;

        double minX = centers[start].x;
        double maxX = centers[start].x;
        double minZ = centers[start].z;
        double maxZ = centers[start].z;

        while (!q.empty())
        {
            size_t t = q.front();
            q.pop();

            cluster.triangleIndices.push_back(static_cast<uint32_t>(t));

            minX = std::min(minX, centers[t].x);
            maxX = std::max(maxX, centers[t].x);
            minZ = std::min(minZ, centers[t].z);
            maxZ = std::max(maxZ, centers[t].z);

            for (size_t n = 0; n < triangleCount; n++)
            {
                if (visited[n])
                    continue;

                double dx = centers[t].x - centers[n].x;
                double dz = centers[t].z - centers[n].z;

                if (std::sqrt(dx*dx + dz*dz) <= threshold)
                {
                    visited[n] = true;
                    q.push(n);
                }
            }
        }

        cluster.minX = minX;
        cluster.maxX = maxX;
        cluster.minZ = minZ;
        cluster.maxZ = maxZ;

        cluster.centerX = (minX + maxX) * 0.5;
        cluster.centerZ = (minZ + maxZ) * 0.5;

        buildings.push_back(cluster);
    }

    std::cout << "Buildings detected: "
              << buildings.size()
              << std::endl;
}



bool GLBTiler::writeTileGLB(
    int tileX,
    int tileZ,
    double tileSize,
    const std::string& outputFile)
{
    if (model.meshes.empty())
        return false;

    if (buildings.empty())
        findNearbyBuildings(2.0);

    const auto& srcMesh = model.meshes[0];
    const auto& srcPrim = srcMesh.primitives[0];

    auto posIt = srcPrim.attributes.find("POSITION");
    if (posIt == srcPrim.attributes.end())
        return false;

    const auto& posAccessor = model.accessors[posIt->second];
    const auto& posView     = model.bufferViews[posAccessor.bufferView];
    const auto& posBuffer   = model.buffers[posView.buffer];

    const float* positions = reinterpret_cast<const float*>(
        &posBuffer.data[posView.byteOffset + posAccessor.byteOffset]);

    const auto& idxAccessor = model.accessors[srcPrim.indices];
    const auto& idxView     = model.bufferViews[idxAccessor.bufferView];
    const auto& idxBuffer   = model.buffers[idxView.buffer];

    std::vector<uint32_t> indices(idxAccessor.count);

    if (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
    {
        const uint16_t* src = reinterpret_cast<const uint16_t*>(
            &idxBuffer.data[idxView.byteOffset + idxAccessor.byteOffset]);

        for (size_t i = 0; i < idxAccessor.count; i++)
            indices[i] = src[i];
    }
    else
    {
        const uint32_t* src = reinterpret_cast<const uint32_t*>(
            &idxBuffer.data[idxView.byteOffset + idxAccessor.byteOffset]);

        for (size_t i = 0; i < idxAccessor.count; i++)
            indices[i] = src[i];
    }

    std::vector<float> outPositions;
    std::vector<uint32_t> outIndices;

    std::unordered_map<uint32_t, uint32_t> vertexMap;

    double minTileX = tileX * tileSize;
    double maxTileX = minTileX + tileSize;
    double minTileZ = tileZ * tileSize;
    double maxTileZ = minTileZ + tileSize;

    int exportedBuildings = 0;

    for (const auto& cluster : buildings)
    {
        if (!(cluster.centerX >= minTileX &&
              cluster.centerX <  maxTileX &&
              cluster.centerZ >= minTileZ &&
              cluster.centerZ <  maxTileZ))
        {
            continue;
        }

        exportedBuildings++;

        for (uint32_t tri : cluster.triangleIndices)
        {
            uint32_t base = tri * 3;

            for (int k = 0; k < 3; k++)
            {
                uint32_t oldIndex = indices[base + k];

                auto it = vertexMap.find(oldIndex);

                if (it == vertexMap.end())
                {
                    uint32_t newIndex =
                        static_cast<uint32_t>(outPositions.size() / 3);

                    vertexMap[oldIndex] = newIndex;

                    outPositions.push_back(positions[oldIndex*3+0]);
                    outPositions.push_back(positions[oldIndex*3+1]);
                    outPositions.push_back(positions[oldIndex*3+2]);

                    outIndices.push_back(newIndex);
                }
                else
                {
                    outIndices.push_back(it->second);
                }
            }
        }
    }

    std::cout << "Buildings exported : " << exportedBuildings << std::endl;
    std::cout << "Vertices exported  : " << outPositions.size()/3 << std::endl;
    std::cout << "Triangles exported : " << outIndices.size()/3 << std::endl;

    if (exportedBuildings == 0)
        return false;

    tinygltf::Model outModel;
    outModel.asset.version = "2.0";

    tinygltf::Buffer buffer;

    size_t posBytes = outPositions.size() * sizeof(float);
    size_t idxBytes = outIndices.size() * sizeof(uint32_t);

    buffer.data.resize(posBytes + idxBytes);

    memcpy(buffer.data.data(),
           outPositions.data(),
           posBytes);

    memcpy(buffer.data.data() + posBytes,
           outIndices.data(),
           idxBytes);

    outModel.buffers.push_back(buffer);

    tinygltf::BufferView posViewOut;
    posViewOut.buffer = 0;
    posViewOut.byteOffset = 0;
    posViewOut.byteLength = posBytes;
    posViewOut.target = TINYGLTF_TARGET_ARRAY_BUFFER;
    outModel.bufferViews.push_back(posViewOut);

    tinygltf::BufferView idxViewOut;
    idxViewOut.buffer = 0;
    idxViewOut.byteOffset = posBytes;
    idxViewOut.byteLength = idxBytes;
    idxViewOut.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
    outModel.bufferViews.push_back(idxViewOut);

    tinygltf::Accessor posAccessorOut;
posAccessorOut.bufferView = 0;
posAccessorOut.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
posAccessorOut.count = outPositions.size() / 3;
posAccessorOut.type = TINYGLTF_TYPE_VEC3;

// Compute bounding box
double minX = 1e30, minY = 1e30, minZ = 1e30;
double maxX = -1e30, maxY = -1e30, maxZ = -1e30;

for (size_t i = 0; i < outPositions.size(); i += 3)
{
    minX = std::min(minX, (double)outPositions[i + 0]);
    minY = std::min(minY, (double)outPositions[i + 1]);
    minZ = std::min(minZ, (double)outPositions[i + 2]);

    maxX = std::max(maxX, (double)outPositions[i + 0]);
    maxY = std::max(maxY, (double)outPositions[i + 1]);
    maxZ = std::max(maxZ, (double)outPositions[i + 2]);
}

posAccessorOut.minValues = { minX, minY, minZ };
posAccessorOut.maxValues = { maxX, maxY, maxZ };

outModel.accessors.push_back(posAccessorOut);

    tinygltf::Accessor idxAccessorOut;
    idxAccessorOut.bufferView = 1;
    idxAccessorOut.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    idxAccessorOut.count = outIndices.size();
    idxAccessorOut.type = TINYGLTF_TYPE_SCALAR;
    outModel.accessors.push_back(idxAccessorOut);

    tinygltf::Primitive primOut;
primOut.attributes["POSITION"] = 0;
primOut.indices = 1;
primOut.mode = TINYGLTF_MODE_TRIANGLES;

    tinygltf::Mesh meshOut;
    meshOut.primitives.push_back(primOut);
    outModel.meshes.push_back(meshOut);

    tinygltf::Node node;
    node.mesh = 0;
    outModel.nodes.push_back(node);

    tinygltf::Scene scene;
    scene.nodes.push_back(0);
    outModel.scenes.push_back(scene);
    outModel.defaultScene = 0;

    tinygltf::TinyGLTF writer;

    bool ok = writer.WriteGltfSceneToFile(
        &outModel,
        outputFile,
        true,
        true,
        true,
        true);

    if (!ok)
        return false;

    std::cout << "Tile GLB written: "
              << outputFile
              << std::endl;

    return true;
}


void GLBTiler::exportAllTiles(double tileSize, const std::string& outputFolder)
{
    if (buildings.empty())
        findNearbyBuildings(6.0);

    if (buildings.empty())
        return;

    double globalMinX = 1e30;
    double globalMinZ = 1e30;
    double globalMaxX = -1e30;
    double globalMaxZ = -1e30;

    for (const auto& b : buildings)
    {
        globalMinX = std::min(globalMinX, b.minX);
        globalMinZ = std::min(globalMinZ, b.minZ);
        globalMaxX = std::max(globalMaxX, b.maxX);
        globalMaxZ = std::max(globalMaxZ, b.maxZ);
    }

    int minTileX = static_cast<int>(std::floor(globalMinX / tileSize));
    int maxTileX = static_cast<int>(std::floor(globalMaxX / tileSize));

    int minTileZ = static_cast<int>(std::floor(globalMinZ / tileSize));
    int maxTileZ = static_cast<int>(std::floor(globalMaxZ / tileSize));

    std::cout << "\n========== EXPORTING TILES ==========\n";

    int exported = 0;

    for (int tz = minTileZ; tz <= maxTileZ; tz++)
    {
        for (int tx = minTileX; tx <= maxTileX; tx++)
        {
            std::string file =
                outputFolder +
                "/tile_" +
                std::to_string(tx) +
                "_" +
                std::to_string(tz) +
                ".glb";

            if (writeTileGLB(tx, tz, tileSize, file))
            {
                exported++;
            }
        }
    }

    writeTilesetJson(
        minTileX,
        maxTileX,
        minTileZ,
        maxTileZ,
        tileSize,
        outputFolder
    );

    std::cout << "Total tiles exported: "
              << exported
              << std::endl;

    std::cout << "tileset.json written" << std::endl;
    std::cout << "=====================================\n";
}


#include <fstream>

void GLBTiler::writeTilesetJson(
    int minTileX,
    int maxTileX,
    int minTileZ,
    int maxTileZ,
    double tileSize,
    const std::string& outputFolder)
{
    std::ofstream out(outputFolder + "/tileset.json");

    out << "{\n";
    out << "  \"asset\": { \"version\": \"1.1\" },\n";
    out << "  \"geometricError\": 1000,\n";
    out << "  \"root\": {\n";

    double centerX =
        ((minTileX + maxTileX + 1) * tileSize) * 0.5;

    double centerZ =
        ((minTileZ + maxTileZ + 1) * tileSize) * 0.5;

    double halfX =
        (maxTileX - minTileX + 1) * tileSize * 0.5;

    double halfZ =
        (maxTileZ - minTileZ + 1) * tileSize * 0.5;

    out << "    \"boundingVolume\": {\n";
    out << "      \"box\": ["
        << centerX << ", 6, " << centerZ << ", "
        << halfX << ",0,0, 0,6,0, 0,0,"
        << halfZ
        << "]\n";
    out << "    },\n";

    out << "    \"geometricError\": 100,\n";
    out << "    \"refine\": \"ADD\",\n";
    out << "    \"children\": [\n";

    bool first = true;

    for (int tz = minTileZ; tz <= maxTileZ; tz++)
    {
        for (int tx = minTileX; tx <= maxTileX; tx++)
        {
            if (!first)
                out << ",\n";

            first = false;

            double cx = tx * tileSize + tileSize * 0.5;
            double cz = tz * tileSize + tileSize * 0.5;

            out << "      {\n";
            out << "        \"boundingVolume\": {\n";
            out << "          \"box\": ["
                << cx << ",6," << cz
                << ", 50,0,0, 0,6,0, 0,0,50]\n";
            out << "        },\n";
            out << "        \"geometricError\": 0,\n";
            out << "        \"content\": {\n";
            out << "          \"uri\": \"tile_"
                << tx << "_" << tz
                << ".glb\"\n";
            out << "        }\n";
            out << "      }";
        }
    }

    out << "\n    ]\n";
    out << "  }\n";
    out << "}\n";
}