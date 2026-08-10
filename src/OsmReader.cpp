#include "OsmReader.h"
#include <fstream>
#include <regex>
#include <iostream>

bool OsmReader::load(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
        return false;

    std::string line;

    std::regex nodeRegex(
        R"OSM(<node id="([0-9]+)".*lat="([0-9.-]+)" lon="([0-9.-]+)")OSM");

    std::regex wayStartRegex(
        R"OSM(<way id="([0-9]+)")OSM");

    std::regex relationStartRegex(
        R"OSM(<relation id="([0-9]+)")OSM");

    std::regex ndRegex(
        R"OSM(<nd ref="([0-9]+)")OSM");

    std::regex memberRegex(
        R"OSM(<member type="way" ref="([0-9]+)".*role="(outer|inner)")OSM");

    std::regex buildingRegex(
        R"OSM(k="building")OSM");

    std::regex heightRegex(
        R"OSM(k="height" v="([0-9.]+)")OSM");

    std::regex levelsRegex(
        R"OSM(k="building:levels" v="([0-9]+)")OSM");

    bool inWay = false;
    bool inRelation = false;

    bool currentIsBuilding = false;

    long long currentWayId = -1;

    Building current;

    std::vector<long long> currentWayNodeList;

    std::vector<long long> relationOuterWays;

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

        if (std::regex_search(line, match, wayStartRegex))
        {
            inWay = true;
            currentWayId = std::stoll(match[1]);
            current.id = currentWayId;
            current.nodeIds.clear();
            current.height = -1.0;
            current.levels = -1;
            currentIsBuilding = false;

            currentWayNodeList.clear();
        }

        if (inWay && std::regex_search(line, match, ndRegex))
        {
            long long id = std::stoll(match[1]);
            current.nodeIds.push_back(id);
            currentWayNodeList.push_back(id);
        }

        if (inWay && std::regex_search(line, match, buildingRegex))
            currentIsBuilding = true;

        if (inWay && std::regex_search(line, match, heightRegex))
            current.height = std::stod(match[1]);

        if (inWay && std::regex_search(line, match, levelsRegex))
            current.levels = std::stoi(match[1]);

        if (inWay && line.find("</way>") != std::string::npos)
        {
            wayNodes[currentWayId] = currentWayNodeList;

            if (currentIsBuilding)
                buildings.push_back(current);

            inWay = false;
        }

        if (std::regex_search(line, match, relationStartRegex))
        {
            inRelation = true;
            relationOuterWays.clear();

            current.nodeIds.clear();
            current.height = -1.0;
            current.levels = -1;
            currentIsBuilding = false;
        }

        if (inRelation && std::regex_search(line, match, memberRegex))
        {
            std::string role = match[2];

            if (role == "outer")
                relationOuterWays.push_back(std::stoll(match[1]));
        }

        if (inRelation && std::regex_search(line, match, buildingRegex))
            currentIsBuilding = true;

        if (inRelation && std::regex_search(line, match, heightRegex))
            current.height = std::stod(match[1]);

        if (inRelation && std::regex_search(line, match, levelsRegex))
            current.levels = std::stoi(match[1]);

        if (inRelation && line.find("</relation>") != std::string::npos)
        {
            if (currentIsBuilding)
            {
                for (long long wayId : relationOuterWays)
                {
                    auto it = wayNodes.find(wayId);

                    if (it == wayNodes.end())
                        continue;

                    const auto& way = it->second;

                    if (current.nodeIds.empty())
                    {
                        current.nodeIds = way;
                    }
                    else
                    {
                        if (!current.nodeIds.empty() &&
                            !way.empty() &&
                            current.nodeIds.back() == way.front())
                        {
                            current.nodeIds.insert(
                                current.nodeIds.end(),
                                way.begin() + 1,
                                way.end());
                        }
                        else
                        {
                            current.nodeIds.insert(
                                current.nodeIds.end(),
                                way.begin(),
                                way.end());
                        }
                    }
                }

                if (current.nodeIds.size() >= 4)
                    buildings.push_back(current);
            }

            inRelation = false;
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