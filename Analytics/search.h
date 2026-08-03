#pragma once
#include "Graph.h"
#include <string>
#include <vector>

namespace algo {

struct PathResult {
    bool connected = false;
    std::vector<std::string> path;   // شامل خود مبدا و مقصد
    int distance = -1;               // تعداد یال‌ها؛ اگر متصل نباشند -1
};

struct DistanceEntry {
    std::string id;
    int distance;     // -1 یعنی بی‌نهایت (غیرقابل‌دسترس)
};

// آیا دو کاربر (از طریق هر مسیری، نه صرفاً دوستی مستقیم) به هم متصل‌اند؟
bool isConnected(const Graph& g, const std::string& from, const std::string& to);

// کوتاه‌ترین مسیر بین دو کاربر (BFS)
PathResult shortestPath(const Graph& g, const std::string& from, const std::string& to);

// فاصله‌ی یک کاربر از تمام کاربران شبکه (BFS تک‌منبعی)
std::vector<DistanceEntry> distancesFromUser(const Graph& g, const std::string& from);

}
