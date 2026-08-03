#include "OsmReader.h"

#include <fstream>
#include <regex>
#include <string>

bool OsmReader::load(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
        return false;

    std::string line;

    std::regex nodeRegex(
    R"OSM(<node id="([0-9]+)".*lat="([0-9.\-]+)" lon="([0-9.\-]+)")OSM"
);

std::regex ndRegex(
    R"OSM(<nd ref="([0-9]+)"/>)OSM"
);

std::regex buildingRegex(
    R"OSM(k="building")OSM"
);

    bool inWay = false;
    bool currentIsBuilding = false;
    Building current;

    while (std::getline(file, line))
    {
        std::smatch match;

        if (std::regex_search(line, match, nodeRegex))
        {
            Node n;
            n.id = std::stoll(match[1]);
            n.lat = std::stod(match[2]);
            n.lon = std::stod(match[3]);
            nodes[n.id] = n;
        }

        if (line.find("<way") != std::string::npos)
        {
            inWay = true;
            current.nodeIds.clear();
            currentIsBuilding = false;
        }

        if (inWay && std::regex_search(line, match, ndRegex))
        {
            current.nodeIds.push_back(std::stoll(match[1]));
        }

        if (inWay && std::regex_search(line, buildingRegex))
        {
            currentIsBuilding = true;
        }

        if (inWay && line.find("</way>") != std::string::npos)
        {
            if (currentIsBuilding)
                buildings.push_back(current);

            inWay = false;
        }
    }

    return true;
}

const std::unordered_map<long long, Node>& OsmReader::getNodes() const
{
    return nodes;
}

const std::vector<Building>& OsmReader::getBuildings() const
{
    return buildings;
}