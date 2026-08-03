#include "FriendRecommendation.h"
#include <unordered_map>
#include <algorithm>

namespace algo {

std::vector<FriendSuggestion>
recommendFriends(const Graph& g, const std::string& userId)
{
    std::vector<FriendSuggestion> result;

    if (!g.findUser(userId))
        return result;

    const auto& myFriends = g.getFriends(userId);

    std::unordered_map<std::string,int> score;

    // دوستانِ دوستان
    for (const std::string& f : myFriends)
    {
        for (const std::string& candidate : g.getFriends(f))
        {
            if (candidate == userId)
                continue;

            if (myFriends.count(candidate))
                continue;

            score[candidate]++;
        }
    }

    

}
