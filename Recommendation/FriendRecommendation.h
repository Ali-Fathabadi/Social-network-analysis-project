#pragma once
#include "Graph.h"
#include <string>
#include <vector>

namespace algo {

struct FriendSuggestion {
    std::string id;
    int mutualCount;
};

std::vector<FriendSuggestion>
recommendFriends(const Graph& g, const std::string& userId);

}
