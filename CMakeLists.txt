cmake_minimum_required(VERSION 3.20)

project(terrain)

set(CMAKE_CXX_STANDARD 17)

include_directories(
    src
    external
    external/tinygltf
)

add_executable(osm_to_tiles
    src/main.cpp
    src/OsmReader.cpp
    # src/CoordinateConverter.cpp
    # src/MeshBuilder.cpp
    # src/OBJWriter.cpp
    # src/GLTFWriter.cpp
    src/tinygltf_impl.cpp
    src/TileBuilder.cpp
    src/TerrainInputWriter.cpp
    src/CoordinateConverter.cpp
    src/MeshBuilder.cpp
    src/GLTFWriter.cpp
     src/B3DMWriter.cpp
     src/TilesetWriter.cpp
 )