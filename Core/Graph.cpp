#include "Graph.h"
#include "json.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <system_error>
#include <utility>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

namespace {

std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

bool fail(std::string* error, const std::string& message) {
    if (error) *error = message;
    return false;
}

bool requireStringField(const json::Value& object, const std::string& field,
                        std::string& output, std::string* error) {
    if (object.type != json::Type::Object || !object.objVal->has(field)) {
        return fail(error, "Missing string field: " + field);
    }
    const json::Value& value = object.objVal->at(field);
    if (value.type != json::Type::String) {
        return fail(error, "Field must be a string: " + field);
    }
    output = value.strVal;
    return true;
}

}  // namespace

bool Graph::addUser(const std::string& id, const std::string& name) {
    const std::string cleanId = trim(id);
    const std::string cleanName = trim(name);
    if (cleanId.empty() || cleanName.empty() || users.count(cleanId)) return false;
    users.emplace(cleanId, User{cleanId, cleanName});
    adjacency.emplace(cleanId, std::unordered_set<std::string>{});
    return true;
}

bool Graph::removeUser(const std::string& id) {
    auto userIt = users.find(id);
    if (userIt == users.end()) return false;

    auto adjacencyIt = adjacency.find(id);
    if (adjacencyIt != adjacency.end()) {
        const auto friendsCopy = adjacencyIt->second;
        for (const std::string& friendId : friendsCopy) {
            auto friendIt = adjacency.find(friendId);
            if (friendIt != adjacency.end()) friendIt->second.erase(id);
        }
    }
    adjacency.erase(id);
    users.erase(userIt);
    return true;
}

bool Graph::editUser(const std::string& id, const std::string& newName) {
    auto it = users.find(id);
    const std::string cleanName = trim(newName);
    if (it == users.end() || cleanName.empty()) return false;
    it->second.name = cleanName;
    return true;
}

bool Graph::addFriendship(const std::string& id1, const std::string& id2) {
    if (id1 == id2 || !users.count(id1) || !users.count(id2)) return false;
    if (areFriends(id1, id2)) return false;
    adjacency[id1].insert(id2);
    adjacency[id2].insert(id1);
    return true;
}

bool Graph::removeFriendship(const std::string& id1, const std::string& id2) {
    if (!users.count(id1) || !users.count(id2) || !areFriends(id1, id2)) return false;
    adjacency[id1].erase(id2);
    adjacency[id2].erase(id1);
    return true;
}

bool Graph::findUser(const std::string& id) const {
    return users.count(id) != 0;
}

const User* Graph::getUser(const std::string& id) const {
    const auto it = users.find(id);
    return it == users.end() ? nullptr : &it->second;
}

bool Graph::areFriends(const std::string& id1, const std::string& id2) const {
    const auto it = adjacency.find(id1);
    return it != adjacency.end() && it->second.count(id2) != 0;
}

const std::unordered_map<std::string, User>& Graph::getAllUsers() const {
    return users;
}

const std::unordered_set<std::string>& Graph::getFriends(const std::string& id) const {
    static const std::unordered_set<std::string> empty;
    const auto it = adjacency.find(id);
    return it == adjacency.end() ? empty : it->second;
}

const std::unordered_map<std::string, std::unordered_set<std::string>>& Graph::getAdjacency() const {
    return adjacency;
}

size_t Graph::friendshipCount() const {
    size_t degreeSum = 0;
    for (const auto& [id, friends] : adjacency) degreeSum += friends.size();
    return degreeSum / 2;
}

