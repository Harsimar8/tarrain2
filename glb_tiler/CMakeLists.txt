cmake_minimum_required(VERSION 3.16)

project(glb_tiler)

set(CMAKE_CXX_STANDARD 17)


add_executable(glb_tiler
    main.cpp
    GLBTiler.cpp
    tinygltf_impl.cpp
)

target_include_directories(glb_tiler PRIVATE
    external/tinygltf
)