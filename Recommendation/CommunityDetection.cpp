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

    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> bridges;

    int timer=0;

    for(const auto& [id,user] : g.getAllUsers())
    {
        if(!visited.count(id))
        {
            dfsBridge(
                g,
                id,
                "",
                timer,
                tin,
                low,
                visited,
                bridges
            );
        }
    }

    visited.clear();

    std::vector<Community> result;

    int cid=1;

    for(const auto& [id,user] : g.getAllUsers())
    {
        if(visited.count(id))
            continue;

        Community c;
        c.id=cid++;

        std::queue<std::string> q;

        q.push(id);
        visited.insert(id);

        while(!q.empty())
        {
            auto cur=q.front();
            q.pop();

            c.members.push_back(cur);

            for(const auto& nxt : g.getFriends(cur))
            {
                if(visited.count(nxt))
                    continue;

                if(bridges.count(makeKey(cur,nxt)))
                    continue;

                visited.insert(nxt);
                q.push(nxt);
            }
        }

        std::sort(c.members.begin(),c.members.end());

        result.push_back(c);
    }

    std::sort(result.begin(),result.end(),
        [](const Community& a,const Community& b)
        {
            return a.members.front()<b.members.front();
        });

    return result;
}

}
