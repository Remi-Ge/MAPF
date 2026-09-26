#pragma once

#include "graph.hpp"

#include <cstddef>
#include <optional>
#include <vector>

struct AgentTask {
    Point2D start;
    Point2D goal;
};

struct CBSResult {
    std::vector<std::vector<Point2D>> pathsByAgent;
    std::size_t cost;
};

std::optional<CBSResult> solve_cbs(
    const GridGraph& graph,
    const std::vector<AgentTask>& agents
);