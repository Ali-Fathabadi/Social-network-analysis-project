#include "Algorithms.h"
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <cmath>

namespace algo {


static std::vector<std::vector<std::string>> connectedComponentsInternal(const Graph& g) {
    std::vector<std::vector<std::string>> components;
    std::unordered_set<std::string> visited;
    for (const auto& [id, user] : g.getAllUsers()) {
        if (visited.count(id)) continue;
        std::vector<std::string> component;
        std::queue<std::string> q;
        q.push(id);
        visited.insert(id);
        while (!q.empty()) {
            std::string cur = q.front(); q.pop();
            component.push_back(cur);
            for (const std::string& nb : g.getFriends(cur)) {
                if (!visited.count(nb)) {
                    visited.insert(nb);
                    q.push(nb);
                }
            }
        }
        components.push_back(std::move(component));
    }
    return components;
}

NetworkStats networkStatistics(const Graph& g) {
    NetworkStats stats;
    stats.totalUsers = g.userCount();

    size_t edgeSum = 0;
    std::string bestId;
    int bestCount = -1;
    for (const auto& [id, friends] : g.getAdjacency()) {
        edgeSum += friends.size();
        int count = (int)friends.size();
        if (count > bestCount || (count == bestCount && (bestId.empty() || id < bestId))) {
            bestCount = count;
            bestId = id;
        }
    }
    stats.totalEdges = edgeSum / 2;
    stats.avgFriends = stats.totalUsers > 0
        ? std::round((2.0 * stats.totalEdges / stats.totalUsers) * 100.0) / 100.0
        : 0.0;
    stats.mostConnectedId = bestId;
    stats.mostConnectedCount = bestCount < 0 ? 0 : bestCount;

    size_t largest = 0;
    for (const auto& comp : connectedComponentsInternal(g)) {
        largest = std::max(largest, comp.size());
    }
    stats.largestComponentSize = largest;

    return stats;
}
