#include "InfluenceMaximization.h"
#include "Algorithms.h"   // findConnectedComponents
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <limits>

namespace algo {

// BFS چندمنبعی: فاصله‌ی هر راس تا نزدیک‌ترین seed
static std::unordered_map<std::string, int> multiSourceBFS(
    const Graph& g, const std::vector<std::string>& seeds) {
    std::unordered_map<std::string, int> dist;
    std::queue<std::string> q;
    for (const auto& s : seeds) { dist[s] = 0; q.push(s); }
    while (!q.empty()) {
        std::string u = q.front(); q.pop();
        for (const std::string& v : g.getFriends(u)) {
            if (!dist.count(v)) {
                dist[v] = dist[u] + 1;
                q.push(v);
            }
        }
    }
    return dist;
}

// مرکز یک مؤلفه: راسی با کمترین "بیشینه فاصله تا بقیه‌ی اعضا"
// (یعنی خبر با شروع از این راس، در کمترین زمان به کل مؤلفه می‌رسد)
