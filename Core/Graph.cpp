#include "Graph.h"
#include "json.h"
#include <fstream>
#include <sstream>
#include <vector>

bool Graph::addUser(const std::string& id, const std::string& name) {
    if (users.count(id)) return false; // شناسه تکراری مجاز نیست
    users[id] = User{id, name};
    adjacency[id]; // مجموعه دوستان خالی ایجاد می شود
    return true;
}

bool Graph::removeUser(const std::string& id) {
    if (!users.count(id)) return false;
    for (const std::string& friendId : adjacency[id]) {
        adjacency[friendId].erase(id);
    }
    adjacency.erase(id);
    users.erase(id);
    return true;
}

bool Graph::editUser(const std::string& id, const std::string& newName) {
    auto it = users.find(id);
    if (it == users.end()) return false;
    it->second.name = newName;
    return true;
}

bool Graph::addFriendship(const std::string& id1, const std::string& id2) {
    if (id1 == id2 || !users.count(id1) || !users.count(id2)) return false;
    adjacency[id1].insert(id2);
    adjacency[id2].insert(id1);
    return true;
}

bool Graph::removeFriendship(const std::string& id1, const std::string& id2) {
    if (!users.count(id1) || !users.count(id2)) return false;
    adjacency[id1].erase(id2);
    adjacency[id2].erase(id1);
    return true;
}

bool Graph::findUser(const std::string& id) const { return users.count(id) > 0; }

const User* Graph::getUser(const std::string& id) const {
    auto it = users.find(id);
    return it == users.end() ? nullptr : &it->second;
}

bool Graph::areFriends(const std::string& id1, const std::string& id2) const {
    auto it = adjacency.find(id1);
    if (it == adjacency.end()) return false;
    return it->second.count(id2) > 0;
}

const std::unordered_map<std::string, User>& Graph::getAllUsers() const { return users; }

const std::unordered_set<std::string>& Graph::getFriends(const std::string& id) const {
    static const std::unordered_set<std::string> empty;
    auto it = adjacency.find(id);
    return it == adjacency.end() ? empty : it->second;
}

const std::unordered_map<std::string, std::unordered_set<std::string>>& Graph::getAdjacency() const {
    return adjacency;
}

// ---------------- Persistence (JSON via json.h) ----------------
// { "users": [ {"id": "...", "name": "..."}, ... ],
//   "edges": [ ["A","B"], ... ] }
// Uses json.h (real parser/serializer with escape + unescape) instead of
// hand-rolled string search, so names with quotes/backslashes/unicode
// round-trip correctly.

bool Graph::saveToFile(const std::string& path) const {
    json::Value root = json::Value::makeObject();

    json::Value usersArr = json::Value::makeArray();
    for (const auto& [id, user] : users) {
        json::Value uv = json::Value::makeObject();
        uv["id"] = json::Value(user.id);
        uv["name"] = json::Value(user.name);
        usersArr.push_back(uv);
    }
    root["users"] = usersArr;

    std::vector<std::pair<std::string, std::string>> edges;
    for (const auto& [id, friends] : adjacency)
        for (const std::string& f : friends)
            if (id < f) edges.push_back({id, f});

    json::Value edgesArr = json::Value::makeArray();
    for (const auto& e : edges) {
        json::Value pair = json::Value::makeArray();
        pair.push_back(json::Value(e.first));
        pair.push_back(json::Value(e.second));
        edgesArr.push_back(pair);
    }
    root["edges"] = edgesArr;

    std::ofstream file(path);
    if (!file.is_open()) return false;
    file << root.dump();
    return true;
}

bool Graph::loadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::stringstream buf;
    buf << file.rdbuf();
    std::string content = buf.str();

    users.clear();
    adjacency.clear();

    if (content.empty()) return true;

    json::Value root;
    try {
        root = json::Value::parse(content);
    } catch (const std::exception&) {
        return false;
    }

    if (root.type != json::Type::Object || !root.objVal->has("users")) return false;

    const json::Value& usersArr = root.objVal->at("users");
    if (usersArr.type == json::Type::Array) {
        for (const auto& uv : *usersArr.arrVal) {
            if (uv.type != json::Type::Object) continue;
            std::string id = uv.objVal->has("id") ? uv.objVal->at("id").strVal : "";
            std::string name = uv.objVal->has("name") ? uv.objVal->at("name").strVal : "";
            if (id.empty()) continue;
            users[id] = User{id, name};
            adjacency[id];
        }
    }

    if (root.objVal->has("edges")) {
        const json::Value& edgesArr = root.objVal->at("edges");
        if (edgesArr.type == json::Type::Array) {
            for (const auto& pv : *edgesArr.arrVal) {
                if (pv.type != json::Type::Array || pv.arrVal->size() < 2) continue;
                const std::string& a = (*pv.arrVal)[0].strVal;
                const std::string& b = (*pv.arrVal)[1].strVal;
                addFriendship(a, b);
            }
        }
    }

    return true;
}
