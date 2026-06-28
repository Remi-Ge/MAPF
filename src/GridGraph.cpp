#include "GridGraph.h"

GridGraph::GridGraph(int width, int height)
    : width(width), height(height) {}

std::string GridGraph::toString() {
    return "Graph Size: " + std::to_string(width) + "x" + std::to_string(height);
}