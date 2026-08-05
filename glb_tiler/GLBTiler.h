#pragma once

#include <string>
#include <map>
#include <vector>

#include "tiny_gltf.h"


struct TileKey
{
    int x;
    int z;

    bool operator<(const TileKey& other) const
    {
        if(x != other.x)
            return x < other.x;

        return z < other.z;
    }
};


class GLBTiler
{

public:

    bool load(
        const std::string& filename
    );


    void printStats();


private:

    tinygltf::Model model;


};