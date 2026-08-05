#include <iostream>

#include "GLBTiler.h"

int main()
{
    GLBTiler tiler;

    if (!tiler.load("../../data/DD3.glb"))
        return 1;

    std::cout << "DD3.glb loaded" << std::endl;

    tiler.printStats();

    tiler.findConnectedBuildings();

    tiler.writeTileGLB(
        0,
        0,
        100.0,
        "../../output/tile_0_0.glb"
    );

    return 0;
}