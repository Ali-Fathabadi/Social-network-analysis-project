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
// ---------- 6. findMostConnectedUsers ----------
std::vector<DegreeEntry> findMostConnectedUsers(const Graph& g) {
    std::vector<DegreeEntry> result;
    for (const auto& [id, friends] : g.getAdjacency()) {
        result.push_back({id, (int)friends.size()});
    }
    std::sort(result.begin(), result.end(), [](const DegreeEntry& a, const DegreeEntry& b) {
        if (a.friendCount != b.friendCount) return a.friendCount > b.friendCount;
        return a.id < b.id; // deterministic tie-break
    });
    return result;
}

// ---------- 7. mutualFriends ----------
std::vector<std::string> mutualFriends(const Graph& g, const std::string& a, const std::string& b) {
    std::vector<std::string> result;
    const auto& friendsA = g.getFriends(a);
    const auto& friendsB = g.getFriends(b);
    // iterate the smaller set for efficiency
    const auto& smaller = friendsA.size() <= friendsB.size() ? friendsA : friendsB;
    const auto& larger  = friendsA.size() <= friendsB.size() ? friendsB : friendsA;
    for (const std::string& f : smaller) {
        if (larger.count(f)) result.push_back(f);
    }
    std::sort(result.begin(), result.end());
    return result;
}

// ---------- Optional 1: findKeyUsers (betweenness centrality, Brandes) ----------
std::vector<std::string> findKeyUsers(const Graph& g) {
    std::unordered_map<std::string, double> centrality;
    for (const auto& [id, user] : g.getAllUsers()) centrality[id] = 0.0;

    for (const auto& [s, userS] : g.getAllUsers()) {
        // single-source shortest paths (BFS) bookkeeping, per Brandes 2001
        std::unordered_map<std::string, std::vector<std::string>> predecessors;
        std::unordered_map<std::string, long long> sigma;   // # shortest paths from s
        std::unordered_map<std::string, int> dist;
        for (const auto& [id, user] : g.getAllUsers()) { sigma[id] = 0; dist[id] = -1; }
        sigma[s] = 1;
        dist[s] = 0;


