#pragma once
#include "Graph.h"
#include <string>
#include <vector>

namespace algo {

struct Community
{
    int id;
    std::vector<std::string> members;
};

std::vector<Community>
communityDetection(const Graph& g);

}
