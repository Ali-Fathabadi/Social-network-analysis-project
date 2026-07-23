#pragma once
#include "User.h"
#include <unordered_map>
#include <unordered_set>
#include <string>

class Graph {
public:
    
    bool addUser(const std::string& id, const std::string& name);
    bool removeUser(const std::string& id);
    bool editUser(const std::string& id, const std::string& newName);

    bool addFriendship(const std::string& id1, const std::string& id2);
    bool removeFriendship(const std::string& id1, const std::string& id2);

    
    bool findUser(const std::string& id) const;   
    const User* getUser(const std::string& id) const;
    bool areFriends(const std::string& id1, const std::string& id2) const;
    const std::unordered_map<std::string, User>& getAllUsers() const;
    const std::unordered_set<std::string>& getFriends(const std::string& id) const;
    const std::unordered_map<std::string, std::unordered_set<std::string>>& getAdjacency() const;
    size_t userCount() const { return users.size(); }

    // ذخیره و بازیابی JSON
    bool saveToFile(const std::string& path) const;
    bool loadFromFile(const std::string& path);

private:
    std::unordered_map<std::string, User> users;                                 
    std::unordered_map<std::string, std::unordered_set<std::string>> adjacency;  
};
