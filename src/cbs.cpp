#include "cbs.hpp"

#include "constraint_tree.hpp"
#include "pathfinding.hpp"

#include <algorithm>
#include <queue>
#include <utility>

namespace {

enum class ConflictType {
    Vertex,
    Edge
};

struct Conflict {
    ConflictType type;
    AgentId firstAgent;
    AgentId secondAgent;
    TimeStep time;
    Point2D vertex{};
    Point2D firstFrom{};
    Point2D firstTo{};
    Point2D secondFrom{};
    Point2D secondTo{};
};

struct OpenConstraintNode {
    ConstraintNodeId nodeId;
    std::size_t cost;
};

struct HigherCostFirst {
    bool operator()(
        const OpenConstraintNode& left,
        const OpenConstraintNode& right
    ) const {
        if (left.cost == right.cost) {
            return left.nodeId > right.nodeId;
        }
        return left.cost > right.cost;
    }
};

Point2D positionAt(const std::vector<Point2D>& path, TimeStep time) {
    return path[std::min(time, path.size() - 1)];
}

std::optional<Conflict> findFirstConflict(
    const std::vector<std::vector<Point2D>>& paths
) {
    TimeStep makespan = 0;
    for (const auto& path : paths) {
        makespan = std::max(makespan, path.size());
    }

    for (TimeStep time = 0; time < makespan; ++time) {
        for (AgentId first = 0; first < paths.size(); ++first) {
            for (AgentId second = first + 1; second < paths.size(); ++second) {
                const Point2D firstPosition = positionAt(paths[first], time);
                const Point2D secondPosition = positionAt(paths[second], time);
                if (firstPosition == secondPosition) {
                    return Conflict{
                        ConflictType::Vertex,
                        first,
                        second,
                        time,
                        firstPosition
                    };
                }
            }
        }

        if (time + 1 >= makespan) {
            continue;
        }

        for (AgentId first = 0; first < paths.size(); ++first) {
            for (AgentId second = first + 1; second < paths.size(); ++second) {
                const Point2D firstFrom = positionAt(paths[first], time);
                const Point2D firstTo = positionAt(paths[first], time + 1);
                const Point2D secondFrom = positionAt(paths[second], time);
                const Point2D secondTo = positionAt(paths[second], time + 1);

                if (firstFrom != firstTo
                    && firstFrom == secondTo
                    && firstTo == secondFrom) {
                    return Conflict{
                        ConflictType::Edge,
                        first,
                        second,
                        time,
                        {},
                        firstFrom,
                        firstTo,
                        secondFrom,
                        secondTo
                    };
                }
            }
        }
    }

    return std::nullopt;
}

} // namespace

std::optional<CBSResult> solve_cbs(
    const GridGraph& graph,
    const std::vector<AgentTask>& agents
) {
    ConstraintTreeNode unconstrained;
    std::vector<std::vector<Point2D>> rootPaths;
    rootPaths.reserve(agents.size());

    for (AgentId agent = 0; agent < agents.size(); ++agent) {
        std::vector<Point2D> path = low_level(
            graph,
            agents[agent].start,
            agents[agent].goal,
            agent,
            unconstrained
        );
        if (path.empty()) {
            return std::nullopt;
        }
        rootPaths.push_back(std::move(path));
    }

    ConstraintTree tree;
    const ConstraintNodeId rootId = tree.addRoot(std::move(rootPaths));
    std::priority_queue<
        OpenConstraintNode,
        std::vector<OpenConstraintNode>,
        HigherCostFirst
    > openSet;
    openSet.push({rootId, tree.at(rootId).cost});

    while (!openSet.empty()) {
        const ConstraintNodeId nodeId = openSet.top().nodeId;
        openSet.pop();
        const ConstraintTreeNode current = tree.at(nodeId);
        const std::optional<Conflict> conflict = findFirstConflict(current.pathsByAgent);

        if (!conflict.has_value()) {
            return CBSResult{current.pathsByAgent, current.cost};
        }

        const auto addChild = [&](AgentId agent, Constraint constraint) {
            ConstraintTreeNode replanningNode = current;
            replanningNode.constraints.push_back(constraint);

            std::vector<Point2D> newPath = low_level(
                graph,
                agents[agent].start,
                agents[agent].goal,
                agent,
                replanningNode
            );
            if (newPath.empty()) {
                return;
            }

            std::vector<std::vector<Point2D>> childPaths = current.pathsByAgent;
            childPaths[agent] = std::move(newPath);
            const ConstraintNodeId childId = tree.addChild(
                nodeId,
                std::move(constraint),
                std::move(childPaths)
            );
            openSet.push({childId, tree.at(childId).cost});
        };

        if (conflict->type == ConflictType::Vertex) {
            addChild(
                conflict->firstAgent,
                VertexConstraint{conflict->firstAgent, conflict->vertex, conflict->time}
            );
            addChild(
                conflict->secondAgent,
                VertexConstraint{conflict->secondAgent, conflict->vertex, conflict->time}
            );
        } else {
            addChild(
                conflict->firstAgent,
                EdgeConstraint{
                    conflict->firstAgent,
                    conflict->firstFrom,
                    conflict->firstTo,
                    conflict->time
                }
            );
            addChild(
                conflict->secondAgent,
                EdgeConstraint{
                    conflict->secondAgent,
                    conflict->secondFrom,
                    conflict->secondTo,
                    conflict->time
                }
            );
        }
    }

    return std::nullopt;
}