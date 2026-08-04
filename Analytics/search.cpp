#include "search.h"
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
 std::reverse(path.begin(), path.end());
    result.path = std::move(path);
    return result;
}

std::vector<DistanceEntry> distancesFromUser(const Graph& g, const std::string& from) {
    std::vector<DistanceEntry> result;
    if (!g.findUser(from)) return result;

    std::unordered_map<std::string, int> dist;
    std::unordered_map<std::string, std::string> parent;
    bfsFrom(g, from, dist, parent);

    for (const auto& [id, user] : g.getAllUsers()) {
        if (id == from) continue; // طبق مثال سند پروژه، خود کاربر در خروجی نمی‌آید
        int d = dist.count(id) ? dist[id] : -1; // -1 = بی‌نهایت
        result.push_back({id, d});
    }

    // مرتب‌سازی صعودی بر اساس فاصله، بی‌نهایت‌ها آخر (طبق مثال سند پروژه)
    std::sort(result.begin(), result.end(), [](const DistanceEntry& a, const DistanceEntry& b) {
        if (a.distance == -1 && b.distance == -1) return a.id < b.id;
        if (a.distance == -1) return false;
        if (b.distance == -1) return true;
        if (a.distance != b.distance) return a.distance < b.distance;
        return a.id < b.id;
    });

    return result;
}

}
