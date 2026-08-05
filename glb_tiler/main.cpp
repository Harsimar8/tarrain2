#include <iostream>

#include "GLBTiler.h"


int main()
{

    GLBTiler tiler;


    if(!tiler.load(
        "../../data/DD3.glb"
    ))
    {
        return 1;
    }


    std::cout
    << "DD3.glb loaded\n";


    tiler.printStats();


    return 0;
}