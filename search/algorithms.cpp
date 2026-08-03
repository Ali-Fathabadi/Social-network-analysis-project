#include "Algorithms.h"
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <cmath>

namespace algo {


std::vector<std::vector<std::string>> findConnectedComponents(const Graph& g) {
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
    for (auto& comp : components) std::sort(comp.begin(), comp.end());
    std::sort(components.begin(), components.end(), [](const auto& a, const auto& b) {
        return a.front() < b.front();
    });
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
    for (const auto& comp : findConnectedComponents(g)) {
        largest = std::max(largest, comp.size());
    }
    stats.largestComponentSize = largest;

    return stats;
}

std::vector<DegreeEntry> findMostConnectedUsers(const Graph& g) {
    std::vector<DegreeEntry> result;
    for (const auto& [id, friends] : g.getAdjacency()) {
        result.push_back({id, (int)friends.size()});
    }
    std::sort(result.begin(), result.end(), [](const DegreeEntry& a, const DegreeEntry& b) {
        if (a.friendCount != b.friendCount) return a.friendCount > b.friendCount;
        return a.id < b.id; 
    });
    return result;
}

std::vector<std::string> mutualFriends(const Graph& g, const std::string& a, const std::string& b) {
    std::vector<std::string> result;
    const auto& friendsA = g.getFriends(a);
    const auto& friendsB = g.getFriends(b);
    const auto& smaller = friendsA.size() <= friendsB.size() ? friendsA : friendsB;
    const auto& larger  = friendsA.size() <= friendsB.size() ? friendsB : friendsA;
    for (const std::string& f : smaller) {
        if (larger.count(f)) result.push_back(f);
    }
    std::sort(result.begin(), result.end());
    return result;
}

std::vector<std::string> findKeyUsers(const Graph& g) {
    std::unordered_map<std::string, double> centrality;
    for (const auto& [id, user] : g.getAllUsers()) centrality[id] = 0.0;

    for (const auto& [s, userS] : g.getAllUsers()) {
        std::unordered_map<std::string, std::vector<std::string>> predecessors;
        std::unordered_map<std::string, long long> sigma;   
        std::unordered_map<std::string, int> dist;
        for (const auto& [id, user] : g.getAllUsers()) { sigma[id] = 0; dist[id] = -1; }
        sigma[s] = 1;
        dist[s] = 0;

        std::stack<std::string> order; 
        std::queue<std::string> q;
        q.push(s);
        while (!q.empty()) {
            std::string v = q.front(); q.pop();
            order.push(v);
            for (const std::string& w : g.getFriends(v)) {
                if (dist[w] < 0) {
                    dist[w] = dist[v] + 1;
                    q.push(w);
                }
                if (dist[w] == dist[v] + 1) {
                    sigma[w] += sigma[v];
                    predecessors[w].push_back(v);
                }
            }
        }

        std::unordered_map<std::string, double> delta;
        for (const auto& [id, user] : g.getAllUsers()) delta[id] = 0.0;

        while (!order.empty()) {
            std::string w = order.top(); order.pop();
            for (const std::string& v : predecessors[w]) {
                delta[v] += ((double)sigma[v] / (double)sigma[w]) * (1.0 + delta[w]);
            }
            if (w != s) centrality[w] += delta[w];
        }
    }

    double maxVal = 0.0;
    for (auto& [id, val] : centrality) {
        val /= 2.0;
        maxVal = std::max(maxVal, val);
    }

    std::vector<std::string> keyUsers;
    if (maxVal > 1e-9) {
        for (const auto& [id, val] : centrality) {
            if (std::abs(val - maxVal) < 1e-6) keyUsers.push_back(id);
        }
        std::sort(keyUsers.begin(), keyUsers.end());
    }
    return keyUsers;
}

} 
