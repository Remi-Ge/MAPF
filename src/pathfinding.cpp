#include "graph.cpp"

#include <algorithm>
#include <cstdlib>
#include <queue>
#include <unordered_map>
#include <vector>

namespace {

struct OpenNode {
	Point2D point;
	int cost;
	int estimatedTotalCost;
};

struct HigherCostFirst {
	bool operator()(const OpenNode& left, const OpenNode& right) const {
		return left.estimatedTotalCost > right.estimatedTotalCost;
	}
};

int heuristic(const Point2D& from, const Point2D& to) {
	return std::abs(from.x - to.x) + std::abs(from.y - to.y);
}

} // namespace

std::vector<Point2D> a_star(
	const GridGraph& graph,
	const Point2D& start,
	const Point2D& goal
) {
	if (!graph.isWalkable(start.x, start.y) || !graph.isWalkable(goal.x, goal.y)) {
		return {};
	}

	std::priority_queue<OpenNode, std::vector<OpenNode>, HigherCostFirst> openSet;
	std::unordered_map<Point2D, int, Point2DHash> costs;
	std::unordered_map<Point2D, Point2D, Point2DHash> parents;

	costs.emplace(start, 0);
	openSet.push({start, 0, heuristic(start, goal)});

	while (!openSet.empty()) {
		const OpenNode current = openSet.top();
		openSet.pop();

		const auto currentCost = costs.find(current.point);
		if (currentCost == costs.end() || current.cost != currentCost->second) {
			continue;
		}

		if (current.point == goal) {
			std::vector<Point2D> path{goal};
			Point2D point = goal;
			while (point != start) {
				const auto parent = parents.find(point);
				if (parent == parents.end()) {
					return {};
				}
				point = parent->second;
				path.push_back(point);
			}
			std::reverse(path.begin(), path.end());
			return path;
		}

		for (const Point2D& neighbor : graph.getNeighbors(current.point)) {
			const int nextCost = current.cost + 1;
			const auto knownCost = costs.find(neighbor);
			if (knownCost != costs.end() && nextCost >= knownCost->second) {
				continue;
			}

			costs[neighbor] = nextCost;
			parents[neighbor] = current.point;
			openSet.push({neighbor, nextCost, nextCost + heuristic(neighbor, goal)});
		}
	}

	return {};
}

std::vector<Point2D> low_level(
	const GridGraph& graph,
	const Point2D& start,
	const Point2D& goal
) {
	return a_star(graph, start, goal);
}
