#pragma once
#include "Graph.h"
#include <string>
#include <vector>

namespace algo {

struct NetworkStats {
    size_t totalUsers;
    size_t totalEdges;
    double avgFriends;
    size_t largestComponentSize;
    std::string mostConnectedId;
    int mostConnectedCount;
};
NetworkStats networkStatistics(const Graph& g);

struct DegreeEntry {
    std::string id;
    int friendCount;
};
std::vector<DegreeEntry> findMostConnectedUsers(const Graph& g);

std::vector<std::string> mutualFriends(const Graph& g, const std::string& a, const std::string& b);


std::vector<std::string> findKeyUsers(const Graph& g);

}
