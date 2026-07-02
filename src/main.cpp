#include <iostream>
#include "GridGraph.h"
#include "AStar.h"

int main() {
    GridGraph graph(5, 10);
    std::cout << graph.toString() << std::endl;
    std::vector<Node> neighbors = graph.getNeighbors(0, 3);
    std::cout << "Neighbors of (0, 3):" << std::endl;
    for (const Node& neighbor : neighbors) {
        std::cout << "(" << neighbor.x << ", " << neighbor.y << ")" << std::endl;
    }
    AStar astar;
    Node start(0, 0);
    Node goal(4, 9);
    std::vector<Node> path = astar.aStar(graph, start, goal);
    std::cout << "Path from (0, 0) to (4, 9):" << std::endl;
    for (const Node& node : path) {
        std::cout << "(" << node.x << ", " << node.y << ")" << std::endl;
    }
    return 0;
}