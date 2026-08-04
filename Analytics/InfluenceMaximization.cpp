#include "influenceMaximization.h"
#include "algorithms.h"   // findConnectedComponents
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
static std::string findComponentCenter(const Graph& g, const std::vector<std::string>& component) {
    std::string best;
    int bestEcc = std::numeric_limits<int>::max();
    for (const std::string& candidate : component) {
        auto dist = multiSourceBFS(g, {candidate});
        int ecc = 0;
        for (const std::string& other : component)
            ecc = std::max(ecc, dist.count(other) ? dist[other] : 0);
        if (ecc < bestEcc) { bestEcc = ecc; best = candidate; }
    }
    return best;
}

std::vector<std::string> optimizeNewsSpread(const Graph& g, int k) {
    std::vector<std::string> seeds;
    if (k <= 0) return seeds;

    // چون خبر هرگز از یک مؤلفه‌ی همبند به مؤلفه‌ی دیگر منتقل نمی‌شود،
    // پوشش نهایی فقط به تعداد و اندازه‌ی مؤلفه‌های انتخاب‌شده بستگی دارد
    std::vector<std::vector<std::string>> components = findConnectedComponents(g);
    std::sort(components.begin(), components.end(),
              [](const auto& a, const auto& b) { return a.size() > b.size(); });

    // برای بیشینه‌کردن مجموع افراد پوشش‌داده‌شده، انتخاب top-K بزرگ‌ترین مؤلفه‌ها
    // بهینه است (هر seed حداکثر یک مؤلفه را می‌پوشاند - اثبات با تبادل/exchange argument)
    int numSeedComponents = std::min((int)components.size(), k);
    for (int i = 0; i < numSeedComponents; ++i)
        seeds.push_back(findComponentCenter(g, components[i]));

    int remaining = k - numSeedComponents;
    if (remaining <= 0) { std::sort(seeds.begin(), seeds.end()); return seeds; }

    // اگر K بیشتر از تعداد مؤلفه‌ها باشد، seedهای اضافی را با الگوریتم حریصانه‌ی
    // "دورترین نقطه" (2-تقریب استاندارد برای مسئله‌ی k-center) اضافه می‌کنیم
    // تا زمان رسیدن خبر به دورترین کاربران کاهش یابد
    std::unordered_set<std::string> seedSet(seeds.begin(), seeds.end());
    for (int i = 0; i < remaining; ++i) {
        auto dist = multiSourceBFS(g, seeds);
        std::string farthest;
        int farthestDist = -1;
        for (const auto& [id, user] : g.getAllUsers()) {
            if (seedSet.count(id)) continue; // یک کاربر نباید دوبار seed شود
            int d = dist.count(id) ? dist[id] : 0;
            if (d > farthestDist) { farthestDist = d; farthest = id; }
        }
        if (farthest.empty()) break; // دیگر کاربر جدیدی برای انتخاب باقی نمانده
        seeds.push_back(farthest);
        seedSet.insert(farthest);
    }

    std::sort(seeds.begin(), seeds.end());
    return seeds;
}

}
