#include "Graph.h"
#include "Algorithms.h"
#include "json.h"
#include <iostream>
#include <string>
#include <vector>

static const std::string DB_PATH = "network.json";

static std::string escapeJson(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c == '"' || c == '\\') out += '\\';
        out += c;
    }
    return out;
}

static void printError(const std::string& message) {
    std::cout << "{\"status\": \"error\", \"message\": \"" << escapeJson(message) << "\"}\n";
}

static std::string jsonStringArray(const std::vector<std::string>& items) {
    std::string out = "[";
    for (size_t i = 0; i < items.size(); ++i) {
        out += "\"" + escapeJson(items[i]) + "\"";
        if (i + 1 < items.size()) out += ", ";
    }
    out += "]";
    return out;
}

int main(int argc, char* argv[]) {
    Graph g;
    g.loadFromFile(DB_PATH); // اگر فایل وجود نداشته باشد، گراف خالی باقی می‌ماند

    if (argc < 2) {
        printError("No command provided");
        return 1;
    }
    std::string cmd = argv[1];

    if (cmd == "addUser") {
        if (argc < 4) { printError("Usage: addUser <id> <name>"); return 1; }
        if (g.addUser(argv[2], argv[3])) {
            g.saveToFile(DB_PATH);
            std::cout << "{\"status\": \"success\", \"message\": \"User added\"}\n";
        } else {
            printError("User already exists");
        }
    }
    else if (cmd == "removeUser") {
        if (argc < 3) { printError("Usage: removeUser <id>"); return 1; }
        if (g.removeUser(argv[2])) {
            g.saveToFile(DB_PATH);
            std::cout << "{\"status\": \"success\", \"message\": \"User removed\"}\n";
        } else {
            printError("User not found");
        }
    }
    else if (cmd == "editUser") {
        if (argc < 4) { printError("Usage: editUser <id> <name>"); return 1; }
        if (g.editUser(argv[2], argv[3])) {
            g.saveToFile(DB_PATH);
            std::cout << "{\"status\": \"success\", \"message\": \"User updated\"}\n";
        } else {
            printError("User not found");
        }
    }
    else if (cmd == "findUser") {
        if (argc < 3) { printError("Usage: findUser <id>"); return 1; }
        const User* u = g.getUser(argv[2]);
        if (u) {
            std::cout << "{\"status\": \"success\", \"found\": true, \"id\": \"" << escapeJson(u->id)
                       << "\", \"name\": \"" << escapeJson(u->name) << "\"}\n";
        } else {
            std::cout << "{\"status\": \"success\", \"found\": false}\n";
        }
    }
    else if (cmd == "getUser") {
        if (argc < 3) { printError("Usage: getUser <id>"); return 1; }
        const User* u = g.getUser(argv[2]);
        if (!u) { printError("User not found"); return 1; }
        std::vector<std::string> friends(g.getFriends(argv[2]).begin(), g.getFriends(argv[2]).end());
        std::cout << "{\"status\": \"success\", \"id\": \"" << escapeJson(u->id) << "\", \"name\": \""
                   << escapeJson(u->name) << "\", \"friends\": " << jsonStringArray(friends) << "}\n";
    }
    else if (cmd == "addFriendship") {
        if (argc < 4) { printError("Usage: addFriendship <id1> <id2>"); return 1; }
        if (g.addFriendship(argv[2], argv[3])) {
            g.saveToFile(DB_PATH);
            std::cout << "{\"status\": \"success\", \"message\": \"Friendship added\"}\n";
        } else {
            printError("Could not add friendship");
        }
    }
    else if (cmd == "removeFriendship") {
        if (argc < 4) { printError("Usage: removeFriendship <id1> <id2>"); return 1; }
        if (g.removeFriendship(argv[2], argv[3])) {
            g.saveToFile(DB_PATH);
            std::cout << "{\"status\": \"success\", \"message\": \"Friendship removed\"}\n";
        } else {
            printError("Could not remove friendship");
        }
    }
    else if (cmd == "areFriends") {
        if (argc < 4) { printError("Usage: areFriends <id1> <id2>"); return 1; }
        bool result = g.areFriends(argv[2], argv[3]);
        std::cout << "{\"status\": \"success\", \"are_friends\": " << (result ? "true" : "false") << "}\n";
    }
    else if (cmd == "getFriends") {
        if (argc < 3) { printError("Usage: getFriends <id>"); return 1; }
        if (!g.findUser(argv[2])) { printError("User not found"); return 1; }
        std::vector<std::string> friends(g.getFriends(argv[2]).begin(), g.getFriends(argv[2]).end());
        std::cout << "{\"status\": \"success\", \"friends\": " << jsonStringArray(friends) << "}\n";
    }
    else if (cmd == "networkStatistics") {
        algo::NetworkStats stats = algo::networkStatistics(g);
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["total_users"] = json::Value((double)stats.totalUsers);
        root["total_edges"] = json::Value((double)stats.totalEdges);
        root["avg_friends"] = json::Value(stats.avgFriends);
        root["largest_comp_size"] = json::Value((double)stats.largestComponentSize);
        root["most_connected_id"] = json::Value(stats.mostConnectedId);
        root["most_connected_count"] = json::Value(stats.mostConnectedCount);
        std::cout << root.dump() << "\n";
    }
    else if (cmd == "findMostConnectedUsers") {
        std::vector<algo::DegreeEntry> entries = algo::findMostConnectedUsers(g);
        json::Value arr = json::Value::makeArray();
        for (const auto& e : entries) {
            json::Value uv = json::Value::makeObject();
            uv["id"] = json::Value(e.id);
            uv["friend_count"] = json::Value(e.friendCount);
            arr.push_back(uv);
        }
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["users"] = arr;
        std::cout << root.dump() << "\n";
    }
    else if (cmd == "mutualFriends") {
        if (argc < 4) { printError("Usage: mutualFriends <id1> <id2>"); return 1; }
        if (!g.findUser(argv[2]) || !g.findUser(argv[3])) { printError("User not found"); return 1; }
        std::vector<std::string> mutual = algo::mutualFriends(g, argv[2], argv[3]);
        json::Value arr = json::Value::makeArray();
        for (const auto& id : mutual) arr.push_back(json::Value(id));
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["mutual_friends"] = arr;
        std::cout << root.dump() << "\n";
    }
    else if (cmd == "findKeyUsers") {
        std::vector<std::string> keyUsers = algo::findKeyUsers(g);
        json::Value arr = json::Value::makeArray();
        for (const auto& id : keyUsers) arr.push_back(json::Value(id));
        json::Value root = json::Value::makeObject();
        root["status"] = json::Value("success");
        root["key_users"] = arr;
        std::cout << root.dump() << "\n";
    }
    else {
        printError("Unknown command: " + cmd);
        return 1;
    }

    return 0;
}
