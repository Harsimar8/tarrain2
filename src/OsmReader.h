#pragma once

#include <string>
#include <vector>
#include <unordered_map>

struct Node
{
    long long id;
    double lat;
    double lon;
};

struct Building
{
    std::vector<long long> nodeIds;

    double height = -1.0;   // height in meters
    int levels = -1;        // number of floors
};

class OsmReader
{
public:
    bool load(const std::string& filename);

    const std::unordered_map<long long, Node>& getNodes() const;
    const std::vector<Building>& getBuildings() const;

private:
    std::unordered_map<long long, Node> nodes;
    std::vector<Building> buildings;
};