#include "Search.h"
#include <queue>
#include <unordered_map>
#include <algorithm>

namespace algo {

// BFS مشترک: فاصله و والدِ هر راس نسبت به مبدا
static void bfsFrom(const Graph& g, const std::string& source,
                     std::unordered_map<std::string, int>& dist,
                     std::unordered_map<std::string, std::string>& parent) {
    std::queue<std::string> q;
    dist[source] = 0;
    q.push(source);
    while (!q.empty()) {
        std::string u = q.front(); q.pop();
        for (const std::string& v : g.getFriends(u)) {
            if (!dist.count(v)) {
                dist[v] = dist[u] + 1;
                parent[v] = u;
                q.push(v);
            }
        }
    }
}

bool isConnected(const Graph& g, const std::string& from, const std::string& to) {
    if (!g.findUser(from) || !g.findUser(to)) return false;
    if (from == to) return true;
    std::unordered_map<std::string, int> dist;
    std::unordered_map<std::string, std::string> parent;
    bfsFrom(g, from, dist, parent);
    return dist.count(to) > 0;
}

PathResult shortestPath(const Graph& g, const std::string& from, const std::string& to) {
    PathResult result;
    if (!g.findUser(from) || !g.findUser(to)) return result;

    if (from == to) {
        result.connected = true;
        result.distance = 0;
        result.path = {from};
        return result;
    }

    std::unordered_map<std::string, int> dist;
    std::unordered_map<std::string, std::string> parent;
    bfsFrom(g, from, dist, parent);

    if (!dist.count(to)) return result; // متصل نیستند

    result.connected = true;
    result.distance = dist[to];

    std::vector<std::string> path;
    std::string cur = to;
    path.push_back(cur);
    while (cur != from) {
        cur = parent[cur];
        path.push_back(cur);
    }
