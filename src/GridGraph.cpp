#include "GridGraph.h"

bool Node::operator<(const Node& other) const {
    return x < other.x || (x == other.x && y < other.y);
}

bool Node::operator==(const Node& other) const {
    return x == other.x && y == other.y;
}

GridGraph::GridGraph(int width, int height)
    : width(width), height(height) {}

std::string GridGraph::toString() {
    return "Graph Size: " + std::to_string(width) + "x" + std::to_string(height);
}

std::vector<Node> GridGraph::getNeighbors(int x, int y) const {
    std::vector<Node> neighbors;
    neighbors.emplace_back(x, y); // wait 
    if (x + 1 < width) neighbors.emplace_back(x + 1, y); // right 
    if (x - 1 >= 0) neighbors.emplace_back(x - 1, y); // left 
    if (y + 1 < height) neighbors.emplace_back(x, y + 1); // up 
    if (y - 1 >= 0) neighbors.emplace_back(x, y - 1); // down
    return neighbors;
}