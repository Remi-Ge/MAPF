# pragma once

#include <string>
#include <vector>

struct Node {
    int x, y;
    
    Node() : x(0), y(0) {} 
    Node(int x, int y) : x(x), y(y) {}

    bool operator<(const Node& other) const;
    bool operator==(const Node& other) const;
};

class GridGraph {
public:
    GridGraph(int width, int height);
    std::string toString();
    std::vector<Node> getNeighbors(int x, int y) const;
private:
    int width;
    int height;
};