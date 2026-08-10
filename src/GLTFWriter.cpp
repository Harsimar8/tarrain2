#include "GLTFWriter.h"
#include <iostream>
#include <vector>
#include <cstring>

#include "tiny_gltf.h"

bool GLTFWriter::writeGLB(const MeshBuilder& mesh, const std::string& filename)
{
    tinygltf::Model model;
    tinygltf::TinyGLTF gltf;

    model.asset.version = "2.0";
    model.asset.generator = "OSM to GLB C++ Converter";

    const auto& vertices = mesh.getVertices();
    const auto& triangles = mesh.getTriangles();

    if (vertices.empty() || triangles.empty())
        return false;

    std::vector<float> positions;
    positions.reserve(vertices.size() * 3);

    float minX = vertices[0].x;
    float minY = vertices[0].y;
    float minZ = vertices[0].z;

    float maxX = vertices[0].x;
    float maxY = vertices[0].y;
    float maxZ = vertices[0].z;

    for (const auto& v : vertices)
    {
        positions.push_back(v.x);
        positions.push_back(v.y);
        positions.push_back(v.z);

        minX = std::min(minX, v.x);
        minY = std::min(minY, v.y);
        minZ = std::min(minZ, v.z);

        maxX = std::max(maxX, v.x);
        maxY = std::max(maxY, v.y);
        maxZ = std::max(maxZ, v.z);
    }

    std::vector<unsigned int> indices;
    indices.reserve(triangles.size() * 3);

    for (const auto& t : triangles)
    {
        indices.push_back((unsigned int)t.a);
        indices.push_back((unsigned int)t.b);
        indices.push_back((unsigned int)t.c);
    }

    size_t positionBytes = positions.size() * sizeof(float);
    size_t indexBytes = indices.size() * sizeof(unsigned int);

    while (positionBytes % 4 != 0)
        positionBytes++;

    std::vector<unsigned char> bufferData(positionBytes + indexBytes, 0);

    std::memcpy(
        bufferData.data(),
        positions.data(),
        positions.size() * sizeof(float)
    );

    std::memcpy(
        bufferData.data() + positionBytes,
        indices.data(),
        indexBytes
    );

    tinygltf::Buffer buffer;
    buffer.data = bufferData;
    buffer.uri = "";
    model.buffers.push_back(buffer);

    tinygltf::BufferView positionView;
    positionView.buffer = 0;
    positionView.byteOffset = 0;
    positionView.byteLength = (int)positionBytes;
    positionView.target = TINYGLTF_TARGET_ARRAY_BUFFER;
    model.bufferViews.push_back(positionView);

    tinygltf::BufferView indexView;
    indexView.buffer = 0;
    indexView.byteOffset = (int)positionBytes;
    indexView.byteLength = (int)indexBytes;
    indexView.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
    model.bufferViews.push_back(indexView);

    tinygltf::Accessor positionAccessor;
    positionAccessor.bufferView = 0;
    positionAccessor.byteOffset = 0;
    positionAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    positionAccessor.count = (int)vertices.size();
    positionAccessor.type = TINYGLTF_TYPE_VEC3;
    positionAccessor.normalized = false;
    positionAccessor.minValues = { minX, minY, minZ };
    positionAccessor.maxValues = { maxX, maxY, maxZ };
    model.accessors.push_back(positionAccessor);

    tinygltf::Accessor indexAccessor;
    indexAccessor.bufferView = 1;
    indexAccessor.byteOffset = 0;
    indexAccessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    indexAccessor.count = (int)indices.size();
    indexAccessor.type = TINYGLTF_TYPE_SCALAR;
    indexAccessor.normalized = false;
    model.accessors.push_back(indexAccessor);

    tinygltf::Primitive primitive;
    primitive.attributes["POSITION"] = 0;
    primitive.indices = 1;
    primitive.mode = TINYGLTF_MODE_TRIANGLES;

    tinygltf::Mesh gltfMesh;
    gltfMesh.primitives.push_back(primitive);
    model.meshes.push_back(gltfMesh);

    tinygltf::Node node;
    node.mesh = 0;
    model.nodes.push_back(node);

    tinygltf::Scene scene;
    scene.nodes.push_back(0);
    model.scenes.push_back(scene);

    model.defaultScene = 0;

    std::string err;
    std::string warn;

    bool ok = gltf.WriteGltfSceneToFile(
        &model,
        filename,
        false,
        false,
        true,
        true
    );

    return ok;
}