bool Graph::saveToFile(const std::string& path, std::string* error) const {
    namespace fs = std::filesystem;
    const fs::path target(path);
    std::error_code ec;
    if (!target.parent_path().empty()) {
        fs::create_directories(target.parent_path(), ec);
        if (ec) return fail(error, "Could not create database directory: " + ec.message());
    }

    json::Value root = json::Value::makeObject();

    std::vector<std::string> ids;
    ids.reserve(users.size());
    for (const auto& [id, user] : users) ids.push_back(id);
    std::sort(ids.begin(), ids.end());

    json::Value usersArray = json::Value::makeArray();
    for (const std::string& id : ids) {
        const User& user = users.at(id);
        json::Value item = json::Value::makeObject();
        item["id"] = json::Value(user.id);
        item["name"] = json::Value(user.name);
        usersArray.push_back(item);
    }
    root["users"] = usersArray;

    std::vector<std::pair<std::string, std::string>> edges;
    for (const auto& [id, friends] : adjacency) {
        for (const std::string& friendId : friends) {
            if (id < friendId) edges.push_back({id, friendId});
        }
    }
    std::sort(edges.begin(), edges.end());

    json::Value edgeArray = json::Value::makeArray();
    for (const auto& [a, b] : edges) {
        json::Value pair = json::Value::makeArray();
        pair.push_back(json::Value(a));
        pair.push_back(json::Value(b));
        edgeArray.push_back(pair);
    }
    root["edges"] = edgeArray;

    const fs::path temporary = target.string() + ".tmp";
    {
        std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
        if (!file.is_open()) return fail(error, "Could not open temporary database file");
        file << root.dump() << '\n';
        file.flush();
        if (!file.good()) {
            file.close();
            fs::remove(temporary, ec);
            return fail(error, "Could not write the complete database file");
        }
    }

#ifdef _WIN32
    // MoveFileExW replaces the target in one filesystem operation and supports Unicode paths.
    if (!MoveFileExW(temporary.c_str(), target.c_str(),
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        const unsigned long code = GetLastError();
        fs::remove(temporary, ec);
        return fail(error, "Could not atomically replace database file (Windows error " +
                           std::to_string(code) + ")");
    }
#else
    // POSIX rename replaces an existing file atomically when both paths are on the same filesystem.
    fs::rename(temporary, target, ec);
    if (ec) {
        const std::string message = ec.message();
        fs::remove(temporary, ec);
        return fail(error, "Could not atomically replace database file: " + message);
    }
#endif
    return true;
}

bool Graph::loadFromFile(const std::string& path, std::string* error) {
    namespace fs = std::filesystem;
    std::error_code ec;
    if (!fs::exists(path, ec)) {
        if (ec) return fail(error, "Could not inspect database file: " + ec.message());
        users.clear();
        adjacency.clear();
        return true;
    }

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return fail(error, "Could not open database file");
    std::ostringstream buffer;
    buffer << file.rdbuf();
    if (!file.good() && !file.eof()) return fail(error, "Could not read database file");
    const std::string content = buffer.str();
    if (trim(content).empty()) return fail(error, "Database file is empty");

    json::Value root;
    try {
        root = json::Value::parse(content);
    } catch (const std::exception& exception) {
        return fail(error, std::string("Invalid JSON: ") + exception.what());
    }

    if (root.type != json::Type::Object) return fail(error, "Database root must be an object");
    if (!root.objVal->has("users") || !root.objVal->has("edges")) {
        return fail(error, "Database must contain users and edges arrays");
    }
    const json::Value& userValues = root.objVal->at("users");
    const json::Value& edgeValues = root.objVal->at("edges");
    if (userValues.type != json::Type::Array || edgeValues.type != json::Type::Array) {
        return fail(error, "users and edges must both be arrays");
    }

    Graph candidate;
    for (const json::Value& item : *userValues.arrVal) {
        std::string id;
        std::string name;
        if (!requireStringField(item, "id", id, error) ||
            !requireStringField(item, "name", name, error)) {
            return false;
        }
        if (!candidate.addUser(id, name)) {
            return fail(error, "User IDs and names must be non-empty and IDs must be unique");
        }
    }

    std::set<std::pair<std::string, std::string>> seenEdges;
    for (const json::Value& edge : *edgeValues.arrVal) {
        if (edge.type != json::Type::Array || edge.arrVal->size() != 2 ||
            (*edge.arrVal)[0].type != json::Type::String ||
            (*edge.arrVal)[1].type != json::Type::String) {
            return fail(error, "Every edge must be an array containing exactly two user IDs");
        }
        std::string a = (*edge.arrVal)[0].strVal;
        std::string b = (*edge.arrVal)[1].strVal;
        if (a > b) std::swap(a, b);
        if (a == b) return fail(error, "Self friendships are not allowed");
        if (!candidate.findUser(a) || !candidate.findUser(b)) {
            return fail(error, "Every edge must reference existing users");
        }
        if (!seenEdges.insert({a, b}).second) return fail(error, "Duplicate friendship in database");
        if (!candidate.addFriendship(a, b)) return fail(error, "Invalid friendship in database");
    }

    users.swap(candidate.users);
    adjacency.swap(candidate.adjacency);
    return true;
}
