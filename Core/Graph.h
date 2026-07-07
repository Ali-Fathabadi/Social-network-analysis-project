#pragma once
#include "User.h"
#include <unordered_map>
#include <unordered_set>
#include <string>

class Graph {
public:
    
    bool addUser(int id, const std::string& name);
    bool removeUser(int id);
    bool editUser(int id, const std::string& newName);

    bool addFriendship(int id1, int id2);
    bool removeFriendship(int id1, int id2);

   
    bool hasUser(int id) const;
    const User* getUser(int id) const;
    const std::unordered_map<int, User>& getAllUsers() const;
    const std::unordered_set<int>& getFriends(int id) const;
    const std::unordered_map<int, std::unordered_set<int>>& getAdjacency() const;
    size_t userCount() const { return users.size(); }

 
    bool saveToFile(const std::string& path) const;
    bool loadFromFile(const std::string& path);

private:
    std::unordered_map<int, User> users;                         
    std::unordered_map<int, std::unordered_set<int>> adjacency;   
};
