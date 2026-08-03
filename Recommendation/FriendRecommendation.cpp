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

    for (const auto& p : score)
    {
        result.push_back({p.first,p.second});
    }

    std::sort(result.begin(), result.end(),
        [](const FriendSuggestion& a,
           const FriendSuggestion& b)
        {
            if (a.mutualCount != b.mutualCount)
                return a.mutualCount > b.mutualCount;

            return a.id < b.id;
        });

    return result;
}

}
