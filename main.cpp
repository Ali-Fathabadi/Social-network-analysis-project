#include "CommunityDetection.h"
#include "FriendRecommendation.h"
#include "Graph.h"
#include "InfluenceMaximization.h"
#include "algorithms.h"
#include "json.h"
#include "search.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

std::string databasePath() {
    const char* configured = std::getenv("SOCIAL_NETWORK_DB");
    return configured && *configured ? configured : "network.json";
}

int printJson(const json::Value& value, int exitCode = 0) {
    std::cout << value.dump() << '\n';
    return exitCode;
}

int printError(const std::string& message, int exitCode = 1) {
    json::Value root = json::Value::makeObject();
    root["status"] = json::Value("error");
    root["message"] = json::Value(message);
    return printJson(root, exitCode);
}

json::Value stringArray(const std::vector<std::string>& values) {
    json::Value result = json::Value::makeArray();
    for (const std::string& value : values) result.push_back(json::Value(value));
    return result;
}

std::string joinArguments(int argc, char* argv[], int firstIndex) {
    std::string result;
    for (int index = firstIndex; index < argc; ++index) {
        if (!result.empty()) result += ' ';
        result += argv[index];
    }
    return result;
}

bool saveGraph(const Graph& graph, const std::string& path, std::string& error) {
    return graph.saveToFile(path, &error);
}

int requireUsers(const Graph& graph, const std::vector<std::string>& ids) {
    for (const std::string& id : ids) {
        if (!graph.findUser(id)) return printError("User not found: " + id);
    }
    return -1;
}

std::vector<std::string> sortedFriends(const Graph& graph, const std::string& id) {
    std::vector<std::string> friends(graph.getFriends(id).begin(), graph.getFriends(id).end());
    std::sort(friends.begin(), friends.end());
    return friends;
}

