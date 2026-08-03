cmake_minimum_required(VERSION 3.20)

project(osm_to_glb_cpp)

set(CMAKE_CXX_STANDARD 17)

include_directories(src external)

add_executable(
osm_to_glb
src/main.cpp
src/OsmReader.cpp
src/CoordinateConverter.cpp
src/MeshBuilder.cpp
src/OBJWriter.cpp
src/B3DMWriter.cpp
src/TilesetWriter.cpp
)
