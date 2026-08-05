

#include "tiny_gltf.h"
#include "GLBTiler.h"

#include <iostream>
#include <limits>
#include <algorithm>
#include <cstring>

bool GLBTiler::load(const std::string& filename)
{
    tinygltf::TinyGLTF loader;

    std::string err;
    std::string warn;

    bool ok =
        loader.LoadBinaryFromFile(
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
        return;

    const auto& primitive = model.meshes[0].primitives[0];

    auto posIt = primitive.attributes.find("POSITION");
    if (posIt == primitive.attributes.end())
        return;

    const auto& accessor = model.accessors[posIt->second];
    const auto& view     = model.bufferViews[accessor.bufferView];
    const auto& buffer   = model.buffers[view.buffer];

    const float* positions =
        reinterpret_cast<const float*>(
            &buffer.data[view.byteOffset + accessor.byteOffset]);

    double minX = 1e30;
    double minY = 1e30;
    double minZ = 1e30;

    double maxX = -1e30;
    double maxY = -1e30;
    double maxZ = -1e30;

    for (size_t i = 0; i < accessor.count; i++)
    {
        double x = positions[i * 3 + 0];
        double y = positions[i * 3 + 1];
        double z = positions[i * 3 + 2];

        minX = std::min(minX, x);
        minY = std::min(minY, y);
        minZ = std::min(minZ, z);

        maxX = std::max(maxX, x);
        maxY = std::max(maxY, y);
        maxZ = std::max(maxZ, z);
    }

    std::cout
        << "\n========== GLB STATS ==========\n"

        << "Meshes : "
        << model.meshes.size()
        << std::endl

        << "Vertices : "
        << accessor.count
        << std::endl

        << "Min : "
        << minX << ", "
        << minY << ", "
        << minZ
        << std::endl

        << "Max : "
        << maxX << ", "
        << maxY << ", "
        << maxZ
        << std::endl

        << "===============================\n";
}


void GLBTiler::findConnectedBuildings()
{
    if (model.meshes.empty())
        return;

    const auto& primitive = model.meshes[0].primitives[0];

    auto posIt = primitive.attributes.find("POSITION");
    if (posIt == primitive.attributes.end())
        return;

    const auto& posAccessor = model.accessors[posIt->second];
    const auto& posView     = model.bufferViews[posAccessor.bufferView];
    const auto& posBuffer   = model.buffers[posView.buffer];

    const float* positions =
        reinterpret_cast<const float*>(
            &posBuffer.data[posView.byteOffset + posAccessor.byteOffset]);

    const auto& idxAccessor = model.accessors[primitive.indices];
    const auto& idxView     = model.bufferViews[idxAccessor.bufferView];
    const auto& idxBuffer   = model.buffers[idxView.buffer];

    std::vector<uint32_t> indices(idxAccessor.count);

    if (idxAccessor.componentType ==
        TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
    {
        const uint16_t* src =
            reinterpret_cast<const uint16_t*>(
                &idxBuffer.data[idxView.byteOffset + idxAccessor.byteOffset]);

        for (size_t i = 0; i < idxAccessor.count; i++)
            indices[i] = src[i];
    }
    else
    {
        const uint32_t* src =
            reinterpret_cast<const uint32_t*>(
                &idxBuffer.data[idxView.byteOffset + idxAccessor.byteOffset]);

        for (size_t i = 0; i < idxAccessor.count; i++)
            indices[i] = src[i];
    }

    size_t triangleCount = indices.size() / 3;

    std::unordered_map<uint32_t, std::vector<uint32_t>>
        vertexToTriangles;

    for (uint32_t t = 0; t < triangleCount; t++)
    {
        vertexToTriangles[indices[t * 3 + 0]].push_back(t);
        vertexToTriangles[indices[t * 3 + 1]].push_back(t);
        vertexToTriangles[indices[t * 3 + 2]].push_back(t);
    }

    std::vector<bool> visited(triangleCount, false);

    buildings.clear();

    for (uint32_t start = 0; start < triangleCount; start++)
    {
        if (visited[start])
            continue;

        BuildingComponent comp;

        std::queue<uint32_t> q;
        q.push(start);
        visited[start] = true;

        std::set<uint32_t> uniqueVerts;

        while (!q.empty())
        {
            uint32_t t = q.front();
            q.pop();

            comp.triangleIndices.push_back(t);

            for (int k = 0; k < 3; k++)
            {
                uint32_t v = indices[t * 3 + k];
                uniqueVerts.insert(v);

                for (uint32_t n : vertexToTriangles[v])
                {
                    if (!visited[n])
                    {
                        visited[n] = true;
                        q.push(n);
                    }
                }
            }
        }

        double sx = 0.0;
        double sz = 0.0;

        for (uint32_t v : uniqueVerts)
        {
            sx += positions[v * 3 + 0];
            sz += positions[v * 3 + 2];
        }

        comp.centerX = sx / uniqueVerts.size();
        comp.centerZ = sz / uniqueVerts.size();

        buildings.push_back(comp);
    }

    std::cout
        << "Buildings detected: "
        << buildings.size()
        << std::endl;
}


bool GLBTiler::writeTileGLB(
    int tileX,
    int tileZ,
    double tileSize,
    const std::string& outputFile)
{
    if (model.meshes.empty() || buildings.empty())
        return false;

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

    std::vector<uint32_t> srcIndices(idxAccessor.count);

    if (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
    {
        const uint16_t* src = reinterpret_cast<const uint16_t*>(
            &idxBuffer.data[idxView.byteOffset + idxAccessor.byteOffset]);

        for (size_t i = 0; i < idxAccessor.count; i++)
            srcIndices[i] = src[i];
    }
    else
    {
        const uint32_t* src = reinterpret_cast<const uint32_t*>(
            &idxBuffer.data[idxView.byteOffset + idxAccessor.byteOffset]);

        for (size_t i = 0; i < idxAccessor.count; i++)
            srcIndices[i] = src[i];
    }

    double minX = tileX * tileSize;
    double maxX = minX + tileSize;

    double minZ = tileZ * tileSize;
    double maxZ = minZ + tileSize;

    std::vector<float> outPositions;
    std::vector<uint32_t> outIndices;

    std::unordered_map<uint32_t, uint32_t> vertexMap;

    int buildingCount = 0;

    for (const auto& b : buildings)
    {
        if (!(b.centerX >= minX &&
              b.centerX < maxX &&
              b.centerZ >= minZ &&
              b.centerZ < maxZ))
            continue;

        buildingCount++;

        for (uint32_t tri : b.triangleIndices)
        {
            uint32_t base = tri * 3;

            for (int k = 0; k < 3; k++)
            {
                uint32_t oldIndex = srcIndices[base + k];

                auto it = vertexMap.find(oldIndex);

                if (it == vertexMap.end())
                {
                    uint32_t newIndex =
                        static_cast<uint32_t>(outPositions.size() / 3);

                    vertexMap[oldIndex] = newIndex;

                    outPositions.push_back(positions[oldIndex * 3 + 0]);
                    outPositions.push_back(positions[oldIndex * 3 + 1]);
                    outPositions.push_back(positions[oldIndex * 3 + 2]);

                    outIndices.push_back(newIndex);
                }
                else
                {
                    outIndices.push_back(it->second);
                }
            }
        }
    }

    std::cout << "Buildings exported : " << buildingCount << std::endl;
    std::cout << "Vertices exported  : " << outPositions.size() / 3 << std::endl;
    std::cout << "Triangles exported : " << outIndices.size() / 3 << std::endl;

    if (buildingCount == 0)
        return false;

    tinygltf::Model outModel;

    outModel.scenes.push_back(tinygltf::Scene());
    outModel.defaultScene = 0;

    outModel.nodes.push_back(tinygltf::Node());
    outModel.scenes[0].nodes.push_back(0);

    tinygltf::Mesh mesh;
    tinygltf::Primitive prim;

    outModel.buffers.resize(2);

    outModel.buffers[0].data.resize(outPositions.size() * sizeof(float));
    memcpy(outModel.buffers[0].data.data(),
           outPositions.data(),
           outModel.buffers[0].data.size());

    outModel.buffers[1].data.resize(outIndices.size() * sizeof(uint32_t));
    memcpy(outModel.buffers[1].data.data(),
           outIndices.data(),
           outModel.buffers[1].data.size());

    outModel.bufferViews.resize(2);

    outModel.bufferViews[0].buffer = 0;
    outModel.bufferViews[0].byteOffset = 0;
    outModel.bufferViews[0].byteLength = outModel.buffers[0].data.size();
    outModel.bufferViews[0].target = TINYGLTF_TARGET_ARRAY_BUFFER;

    outModel.bufferViews[1].buffer = 1;
    outModel.bufferViews[1].byteOffset = 0;
    outModel.bufferViews[1].byteLength = outModel.buffers[1].data.size();
    outModel.bufferViews[1].target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

    outModel.accessors.resize(2);

    outModel.accessors[0].bufferView = 0;
    outModel.accessors[0].componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    outModel.accessors[0].count = outPositions.size() / 3;
    outModel.accessors[0].type = TINYGLTF_TYPE_VEC3;

    outModel.accessors[1].bufferView = 1;
    outModel.accessors[1].componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    outModel.accessors[1].count = outIndices.size();
    outModel.accessors[1].type = TINYGLTF_TYPE_SCALAR;

    prim.attributes["POSITION"] = 0;
    prim.indices = 1;
    prim.mode = TINYGLTF_MODE_TRIANGLES;

    mesh.primitives.push_back(prim);

    outModel.meshes.push_back(mesh);
    outModel.nodes[0].mesh = 0;

    tinygltf::TinyGLTF writer;

    bool ok = writer.WriteGltfSceneToFile(
        &outModel,
        outputFile,
        true,
        true,
        true,
        true);

    if (!ok)
    {
        std::cout << "Failed writing tile GLB" << std::endl;
        return false;
    }

    std::cout << "Tile GLB written: " << outputFile << std::endl;

    return true;
}