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