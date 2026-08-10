#pragma once

#include <string>
#include <vector>
#include "TileBuilder.h"

class TilesetWriter
{
public:
    bool writeTileset(
        const std::vector<Tile>& tiles,
        const std::vector<double>& tileElevations,
        const std::string& filename
    );
};