#pragma once

#include "User.h"

#include <cstddef>
#include <string>
#include <unordered_map>
#include <unordered_set>

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
    size_t friendshipCount() const;

    bool loadFromFile(const std::string& path, std::string* error = nullptr);

    bool saveToFile(const std::string& path, std::string* error = nullptr) const;

private:
    std::unordered_map<std::string, User> users;
    std::unordered_map<std::string, std::unordered_set<std::string>> adjacency;
};
