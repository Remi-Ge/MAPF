#include "graph.cpp"

#include <cassert>
#include <iostream>
#include <vector>

int main() {
    GridGraph graph(3, 2);

    assert(graph.isWalkable(0, 0));
    assert(graph.isWalkable(2, 1));
    assert(!graph.isWalkable(-1, 0));
    assert(!graph.isWalkable(3, 1));

    const std::vector<Point2D> cornerNeighbors = graph.getNeighbors({0, 0});
    assert(cornerNeighbors == std::vector<Point2D>({{1, 0}, {0, 1}}));

    graph.setObstacle(1, 0, true);
    assert(!graph.isWalkable(1, 0));
    assert(graph.getNeighbors({0, 0}) == std::vector<Point2D>({{0, 1}}));

    graph.setObstacle(1, 0, false);
    assert(graph.isWalkable(1, 0));
    assert(graph.getNeighbors({0, 0}) == cornerNeighbors);

    graph.setObstacle(3, 1, true);
    assert(graph.getNeighbors({2, 1}) == std::vector<Point2D>({{1, 1}, {2, 0}}));

    std::cout << "GridGraph tests passed\n";
    return 0;
}