json::Value userObject(const Graph& graph, const std::string& id) {
    const User* user = graph.getUser(id);
    json::Value object = json::Value::makeObject();
    object["id"] = json::Value(id);
    object["name"] = json::Value(user ? user->name : "");
    object["friend_count"] = json::Value(graph.getFriends(id).size());
    object["friends"] = stringArray(sortedFriends(graph, id));
    return object;
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) return printError("No command provided");

    const std::string path = databasePath();
    Graph graph;
    std::string error;
    if (!graph.loadFromFile(path, &error)) {
        return printError("Database load failed: " + error, 2);
    }

    const std::string command = argv[1];

    // ---------------- User management ----------------
    if (command == "addUser") {
        if (argc < 4) return printError("Usage: addUser <id> <name>");
        const std::string name = joinArguments(argc, argv, 3);
        if (!graph.addUser(argv[2], name)) {
            return printError("User ID must be unique and ID/name cannot be empty");
        }
        if (!saveGraph(graph, path, error)) return printError("Database save failed: " + error, 2);
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["message"] = json::Value("User added");
        return printJson(root);
    }

    if (command == "removeUser") {
        if (argc < 3) return printError("Usage: removeUser <id>");
        if (!graph.removeUser(argv[2])) return printError("User not found");
        if (!saveGraph(graph, path, error)) return printError("Database save failed: " + error, 2);
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["message"] = json::Value("User removed");
        return printJson(root);
    }

    if (command == "editUser") {
        if (argc < 4) return printError("Usage: editUser <id> <name>");
        const std::string name = joinArguments(argc, argv, 3);
        if (!graph.editUser(argv[2], name)) return printError("User not found or name is empty");
        if (!saveGraph(graph, path, error)) return printError("Database save failed: " + error, 2);
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["message"] = json::Value("User updated");
        return printJson(root);
    }

    if (command == "findUser") {
        if (argc < 3) return printError("Usage: findUser <id>");
        const User* user = graph.getUser(argv[2]);
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["found"] = json::Value(user != nullptr);
        if (user) {
            root["id"] = json::Value(user->id);
            root["name"] = json::Value(user->name);
        }
        return printJson(root);
    }

    if (command == "getUser") {
        if (argc < 3) return printError("Usage: getUser <id>");
        if (const int result = requireUsers(graph, {argv[2]}); result >= 0) return result;
        const User* user = graph.getUser(argv[2]);
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["id"] = json::Value(user->id);
        root["name"] = json::Value(user->name);
        root["friends"] = stringArray(sortedFriends(graph, argv[2]));
        return printJson(root);
    }

    // Internal GUI helper: returns all users with names, degrees and friends.
    if (command == "listUsers") {
        std::vector<std::string> ids;
        ids.reserve(graph.userCount());
        for (const auto& [id, user] : graph.getAllUsers()) ids.push_back(id);
        std::sort(ids.begin(), ids.end());
        json::Value users = json::Value::makeArray();
        for (const std::string& id : ids) users.push_back(userObject(graph, id));
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["users"] = users;
        return printJson(root);
    }

    // ---------------- Friendship management ----------------
    if (command == "addFriendship" || command == "removeFriendship") {
        if (argc < 4) return printError("Usage: " + command + " <id1> <id2>");
        if (const int result = requireUsers(graph, {argv[2], argv[3]}); result >= 0) return result;
        const bool changed = command == "addFriendship"
            ? graph.addFriendship(argv[2], argv[3])
            : graph.removeFriendship(argv[2], argv[3]);
        if (!changed) {
            return printError(command == "addFriendship"
                ? "Friendship already exists or self-friendship was requested"
                : "Friendship does not exist");
        }
        if (!saveGraph(graph, path, error)) return printError("Database save failed: " + error, 2);
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["message"] = json::Value(command == "addFriendship"
            ? "Friendship added" : "Friendship removed");
        return printJson(root);
    }

    if (command == "areFriends") {
        if (argc < 4) return printError("Usage: areFriends <id1> <id2>");
        if (const int result = requireUsers(graph, {argv[2], argv[3]}); result >= 0) return result;
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["are_friends"] = json::Value(graph.areFriends(argv[2], argv[3]));
        return printJson(root);
    }

    if (command == "getFriends") {
        if (argc < 3) return printError("Usage: getFriends <id>");
        if (const int result = requireUsers(graph, {argv[2]}); result >= 0) return result;
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["friends"] = stringArray(sortedFriends(graph, argv[2]));
        return printJson(root);
    }

    // ---------------- Mandatory algorithms ----------------
    if (command == "isConnected") {
        if (argc < 4) return printError("Usage: isConnected <id1> <id2>");
        if (const int result = requireUsers(graph, {argv[2], argv[3]}); result >= 0) return result;
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["connected"] = json::Value(algo::isConnected(graph, argv[2], argv[3]));
        return printJson(root);
    }

    if (command == "shortestPath") {
        if (argc < 4) return printError("Usage: shortestPath <id1> <id2>");
        if (const int result = requireUsers(graph, {argv[2], argv[3]}); result >= 0) return result;
        const algo::PathResult pathResult = algo::shortestPath(graph, argv[2], argv[3]);
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["path"] = stringArray(pathResult.path);
        root["distance"] = pathResult.connected ? json::Value(pathResult.distance) : json::Value(nullptr);
        return printJson(root);
    }

    if (command == "findConnectedComponents") {
        json::Value components = json::Value::makeArray();
        for (const auto& component : algo::findConnectedComponents(graph)) {
            components.push_back(stringArray(component));
        }
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["components"] = components;
        return printJson(root);
    }

    if (command == "recommendFriends") {
        if (argc < 3) return printError("Usage: recommendFriends <id>");
        if (const int result = requireUsers(graph, {argv[2]}); result >= 0) return result;
        json::Value suggestions = json::Value::makeArray();
        for (const auto& suggestion : algo::recommendFriends(graph, argv[2])) {
            json::Value item = json::Value::makeObject();
            item["id"] = json::Value(suggestion.id);
            item["mutual_count"] = json::Value(suggestion.mutualCount);
            suggestions.push_back(item);
        }
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["suggestions"] = suggestions;
        return printJson(root);
    }

    if (command == "mutualFriends") {
        if (argc < 4) return printError("Usage: mutualFriends <id1> <id2>");
        if (const int result = requireUsers(graph, {argv[2], argv[3]}); result >= 0) return result;
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["mutual_friends"] = stringArray(algo::mutualFriends(graph, argv[2], argv[3]));
        return printJson(root);
    }

    if (command == "networkStatistics") {
        const algo::NetworkStats stats = algo::networkStatistics(graph);
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["total_users"] = json::Value(stats.totalUsers);
        root["total_edges"] = json::Value(stats.totalEdges);
        root["avg_friends"] = json::Value(stats.avgFriends);
        root["largest_comp_size"] = json::Value(stats.largestComponentSize);
        root["largest_component"] = stringArray(stats.largest_component);
        root["most_connected_id"] = json::Value(stats.mostConnectedId);
        root["most_connected_count"] = json::Value(stats.mostConnectedCount);
        return printJson(root);
    }

    if (command == "distanceFromUser") {
        if (argc < 3) return printError("Usage: distanceFromUser <id>");
        if (const int result = requireUsers(graph, {argv[2]}); result >= 0) return result;
        json::Value distances = json::Value::makeArray();
        for (const auto& entry : algo::distancesFromUser(graph, argv[2])) {
            json::Value item = json::Value::makeObject();
            item["id"] = json::Value(entry.id);
            item["distance"] = entry.distance < 0 ? json::Value(nullptr) : json::Value(entry.distance);
            distances.push_back(item);
        }
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["distances"] = distances;
        return printJson(root);
    }

    if (command == "findMostConnectedUsers") {
        json::Value users = json::Value::makeArray();
        for (const auto& entry : algo::findMostConnectedUsers(graph)) {
            json::Value item = json::Value::makeObject();
            item["id"] = json::Value(entry.id);
            item["friend_count"] = json::Value(entry.friendCount);
            users.push_back(item);
        }
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["users"] = users;
        return printJson(root);
    }

    // Internal GUI helper: ranking with display names.
    if (command == "degreeRanking") {
        json::Value users = json::Value::makeArray();
        for (const auto& entry : algo::degreeRanking(graph)) {
            json::Value item = json::Value::makeObject();
            item["id"] = json::Value(entry.id);
            item["name"] = json::Value(graph.getUser(entry.id)->name);
            item["friend_count"] = json::Value(entry.friendCount);
            users.push_back(item);
        }
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["users"] = users;
        return printJson(root);
    }

    // ---------------- Optional algorithms ----------------
    if (command == "findKeyUsers") {
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["key_users"] = stringArray(algo::findKeyUsers(graph));
        return printJson(root);
    }

    // Internal GUI helper: full betweenness ranking.
    if (command == "keyUserRanking") {
        json::Value ranking = json::Value::makeArray();
        for (const auto& entry : algo::betweennessCentrality(graph)) {
            json::Value item = json::Value::makeObject();
            item["id"] = json::Value(entry.id);
            item["name"] = json::Value(graph.getUser(entry.id)->name);
            item["score"] = json::Value(entry.score);
            ranking.push_back(item);
        }
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["ranking"] = ranking;
        root["key_users"] = stringArray(algo::findKeyUsers(graph));
        return printJson(root);
    }

    if (command == "communityDetection") {
        json::Value communities = json::Value::makeArray();
        for (const auto& community : algo::communityDetection(graph)) {
            json::Value item = json::Value::makeObject();
            item["id"] = json::Value(community.id);
            item["members"] = stringArray(community.members);
            communities.push_back(item);
        }
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["communities"] = communities;
        return printJson(root);
    }

    if (command == "optimizeNewsSpread") {
        if (argc < 3) return printError("Usage: optimizeNewsSpread <k>");
        char* end = nullptr;
        const long parsed = std::strtol(argv[2], &end, 10);
        if (!end || *end != '\0' || parsed < 1 || parsed > static_cast<long>(graph.userCount())) {
            return printError("K must be an integer between 1 and the total number of users");
        }
        const auto selected = algo::optimizeNewsSpread(graph, static_cast<int>(parsed));
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["selected_users"] = stringArray(selected);
        return printJson(root);
    }

    // Internal GUI helper: complete graph data.
    if (command == "graphData") {
        std::vector<std::string> ids;
        ids.reserve(graph.userCount());
        for (const auto& [id, user] : graph.getAllUsers()) ids.push_back(id);
        std::sort(ids.begin(), ids.end());
        json::Value nodes = json::Value::makeArray();
        for (const std::string& id : ids) nodes.push_back(userObject(graph, id));

        std::vector<std::pair<std::string, std::string>> edgePairs;
        for (const auto& [id, friends] : graph.getAdjacency()) {
            for (const std::string& friendId : friends) {
                if (id < friendId) edgePairs.push_back({id, friendId});
            }
        }
        std::sort(edgePairs.begin(), edgePairs.end());
        json::Value edges = json::Value::makeArray();
        for (const auto& [first, second] : edgePairs) {
            json::Value edge = json::Value::makeArray();
            edge.push_back(json::Value(first));
            edge.push_back(json::Value(second));
            edges.push_back(edge);
        }
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["nodes"] = nodes;
        root["edges"] = edges;
        return printJson(root);
    }

    return printError("Unknown command: " + command);
}
