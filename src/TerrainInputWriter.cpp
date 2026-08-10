#include "TerrainInputWriter.h"

#include <fstream>

void writeTileTerrainInput(
    const Tile& tile,
    const std::vector<Building>& buildings,
    const std::unordered_map<long long, Node>& nodes,
    const std::string& filename)
{
    std::ofstream out(filename);

    if (!out.is_open())
        return;

    out << "{\n";
    out << "  \"tileCenter\": {\n";
    out << "    \"lat\": " << tile.centerLat << ",\n";
    out << "    \"lon\": " << tile.centerLon << "\n";
    out << "  },\n";

    out << "  \"buildings\": [\n";

    bool firstBuilding = true;

    for (int buildingIndex : tile.buildingIndices)
    {
        const Building& b = buildings[buildingIndex];

        if (!firstBuilding)
            out << ",\n";

        firstBuilding = false;

        out << "    {\n";
        out << "      \"id\": " << b.id << ",\n";
        out << "      \"nodes\": [\n";

        bool firstNode = true;

        for (long long nodeId : b.nodeIds)
        {
            auto it = nodes.find(nodeId);

            if (it == nodes.end())
                continue;

            if (!firstNode)
                out << ",\n";

            firstNode = false;

            out << "        { \"lat\": "
                << it->second.lat
                << ", \"lon\": "
                << it->second.lon
                << " }";
        }

        out << "\n      ]\n";
        out << "    }";
    }

    out << "\n  ]\n";
    out << "}\n";

    out.close();
}