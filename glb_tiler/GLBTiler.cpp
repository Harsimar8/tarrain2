#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE

#include "tiny_gltf.h"

#include "GLBTiler.h"

#include <iostream>
#include <limits>

bool GLBTiler::load(
    const std::string& filename
)
{

    tinygltf::TinyGLTF loader;


    std::string err;
    std::string warn;


    bool result =
        loader.LoadBinaryFromFile(
            &model,
            &err,
            &warn,
            filename
        );


    if(!warn.empty())
        std::cout << warn << std::endl;


    if(!err.empty())
        std::cout << err << std::endl;


    if(!result)
    {
        std::cout
        << "Failed loading GLB\n";

        return false;
    }


    return true;
}




void GLBTiler::printStats()
{

    size_t vertexCount = 0;
    size_t triangleCount = 0;


    double minX =
        std::numeric_limits<double>::max();

    double minY =
        std::numeric_limits<double>::max();

    double minZ =
        std::numeric_limits<double>::max();


    double maxX =
        -std::numeric_limits<double>::max();

    double maxY =
        -std::numeric_limits<double>::max();

    double maxZ =
        -std::numeric_limits<double>::max();



    for(auto& mesh : model.meshes)
    {

        for(auto& primitive : mesh.primitives)
        {


            auto it =
            primitive.attributes.find("POSITION");


            if(it == primitive.attributes.end())
                continue;


            const auto& accessor =
                model.accessors[
                    it->second
                ];


            vertexCount += accessor.count;


            triangleCount += accessor.count / 3;



            const auto& view =
                model.bufferViews[
                    accessor.bufferView
                ];


            const auto& buffer =
                model.buffers[
                    view.buffer
                ];


            const float* positions =
            reinterpret_cast<const float*>(
                &buffer.data[
                    view.byteOffset
                    +
                    accessor.byteOffset
                ]
            );



            for(size_t i=0;i<accessor.count;i++)
            {

                double x =
                    positions[i*3+0];

                double y =
                    positions[i*3+1];

                double z =
                    positions[i*3+2];


                minX = std::min(minX,x);
                minY = std::min(minY,y);
                minZ = std::min(minZ,z);


                maxX = std::max(maxX,x);
                maxY = std::max(maxY,y);
                maxZ = std::max(maxZ,z);

            }

        }

    }


    std::cout
    << "\n========== GLB TILER STATS ==========\n";


    std::cout
    << "Meshes : "
    << model.meshes.size()
    << "\n";


    std::cout
    << "Vertices : "
    << vertexCount
    << "\n";


    std::cout
    << "Triangles : "
    << triangleCount
    << "\n";


    std::cout
    << "Min : "
    << minX << ", "
    << minY << ", "
    << minZ
    << "\n";


    std::cout
    << "Max : "
    << maxX << ", "
    << maxY << ", "
    << maxZ
    << "\n";


    std::cout
    << "=====================================\n";

}