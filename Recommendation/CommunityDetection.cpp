#include "CommunityDetection.h"

#include <set>
#include <stack>
#include <string>
#include <utility>
#include <vector>
#include <cmath>
#include <map>
#include <algorithm>
#include <queue>

namespace algo {

namespace
{

using Adjacency = std::map<std::string, std::set<std::string>>;
using Edge = std::pair<std::string, std::string>;

Edge canonicalEdge(std::string first, std::string second) {
    if (second < first) std::swap(first, second);
    return {first, second};
}

Adjacency makeAdjacency(const Graph& graph) {
    Adjacency adjacency;
    for (const auto& [id, user] : graph.getAllUsers()) adjacency[id];
    for (const auto& [id, friends] : graph.getAdjacency()) {
        for (const std::string& friendId : friends) adjacency[id].insert(friendId);
    }
    return adjacency;
}

size_t edgeCount(const Adjacency& adjacency) {
    size_t degreeSum = 0;
    for (const auto& [id, friends] : adjacency) degreeSum += friends.size();
    return degreeSum / 2;
}

std::vector<std::vector<std::string>> components(const Adjacency& adjacency) {
    std::set<std::string> visited;
    std::vector<std::vector<std::string>> result;
    for (const auto& [start, ignored] : adjacency) {
        if (visited.count(start)) continue;
        std::queue<std::string> queue;
        std::vector<std::string> component;
        queue.push(start);
        visited.insert(start);
        while (!queue.empty()) {
            const std::string current = queue.front();
            queue.pop();
            component.push_back(current);
            for (const std::string& neighbour : adjacency.at(current)) {
                if (visited.insert(neighbour).second) queue.push(neighbour);
            }
        }
        std::sort(component.begin(), component.end());
        result.push_back(std::move(component));
    }
    std::sort(result.begin(), result.end());
    return result;
}

std::map<Edge, double> edgeBetweenness(const Adjacency& adjacency) {
    std::map<Edge, double> scores;
    for (const auto& [source, ignored] : adjacency) {
        std::stack<std::string> order;
        std::queue<std::string> queue;
        std::map<std::string, std::vector<std::string>> predecessors;
        std::map<std::string, double> sigma;
        std::map<std::string, int> distance;
        for (const auto& [id, neighbours] : adjacency) {
            sigma[id] = 0.0;
            distance[id] = -1;
        }
        sigma[source] = 1.0;
        distance[source] = 0;
        queue.push(source);

        while (!queue.empty()) {
            const std::string current = queue.front();
            queue.pop();
            order.push(current);
            for (const std::string& neighbour : adjacency.at(current)) {
                if (distance[neighbour] < 0) {
                    distance[neighbour] = distance[current] + 1;
                    queue.push(neighbour);
                }
                if (distance[neighbour] == distance[current] + 1) {
                    sigma[neighbour] += sigma[current];
                    predecessors[neighbour].push_back(current);
                }
            }
        }

        std::map<std::string, double> dependency;
        for (const auto& [id, neighbours] : adjacency) dependency[id] = 0.0;
        while (!order.empty()) {
            const std::string child = order.top();
            order.pop();
            if (sigma[child] <= 0.0) continue;
            for (const std::string& parent : predecessors[child]) {
                const double contribution = (sigma[parent] / sigma[child]) *
                                            (1.0 + dependency[child]);
                scores[canonicalEdge(parent, child)] += contribution;
                dependency[parent] += contribution;
            }
        }
    }
    for (auto& [edge, score] : scores) score /= 2.0;
    return scores;
}

double modularity(const Adjacency& original,
                  const std::vector<std::vector<std::string>>& partition) {
    const double m = static_cast<double>(edgeCount(original));
    if (m == 0.0) return 0.0;

    double result = 0.0;
    for (const auto& community : partition) {
        std::set<std::string> members(community.begin(), community.end());
        double internalEdgesTwice = 0.0;
        double degreeSum = 0.0;
        for (const std::string& id : community) {
            degreeSum += static_cast<double>(original.at(id).size());
            for (const std::string& neighbour : original.at(id)) {
                if (members.count(neighbour)) internalEdgesTwice += 1.0;
            }
        }
        const double internalEdges = internalEdgesTwice / 2.0;
        result += (internalEdges / m) - std::pow(degreeSum / (2.0 * m), 2.0);
    }
    return result;
}

}  // namespace

std::vector<Community> communityDetection(const Graph& graph) {
    const Adjacency original = makeAdjacency(graph);
    if (original.empty()) return {};

    Adjacency working = original;
    std::vector<std::vector<std::string>> bestPartition = components(working);
    double bestModularity = modularity(original, bestPartition);

    while (edgeCount(working) > 0) {
        const auto scores = edgeBetweenness(working);
        if (scores.empty()) break;

        Edge selected = scores.begin()->first;
        double maximum = scores.begin()->second;
        for (const auto& [edge, score] : scores) {
            if (score > maximum + 1e-12 ||
                (std::abs(score - maximum) <= 1e-12 && edge < selected)) {
                selected = edge;
                maximum = score;
            }
        }
        working[selected.first].erase(selected.second);
        working[selected.second].erase(selected.first);

        auto partition = components(working);
        const double score = modularity(original, partition);
        if (score > bestModularity + 1e-10 ||
            (std::abs(score - bestModularity) <= 1e-10 && partition < bestPartition)) {
            bestModularity = score;
            bestPartition = std::move(partition);
        }
    }

    std::vector<Community> result;
    for (size_t index = 0; index < bestPartition.size(); ++index) {
        result.push_back({static_cast<int>(index + 1), bestPartition[index]});
    }
    return result;
}

}//namespace algo
