#include <iostream>
#include <iomanip>

#include "OsmReader.h"
#include "TileBuilder.h"
#include "TerrainInputWriter.h"

int main()
{
    OsmReader reader;

    if (!reader.load("../data/dehradun_buildings.osm"))
    {
        std::cout << "Cannot open OSM file" << std::endl;
        return 1;
    }

    const auto& nodes = reader.getNodes();
    const auto& buildings = reader.getBuildings();

    TileBuilder tileBuilder;
    auto tiles = tileBuilder.buildTiles(buildings, nodes, 250.0);

    std::cout << "Total buildings: " << buildings.size() << std::endl;
    std::cout << "Total tiles: " << tiles.size() << std::endl;

    size_t totalAssigned = 0;

    // for (size_t i = 0; i < tiles.size(); ++i)
    // {
    //     totalAssigned += tiles[i].buildingIndices.size();

    //     std::string filename =
    // "../output/tiles/tile_" + std::to_string(i) + "_terrain_input.json";

    //     writeTileTerrainInput(
    //         tiles[i],
    //         buildings,
    //         nodes,
    //         filename
    //     );
    // }

    // std::cout
    //     << "Terrain input exported for "
    //     << tiles.size()
    //     << " tiles"
    //     << std::endl;

    std::cout
        << "Total assigned buildings: "
        << totalAssigned
        << std::endl;

    return 0;
}