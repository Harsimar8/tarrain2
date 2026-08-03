#pragma once

#include <string>
#include "MeshBuilder.h"

class OBJWriter
{
public:
bool writeOBJ(
const MeshBuilder& mesh,
const std::string& filename
);
};
