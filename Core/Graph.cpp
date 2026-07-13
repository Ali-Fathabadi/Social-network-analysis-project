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
