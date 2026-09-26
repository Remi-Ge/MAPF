#include "cbs.hpp"

#include <algorithm>
#include <cassert>
#include <vector>

namespace {

Point2D positionAt(const std::vector<Point2D>& path, std::size_t time) {
    return path[std::min(time, path.size() - 1)];
}

bool pathsAreConflictFree(const std::vector<std::vector<Point2D>>& paths) {
    std::size_t makespan = 0;
    for (const auto& path : paths) {
        if (path.empty()) return false;
        makespan = std::max(makespan, path.size());
    }

    for (std::size_t time = 0; time < makespan; ++time) {
        for (std::size_t first = 0; first < paths.size(); ++first) {
            for (std::size_t second = first + 1; second < paths.size(); ++second) {
                if (positionAt(paths[first], time) == positionAt(paths[second], time)) {
                    return false;
                }
                if (time + 1 < makespan) {
                    const Point2D firstFrom = positionAt(paths[first], time);
                    const Point2D firstTo = positionAt(paths[first], time + 1);
                    const Point2D secondFrom = positionAt(paths[second], time);
                    const Point2D secondTo = positionAt(paths[second], time + 1);
                    if (firstFrom != firstTo
                        && firstFrom == secondTo
                        && firstTo == secondFrom) {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

} // namespace

int main() {
    GridGraph vertexGraph(3, 3);
    const auto vertexResult = solve_cbs(vertexGraph, {
        {{0, 1}, {2, 1}},
        {{1, 0}, {1, 2}}
    });
    assert(vertexResult.has_value());
    assert(vertexResult->pathsByAgent.size() == 2);
    assert(pathsAreConflictFree(vertexResult->pathsByAgent));
    assert(vertexResult->cost == 5);

    GridGraph edgeGraph(3, 3);
    const auto edgeResult = solve_cbs(edgeGraph, {
        {{0, 1}, {1, 1}},
        {{1, 1}, {0, 1}}
    });
    assert(edgeResult.has_value());
    assert(pathsAreConflictFree(edgeResult->pathsByAgent));

    GridGraph manyAgentsGraph(2, 9);
    std::vector<AgentTask> manyAgents;
    for (int row = 0; row < 9; ++row) {
        manyAgents.push_back({{0, row}, {1, row}});
    }
    const auto manyAgentsResult = solve_cbs(manyAgentsGraph, manyAgents);
    assert(manyAgentsResult.has_value());
    assert(manyAgentsResult->pathsByAgent.size() == 9);
    assert(manyAgentsResult->cost == 9);
    assert(pathsAreConflictFree(manyAgentsResult->pathsByAgent));

    GridGraph disconnectedGraph(3, 1);
    disconnectedGraph.setObstacle(1, 0, true);
    assert(!solve_cbs(disconnectedGraph, {{{0, 0}, {2, 0}}}).has_value());

    return 0;
}