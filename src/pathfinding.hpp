#pragma once

#include "constraint_tree.hpp"

#include <vector>

std::vector<Point2D> a_star(
    const GridGraph& graph,
    const Point2D& start,
    const Point2D& goal
);

std::vector<Point2D> low_level(
    const GridGraph& graph,
    const Point2D& start,
    const Point2D& goal
);

std::vector<Point2D> low_level(
    const GridGraph& graph,
    const Point2D& start,
    const Point2D& goal,
    AgentId agent,
    const ConstraintTreeNode& node
);