#include "constraint_tree.hpp"

#include <cassert>
#include <vector>

int main() {
    ConstraintTree tree;
    const std::vector<std::vector<Point2D>> rootPaths = {
        {{0, 0}, {1, 0}},
        {{2, 0}, {2, 1}}
    };

    const ConstraintNodeId rootId = tree.addRoot(rootPaths);
    assert(rootId == 0);
    assert(tree.at(rootId).cost == 2);

    const std::vector<std::vector<Point2D>> vertexBranchPaths = {
        {{0, 0}, {0, 1}, {1, 1}},
        {{2, 0}, {2, 1}}
    };
    const ConstraintNodeId vertexChildId = tree.addChild(
        rootId,
        VertexConstraint{0, {1, 0}, 1},
        vertexBranchPaths
    );
    const ConstraintTreeNode& vertexChild = tree.at(vertexChildId);

    assert(vertexChild.parent == rootId);
    assert(vertexChild.constraints.size() == 1);
    assert(vertexChild.cost == 3);
    assert(vertexChild.forbidsVertex(0, {1, 0}, 1));
    assert(!vertexChild.forbidsVertex(1, {1, 0}, 1));
    assert(!vertexChild.forbidsVertex(0, {1, 0}, 2));
    assert(tree.at(rootId).children == std::vector<ConstraintNodeId>({vertexChildId}));

    const ConstraintNodeId edgeChildId = tree.addChild(
        rootId,
        EdgeConstraint{1, {2, 0}, {2, 1}, 0},
        rootPaths
    );
    const ConstraintTreeNode& edgeChild = tree.at(edgeChildId);

    assert(edgeChild.parent == rootId);
    assert(edgeChild.constraints.size() == 1);
    assert(edgeChild.forbidsEdge(1, {2, 0}, {2, 1}, 0));
    assert(!edgeChild.forbidsEdge(0, {2, 0}, {2, 1}, 0));
    assert(!edgeChild.forbidsEdge(1, {2, 1}, {2, 0}, 0));
    assert(!edgeChild.forbidsEdge(1, {2, 0}, {2, 1}, 1));
    assert(!vertexChild.forbidsEdge(1, {2, 0}, {2, 1}, 0));
    assert(tree.size() == 3);

    return 0;
}