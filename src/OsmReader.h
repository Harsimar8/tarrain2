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
    double height = -1.0;
    int levels = -1;
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

    // store every way, not only buildings
    std::unordered_map<long long, std::vector<long long>> wayNodes;
};