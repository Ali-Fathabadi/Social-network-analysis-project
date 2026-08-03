#include "CommunityDetection.h"

#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <queue>

namespace algo {

namespace
{

using Edge = std::pair<std::string,std::string>;

std::string makeKey(std::string a,std::string b)
{
    if(a>b)
        std::swap(a,b);

    return a + "|" + b;
}

void dfsBridge(
    const Graph& g,
    const std::string& u,
    const std::string& parent,
    int& timer,

    std::unordered_map<std::string,int>& tin,
    std::unordered_map<std::string,int>& low,

    std::unordered_set<std::string>& visited,
    std::unordered_set<std::string>& bridges)
{
    visited.insert(u);

    tin[u]=low[u]=timer++;

    for(const auto& v : g.getFriends(u))
    {
        if(v==parent)
            continue;

        if(visited.count(v))
        {
            low[u]=std::min(low[u],tin[v]);
        }
        else
        {
            dfsBridge(g,v,u,timer,tin,low,visited,bridges);

            low[u]=std::min(low[u],low[v]);

            if(low[v]>tin[u])
            {
                bridges.insert(makeKey(u,v));
            }
        }
    }
}

}

std::vector<Community>
communityDetection(const Graph& g)
{
    std::unordered_map<std::string,int> tin;
    std::unordered_map<std::string,int> low;

 
