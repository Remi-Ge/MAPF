#pragma once

#include "graph.hpp"

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <utility>
#include <variant>
#include <vector>

using AgentId = std::size_t;
using TimeStep = std::size_t;
using ConstraintNodeId = std::size_t;

struct VertexConstraint {
    AgentId agent;
    Point2D vertex;
    TimeStep time;
};

struct EdgeConstraint {
    AgentId agent;
    Point2D from;
    Point2D to;
    TimeStep departureTime;
};

using Constraint = std::variant<VertexConstraint, EdgeConstraint>;

struct ConstraintTreeNode {
    std::optional<ConstraintNodeId> parent;
    std::vector<ConstraintNodeId> children;
    std::vector<Constraint> constraints;
    std::vector<std::vector<Point2D>> pathsByAgent;
    std::size_t cost = 0;

    bool forbidsVertex(AgentId agent, const Point2D& vertex, TimeStep time) const {
        for (const Constraint& constraint : constraints) {
            const auto* vertexConstraint = std::get_if<VertexConstraint>(&constraint);
            if (vertexConstraint != nullptr
                && vertexConstraint->agent == agent
                && vertexConstraint->vertex == vertex
                && vertexConstraint->time == time) {
                return true;
            }
        }
        return false;
    }

    bool forbidsEdge(
        AgentId agent,
        const Point2D& from,
        const Point2D& to,
        TimeStep departureTime
    ) const {
        for (const Constraint& constraint : constraints) {
            const auto* edgeConstraint = std::get_if<EdgeConstraint>(&constraint);
            if (edgeConstraint != nullptr
                && edgeConstraint->agent == agent
                && edgeConstraint->from == from
                && edgeConstraint->to == to
                && edgeConstraint->departureTime == departureTime) {
                return true;
            }
        }
        return false;
    }
};

class ConstraintTree {
public:
    ConstraintNodeId addRoot(std::vector<std::vector<Point2D>> pathsByAgent) {
        if (!nodes.empty()) {
            throw std::logic_error("Constraint tree already has a root");
        }

        ConstraintTreeNode root;
        root.pathsByAgent = std::move(pathsByAgent);
        root.cost = sumOfCosts(root.pathsByAgent);
        nodes.push_back(std::move(root));
        return 0;
    }

    ConstraintNodeId addChild(
        ConstraintNodeId parentId,
        Constraint constraint,
        std::vector<std::vector<Point2D>> pathsByAgent
    ) {
        const ConstraintTreeNode& parent = nodes.at(parentId);

        ConstraintTreeNode child;
        child.parent = parentId;
        child.constraints = parent.constraints;
        child.constraints.push_back(std::move(constraint));
        child.pathsByAgent = std::move(pathsByAgent);
        child.cost = sumOfCosts(child.pathsByAgent);

        const ConstraintNodeId childId = nodes.size();
        nodes.push_back(std::move(child));
        nodes.at(parentId).children.push_back(childId);
        return childId;
    }

    const ConstraintTreeNode& at(ConstraintNodeId nodeId) const {
        return nodes.at(nodeId);
    }

    std::size_t size() const {
        return nodes.size();
    }

private:
    std::vector<ConstraintTreeNode> nodes;

    static std::size_t sumOfCosts(const std::vector<std::vector<Point2D>>& paths) {
        std::size_t total = 0;
        for (const auto& path : paths) {
            if (!path.empty()) {
                total += path.size() - 1;
            }
        }
        return total;
    }
};