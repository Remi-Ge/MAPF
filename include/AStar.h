# pragma once

#include <vector>
#include <unordered_map>
#include "GridGraph.h"

class AStar {
public:
    std::vector<Node> aStar(const GridGraph& graph, Node start, Node goal);
private:
    std::vector<Node> reconstructPath(Node current, const std::unordered_map<Node, Node>& cameFrom);
};