#include "Graph.h"
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
    g.loadFromFile(DB_PATH); // اگر فایل وجود نداشته باشد، گراف خالی باقی می ماند

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
    else {
        printError("Unknown command: " + cmd);
        return 1;
    }

    return 0;
}
