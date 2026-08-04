#pragma once

#include <string>
#include "MeshBuilder.h"


class TilesetWriter
{

public:

    bool writeTileset(

        const MeshBuilder& mesh,

        double originLat,

        double originLon,

        const std::string& filename

    );

};