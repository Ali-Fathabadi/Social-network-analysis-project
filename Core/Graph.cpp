#include "Graph.h"
#include <fstream>
#include <sstream>
#include <vector>



bool Graph::addUser(int id, const std::string& name) {
    if (users.count(id)) return false; 
    users[id] = User{id, name};
    adjacency[id]; // مجموعه دوستان خالی ایجاد می شود
    return true;
}

bool Graph::removeUser(int id) {
    if (!users.count(id)) return false;
    // حذف کاربر از لیست دوستان همه ی دوستانش
    for (int friendId : adjacency[id]) {
        adjacency[friendId].erase(id);
    }
    adjacency.erase(id);
    users.erase(id);
    return true;
}

bool Graph::editUser(int id, const std::string& newName) {
    auto it = users.find(id);
    if (it == users.end()) return false;
    it->second.name = newName;
    return true;
}

bool Graph::addFriendship(int id1, int id2) {
    if (id1 == id2  !users.count(id1)  !users.count(id2)) return false;
    adjacency[id1].insert(id2);
    adjacency[id2].insert(id1);
    return true;
}

bool Graph::removeFriendship(int id1, int id2) {
    if (!users.count(id1) || !users.count(id2)) return false;
    adjacency[id1].erase(id2);
    adjacency[id2].erase(id1);
    return true;
}


bool Graph::hasUser(int id) const { return users.count(id) > 0; }

const User* Graph::getUser(int id) const {
    auto it = users.find(id);
    return it == users.end() ? nullptr : &it->second;
}

const std::unordered_map<int, User>& Graph::getAllUsers() const { return users; }
const std::unordered_set<int>& Graph::getFriends(int id) const {
    static const std::unordered_set<int> empty;
    auto it = adjacency.find(id);
    return it == adjacency.end() ? empty : it->second;
}

const std::unordered_map<int, std::unordered_set<int>>& Graph::getAdjacency() const {
    return adjacency;
}
static std::string escapeJson(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c == '"' || c == '\\') out += '\\';
        out += c;
    }
    return out;
}


static size_t findMatching(const std::string& s, size_t start) {
    char open = s[start];
    char close = (open == '[') ? ']' : '}';
    int depth = 0;
    for (size_t i = start; i < s.size(); ++i) {
        if (s[i] == open) depth++;
        else if (s[i] == close) {
            depth--;
            if (depth == 0) return i;
        }
    }
    return std::string::npos;
}


bool Graph::saveToFile(const std::string& path) const {
    std::ofstream file(path);
    if (!file.is_open()) return false;

    file << "{\n  \"users\": [\n";
    size_t count = 0, total = users.size();
    for (const auto& [id, user] : users) {
        file << "    {\"id\": " << user.id << ", \"name\": \"" << escapeJson(user.name) << "\"}";
        if (++count < total) file << ",";
        file << "\n";
    }
    file << "  ],\n  \"edges\": [\n";

   
    std::vector<std::pair<int, int>> edges;
    for (const auto& [id, friends] : adjacency)
        for (int f : friends)
            if (id < f) edges.push_back({id, f});

    for (size_t i = 0; i < edges.size(); ++i) {
        file << "    [" << edges[i].first << ", " << edges[i].second << "]";
        if (i + 1 < edges.size()) file << ",";
        file << "\n";
    }
    file << "  ]\n}\n";
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

    
    size_t usersPos = content.find("\"users\"");
    if (usersPos == std::string::npos) return false;
    size_t usersArrStart = content.find('[', usersPos);
    size_t usersArrEnd = findMatching(content, usersArrStart);
    std::string usersBlock = content.substr(usersArrStart, usersArrEnd - usersArrStart + 1);

    size_t pos = 0;
    while ((pos = usersBlock.find('{', pos)) != std::string::npos) {
        size_t objEnd = findMatching(usersBlock, pos);
        std::string obj = usersBlock.substr(pos, objEnd - pos + 1);

        size_t idPos = obj.find("\"id\"");
        size_t idColon = obj.find(':', idPos);
        size_t idEnd = obj.find_first_of(",}", idColon);
        int id = std::stoi(obj.substr(idColon + 1, idEnd - idColon - 1));

        size_t namePos = obj.find("\"name\"");
        size_t nameColon = obj.find(':', namePos);
        size_t q1 = obj.find('"', nameColon + 1);
        size_t q2 = obj.find('"', q1 + 1);
        std::string name = obj.substr(q1 + 1, q2 - q1 - 1);

        users[id] = User{id, name};
        adjacency[id];
        pos = objEnd + 1;
    }

    
    size_t edgesPos = content.find("\"edges\"");
    if (edgesPos == std::string::npos) return true; // بدون یال هم مجاز است
    size_t edgesArrStart = content.find('[', edgesPos);
    size_t edgesArrEnd = findMatching(content, edgesArrStart);
    std::string edgesBlock = content.substr(edgesArrStart, edgesArrEnd - edgesArrStart + 1);

    pos = 0;
    while ((pos = edgesBlock.find('[', pos + 1)) != std::string::npos) {
        size_t objEnd = findMatching(edgesBlock, pos);
        std::string pairStr = edgesBlock.substr(pos + 1, objEnd - pos - 1);
size_t comma = pairStr.find(',');
        if (comma == std::string::npos) { pos = objEnd; continue; }
        int a = std::stoi(pairStr.substr(0, comma));
        int b = std::stoi(pairStr.substr(comma + 1));
        addFriendship(a, b);
        pos = objEnd;
    }

    return true;
}
