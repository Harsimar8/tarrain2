#pragma once

#include <string>
#include "MeshBuilder.h"

class GLTFWriter
{
public:
    bool writeGLB(
        const MeshBuilder& mesh,
        const std::string& filename
    );
};