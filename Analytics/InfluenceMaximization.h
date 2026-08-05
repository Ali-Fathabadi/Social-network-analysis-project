#pragma once

#include "Graph.h"

#include <cstddef>
#include <string>
#include <vector>

namespace algo {
struct InfluenceResult {
    std::vector<std::string> selectedUsers;
    size_t reachedUsers = 0;
    int days = 0;
    bool exact = true;
};

InfluenceResult optimizeNewsSpreadDetailed(const Graph& graph, int k);
std::vector<std::string> optimizeNewsSpread(const Graph& graph, int k);

}// namespace algo
