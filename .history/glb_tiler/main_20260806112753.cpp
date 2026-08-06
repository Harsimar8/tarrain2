#include <iostream>
#include "GLBTiler.h"

int main()
{
    GLBTiler tiler;

    if (!tiler.load("../../data/DD3.glb"))
        return 1;

    std::cout << "DD3.glb loaded" << std::endl;

    tiler.printStats();

    tiler.findNearbyBuildings(6.0);

    

    return 0;
}