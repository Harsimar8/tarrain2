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
const auto& wallTriangles = mesh.getWallTriangles();
const auto& roofTriangles = mesh.getRoofTriangles();

if (vertices.empty() || (wallTriangles.empty() && roofTriangles.empty()))
    return false;

   // Compute bounds (keep this for accessor min/max)
float minX = vertices[0].x;
float minY = vertices[0].y;
float minZ = vertices[0].z;

float maxX = vertices[0].x;
float maxY = vertices[0].y;
float maxZ = vertices[0].z;

for (const auto& v : vertices)
{
    minX = std::min(minX, v.x);
    minY = std::min(minY, v.y);
    minZ = std::min(minZ, v.z);

    maxX = std::max(maxX, v.x);
    maxY = std::max(maxY, v.y);
    maxZ = std::max(maxZ, v.z);
}

// Export positions WITHOUT centering.
// Keep local coordinates relative to the tile center.
// Only convert axes for glTF/Cesium.
std::vector<float> positions;
positions.reserve(vertices.size() * 3);

for (const auto& v : vertices)
{
    positions.push_back(v.x);   // East
    positions.push_back(v.z);   // Up
    positions.push_back(-v.y);  // North
}


// Simple UV coordinates
std::vector<float> texcoords;
texcoords.reserve(vertices.size() * 2);

for (const auto& v : vertices)
{
    texcoords.push_back(v.u);
    texcoords.push_back(v.v);
}

    std::vector<unsigned int> wallIndices;
wallIndices.reserve(wallTriangles.size() * 3);

for (const auto& t : wallTriangles)
{
    wallIndices.push_back((unsigned int)t.a);
    wallIndices.push_back((unsigned int)t.b);
    wallIndices.push_back((unsigned int)t.c);
}

std::vector<unsigned int> roofIndices;
roofIndices.reserve(roofTriangles.size() * 3);

for (const auto& t : roofTriangles)
{
    roofIndices.push_back((unsigned int)t.a);
    roofIndices.push_back((unsigned int)t.b);
    roofIndices.push_back((unsigned int)t.c);
}

    size_t positionBytes = positions.size() * sizeof(float);
size_t texcoordBytes = texcoords.size() * sizeof(float);
size_t wallIndexBytes = wallIndices.size() * sizeof(unsigned int);
size_t roofIndexBytes = roofIndices.size() * sizeof(unsigned int);

while (positionBytes % 4 != 0) positionBytes++;
while (texcoordBytes % 4 != 0) texcoordBytes++;

std::vector<unsigned char> bufferData(
    positionBytes + texcoordBytes + wallIndexBytes + roofIndexBytes,
    0
);

// Positions
std::memcpy(
    bufferData.data(),
    positions.data(),
    positions.size() * sizeof(float)
);

// UVs
std::memcpy(
    bufferData.data() + positionBytes,
    texcoords.data(),
    texcoords.size() * sizeof(float)
);

// Indices
std::memcpy(
    bufferData.data() + positionBytes + texcoordBytes,
    wallIndices.data(),
    wallIndexBytes
);


std::memcpy(
    bufferData.data() + positionBytes + texcoordBytes + wallIndexBytes,
    roofIndices.data(),
    roofIndexBytes
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

// UV buffer view
tinygltf::BufferView texcoordView;
texcoordView.buffer = 0;
texcoordView.byteOffset = (int)positionBytes;
texcoordView.byteLength = (int)texcoordBytes;
texcoordView.target = TINYGLTF_TARGET_ARRAY_BUFFER;
model.bufferViews.push_back(texcoordView);

// Index buffer view
tinygltf::BufferView indexView;
indexView.buffer = 0;
indexView.byteOffset = (int)(positionBytes + texcoordBytes);
indexView.byteLength = (int)wallIndexBytes;
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


    tinygltf::Accessor texcoordAccessor;
texcoordAccessor.bufferView = 1;
texcoordAccessor.byteOffset = 0;
texcoordAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
texcoordAccessor.count = (int)vertices.size();
texcoordAccessor.type = TINYGLTF_TYPE_VEC2;
texcoordAccessor.normalized = false;
model.accessors.push_back(texcoordAccessor);


    tinygltf::Accessor indexAccessor;
    indexAccessor.bufferView = 2;
    indexAccessor.byteOffset = 0;
    indexAccessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    indexAccessor.count = (int)wallIndices.size();
    indexAccessor.type = TINYGLTF_TYPE_SCALAR;
    indexAccessor.normalized = false;
    model.accessors.push_back(indexAccessor);

    tinygltf::Primitive primitive;
    primitive.attributes["POSITION"] = 0;
primitive.attributes["TEXCOORD_0"] = 1;
primitive.indices = 2;
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