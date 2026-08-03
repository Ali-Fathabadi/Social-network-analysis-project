// Algorithms.h - Graph algorithms built strictly on Graph's public API
// (getAllUsers, getFriends, getAdjacency, userCount). No new members were
// added to Graph; all algorithm logic lives here, separate from data storage.
#pragma once
#include "Graph.h"
#include <string>
#include <vector>

namespace algo {

struct NetworkStats {
    size_t totalUsers;
    size_t totalEdges;
    double avgFriends;
    size_t largestComponentSize;
    std::string mostConnectedId;
    int mostConnectedCount;
};
// Aggregate network statistics. O(V+E)
NetworkStats networkStatistics(const Graph& g);

struct DegreeEntry {
    std::string id;
    int friendCount;
};
// All users ranked by friend count, descending (ties broken by id ascending
// for deterministic output). O(V log V)
std::vector<DegreeEntry> findMostConnectedUsers(const Graph& g);

// Intersection of two users' friend sets, sorted ascending for determinism.
// O(min(deg_a, deg_b))
std::vector<std::string> mutualFriends(const Graph& g, const std::string& a, const std::string& b);

// ---------- Optional (bonus) ----------

// Betweenness centrality via Brandes' algorithm, O(V*(V+E)) for unweighted
// graphs. Returns the id(s) of the user(s) with the maximum betweenness
// score (the "bridge" users whose removal fragments the network / greatly
// increases distances). Returns an empty list if the max score is 0 (no
// single user sits on any shortest path between others).
std::vector<std::string> findKeyUsers(const Graph& g);

} // namespace algo
