#pragma once
#include "Graph.h"
#include <string>
#include <vector>
#include <cstddef>

namespace algo {


std::vector<std::vector<std::string>> findConnectedComponents(const Graph& graph);


struct NetworkStats {
    size_t totalUsers=0;
    size_t totalEdges=0;
    double avgFriends=0;
    size_t largestComponentSize=0;
    std::vector<std::string> largestComponent;
    std::string mostConnectedId;
    int mostConnectedCount=0;
};
NetworkStats networkStatistics(const Graph& graph);

struct DegreeEntry {
    std::string id;
    int friendCount=0;
};
std::vector<DegreeEntry> degreeRanking(const Graph& graph);
std::vector<DegreeEntry> findMostConnectedUsers(const Graph& graph);

std::vector<std::string> mutualFriends(const Graph& graph,
                                       const std::string& first,
                                       const std::string& second);

struct CentralityEntry {
    std::string id;
    double score = 0.0;
};
std::vector<CentralityEntry> betweennessCentrality(const Graph& graph);
std::vector<std::string> findKeyUsers(const Graph& graph);

}
