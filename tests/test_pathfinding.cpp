#include "pathfinding.cpp"

#include <cassert>
#include <cstdlib>

int main() {
    GridGraph graph(5, 5);
    for (int y = 0; y < 4; ++y) {
        graph.setObstacle(2, y, true);
    }

    const Point2D start{0, 0};
    const Point2D goal{4, 0};
    const std::vector<Point2D> path = low_level(graph, start, goal);

    assert(path.size() == 13);
    assert(path.front() == start);
    assert(path.back() == goal);
    for (std::size_t index = 1; index < path.size(); ++index) {
        assert(graph.isWalkable(path[index].x, path[index].y));
        assert(std::abs(path[index].x - path[index - 1].x)
            + std::abs(path[index].y - path[index - 1].y) == 1);
    }

    graph.setObstacle(2, 4, true);
    assert(low_level(graph, start, goal).empty());
    assert(low_level(graph, start, start) == std::vector<Point2D>({start}));

    return 0;
}