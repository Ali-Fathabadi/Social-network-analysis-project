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
std::vector<DegreeEntry> findMostConnectedUsers(const Graph& graph) {
    std::vector<DegreeEntry> ranking = degreeRanking(graph);
    if (ranking.empty()) return {};
    const int maximum = ranking.front().friendCount;
    ranking.erase(std::remove_if(ranking.begin(), ranking.end(), [maximum](const DegreeEntry& entry) {
        return entry.friendCount != maximum;
    }), ranking.end());
    return ranking;
}

std::vector<std::string> mutualFriends(const Graph& graph,const std::string& first,const std::string& second)  {
    const auto& firstFriends = graph.getFriends(first);
    const auto& secondFriends = graph.getFriends(second);
    const auto* smaller = &firstFriends;
    const auto* larger = &secondFriends;
    if (firstFriends.size() > secondFriends.size()) std::swap(smaller, larger);

    std::vector<std::string> result;
    for (const std::string& id : *smaller) {
        if (larger->count(id)) result.push_back(id);
    }
    std::sort(result.begin(), result.end());
    return result;
}
std::vector<CentralityEntry> betweennessCentrality(const Graph& graph) {
    std::vector<std::string> ids;
    ids.reserve(graph.userCount());
    for (const auto& [id, user] : graph.getAllUsers()) ids.push_back(id);
    std::sort(ids.begin(), ids.end());

    std::unordered_map<std::string, double> centrality;
    for (const std::string& id : ids) centrality[id] = 0.0;

    for (const std::string& source : ids) {
        std::stack<std::string> order;
        std::queue<std::string> queue;
        std::unordered_map<std::string, std::vector<std::string>> predecessors;
        std::unordered_map<std::string, double> pathCount;
        std::unordered_map<std::string, int> distance;
        for (const std::string& id : ids) {
            pathCount[id] = 0.0;
            distance[id] = -1;
        }
        pathCount[source] = 1.0;
        distance[source] = 0;
        queue.push(source);

        while (!queue.empty()) {
            const std::string current = queue.front();
            queue.pop();
            order.push(current);
            std::vector<std::string> neighbours(graph.getFriends(current).begin(),
                                                graph.getFriends(current).end());
            std::sort(neighbours.begin(), neighbours.end());
            for (const std::string& neighbour : neighbours) {
                if (distance[neighbour] < 0) {
                    distance[neighbour] = distance[current] + 1;
                    queue.push(neighbour);
                }
                if (distance[neighbour] == distance[current] + 1) {
                    pathCount[neighbour] += pathCount[current];
                    predecessors[neighbour].push_back(current);
                }
            }
        }

        std::unordered_map<std::string, double> dependency;
        for (const std::string& id : ids) dependency[id] = 0.0;
        while (!order.empty()) {
            const std::string child = order.top();
            order.pop();
            if (pathCount[child] > 0.0) {
                for (const std::string& parent : predecessors[child]) {
                    dependency[parent] += (pathCount[parent] / pathCount[child]) *
                                          (1.0 + dependency[child]);
                }
            }
            if (child != source) centrality[child] += dependency[child];
        }
    }

    std::vector<CentralityEntry> result;
    result.reserve(ids.size());
    for (const std::string& id : ids) result.push_back({id, centrality[id] / 2.0});
    std::sort(result.begin(), result.end(), [](const CentralityEntry& a, const CentralityEntry& b) {
        if (std::abs(a.score - b.score) > 1e-9) return a.score > b.score;
        return a.id < b.id;
    });
    return result;
}

std::vector<std::string> findKeyUsers(const Graph& graph) {
    const auto ranking = betweennessCentrality(graph);
    if (ranking.empty()) return {};
    const double maximum = ranking.front().score;
    if (maximum <= 1e-9) return {};
    std::vector<std::string> result;
    for (const auto& entry : ranking) {
        if (std::abs(entry.score - maximum) <= 1e-9) result.push_back(entry.id);
    }
    std::sort(result.begin(), result.end());
    return result;
}

}  // namespace algo
