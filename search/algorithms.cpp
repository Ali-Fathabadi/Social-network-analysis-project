#include "algorithms.h"
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <cmath>

namespace algo {


std::vector<std::vector<std::string>> findConnectedComponents(const Graph& graph) {
    std::vector<std::string> ids;
    ids.reserve(graph.userCount());
    for (const auto& [id, user] : graph.getAllUsers()) ids.push_back(id);
    std::sort(ids.begin(), ids.end());
    std::vector<std::vector<std::string>> components;
    std::unordered_set<std::string> visited;
    for (const std::string& start : ids) {
        if (visited.count(start)) continue;
        std::queue<std::string> queue;
        std::vector<std::string> component;
       queue.push(start);
        visited.insert(start);
        while (!queue.empty()) {
            const std::string current = queue.front();
            queue.pop();
            component.push_back(current);
            std::vector<std::string> neighbours(graph.getFriends(current).begin(),
                                                graph.getFriends(current).end());
            std::sort(neighbours.begin(), neighbours.end());
            for (const std::string& neighbour : neighbours) {
                if (visited.insert(neighbour).second) queue.push(neighbour);
            }
        }
        std::sort(component.begin(), component.end());
        components.push_back(std::move(component));
    }
    std::sort(components.begin(), components.end(), [](const auto& a, const auto& b) {
        return a.front() < b.front();
    });
    return components;
}

NetworkStats networkStatistics(const Graph& graph) {
    NetworkStats stats;
    stats.totalUsers = graph.userCount();
    stats.totalEdges = graph.friendshipCount();
    stats.avgFriends = stats.totalUsers == 0
        ? 0.0
        : (2.0 * static_cast<double>(stats.totalEdges)) / static_cast<double>(stats.totalUsers);

    const auto components = findConnectedComponents(graph);
    for (const auto& component : components) {
        if (component.size() > stats.largestComponentSize ||
            (component.size() == stats.largestComponentSize &&
             (stats.largestComponent.empty() || component < stats.largestComponent))) {
            stats.largestComponentSize = component.size();
            stats.largestComponent = component;
        }
    }

    const auto mostConnected = findMostConnectedUsers(graph);
    if (!mostConnected.empty()) {
        stats.mostConnectedId = mostConnected.front().id;
        stats.mostConnectedCount = mostConnected.front().friendCount;
    }
    return stats;
}

std::vector<DegreeEntry> degreeRanking(const Graph& graph) {
    std::vector<DegreeEntry> result;
    result.reserve(graph.userCount());
    for (const auto& [id, friends] : graph.getAdjacency()) {
        result.push_back({id, static_cast<int>(friends.size())});
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
