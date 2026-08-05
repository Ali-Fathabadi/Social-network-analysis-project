#include "InfluenceMaximization.h"

#include <cstdint>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <limits>

namespace algo {
namespace {

constexpr std::uint64_t MAX_EXACT_COMBINATIONS = 2000000ULL;

struct Evaluation {
    size_t reached = 0;
    int days = 0;
    long long distanceSum = 0;
};

Evaluation evaluate(const Graph& graph, const std::vector<std::string>& seeds) {
    Evaluation result;
    if (seeds.empty()) return result;

    std::unordered_map<std::string, int> distance;
    std::queue<std::string> queue;
    for (const std::string& seed : seeds) {
        if (!graph.findUser(seed) || distance.count(seed)) continue;
        distance[seed] = 0;
        queue.push(seed);
    }
    while (!queue.empty()) {
        const std::string current = queue.front();
        queue.pop();
        for (const std::string& neighbour : graph.getFriends(current)) {
            if (!distance.count(neighbour)) {
                distance[neighbour] = distance[current] + 1;
                queue.push(neighbour);
            }
        }
    }

    result.reached = distance.size();
    for (const auto& [id, value] : distance) {
        result.days = std::max(result.days, value);
        result.distanceSum += value;
    }
    return result;
}

bool better(const Evaluation& candidateEvaluation,
            const std::vector<std::string>& candidateSeeds,
            const Evaluation& currentEvaluation,
            const std::vector<std::string>& currentSeeds) {
    if (candidateEvaluation.reached != currentEvaluation.reached) {
        return candidateEvaluation.reached > currentEvaluation.reached;
    }
    if (candidateEvaluation.days != currentEvaluation.days) {
        return candidateEvaluation.days < currentEvaluation.days;
    }
    if (candidateEvaluation.distanceSum != currentEvaluation.distanceSum) {
        return candidateEvaluation.distanceSum < currentEvaluation.distanceSum;
    }
    return currentSeeds.empty() || candidateSeeds < currentSeeds;
}

std::uint64_t combinationCount(size_t n, size_t k) {
    if (k > n) return 0;
    k = std::min(k, n - k);
    std::uint64_t result = 1;
    for (size_t i = 1; i <= k; ++i) {
        const std::uint64_t numerator = static_cast<std::uint64_t>(n - k + i);
        if (result > MAX_EXACT_COMBINATIONS * static_cast<std::uint64_t>(i) / numerator) {
            return MAX_EXACT_COMBINATIONS + 1;
        }
        result = (result * numerator) / static_cast<std::uint64_t>(i);
        if (result > MAX_EXACT_COMBINATIONS) return result;
    }
    return result;
}

void enumerateCombinations(const Graph& graph,
                           const std::vector<std::string>& ids,
                           size_t targetSize,
                           size_t start,
                           std::vector<std::string>& current,
                           std::vector<std::string>& bestSeeds,
                           Evaluation& bestEvaluation) {
    if (current.size() == targetSize) {
        const Evaluation candidate = evaluate(graph, current);
        if (better(candidate, current, bestEvaluation, bestSeeds)) {
            bestEvaluation = candidate;
            bestSeeds = current;
        }
        return;
    }
    const size_t remaining = targetSize - current.size();
    for (size_t i = start; i + remaining <= ids.size(); ++i) {
        current.push_back(ids[i]);
        enumerateCombinations(graph, ids, targetSize, i + 1, current, bestSeeds, bestEvaluation);
        current.pop_back();
    }
}

InfluenceResult greedySelection(const Graph& graph,
                                const std::vector<std::string>& ids,
                                size_t k) {
    std::vector<std::string> seeds;
    Evaluation seedEvaluation;
    while (seeds.size() < k) {
        std::vector<std::string> bestCandidateSeeds;
        Evaluation bestCandidateEvaluation;
        bool found = false;
        for (const std::string& candidate : ids) {
            if (std::binary_search(seeds.begin(), seeds.end(), candidate)) continue;
            std::vector<std::string> candidateSeeds = seeds;
            candidateSeeds.push_back(candidate);
            std::sort(candidateSeeds.begin(), candidateSeeds.end());
            const Evaluation candidateEvaluation = evaluate(graph, candidateSeeds);
            if (!found || better(candidateEvaluation, candidateSeeds,
                                 bestCandidateEvaluation, bestCandidateSeeds)) {
                found = true;
                bestCandidateSeeds = std::move(candidateSeeds);
                bestCandidateEvaluation = candidateEvaluation;
            }
        }
        if (!found) break;
        seeds = std::move(bestCandidateSeeds);
        seedEvaluation = bestCandidateEvaluation;
    }
    return {seeds, seedEvaluation.reached, seedEvaluation.days, false};
}

}  // namespace

InfluenceResult optimizeNewsSpreadDetailed(const Graph& graph, int k) {
    std::vector<std::string> ids;
    ids.reserve(graph.userCount());
    for (const auto& [id, user] : graph.getAllUsers()) ids.push_back(id);
    std::sort(ids.begin(), ids.end());

    if (k <= 0 || ids.empty()) return {};
    const size_t targetSize = std::min(static_cast<size_t>(k), ids.size());
    if (targetSize == ids.size()) return {ids, ids.size(), 0, true};

    if (combinationCount(ids.size(), targetSize) <= MAX_EXACT_COMBINATIONS) {
        std::vector<std::string> current;
        std::vector<std::string> bestSeeds;
        Evaluation bestEvaluation;
        enumerateCombinations(graph, ids, targetSize, 0, current, bestSeeds, bestEvaluation);
        return {bestSeeds, bestEvaluation.reached, bestEvaluation.days, true};
    }
    return greedySelection(graph, ids, targetSize);
}

std::vector<std::string> optimizeNewsSpread(const Graph& graph, int k) {
    return optimizeNewsSpreadDetailed(graph, k).selectedUsers;
}

}  // namespace algo
