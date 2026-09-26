#include "pathfinding.hpp"

#include <algorithm>
#include <cstdlib>
#include <functional>
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

struct TimedPoint {
	Point2D point;
	TimeStep time;

	bool operator==(const TimedPoint& other) const {
		return point == other.point && time == other.time;
	}
};

struct TimedPointHash {
	std::size_t operator()(const TimedPoint& state) const noexcept {
		const std::size_t pointHash = Point2DHash{}(state.point);
		const std::size_t timeHash = std::hash<TimeStep>{}(state.time);
		return pointHash ^ (timeHash + 0x9e3779b9U + (pointHash << 6) + (pointHash >> 2));
	}
};

struct TimedOpenNode {
	TimedPoint state;
	std::size_t cost;
	std::size_t estimatedTotalCost;
};

struct TimedHigherCostFirst {
	bool operator()(const TimedOpenNode& left, const TimedOpenNode& right) const {
		return left.estimatedTotalCost > right.estimatedTotalCost;
	}
};

TimeStep lastConstraintTime(const ConstraintTreeNode& node, AgentId agent) {
	TimeStep lastTime = 0;
	for (const Constraint& constraint : node.constraints) {
		if (const auto* vertex = std::get_if<VertexConstraint>(&constraint)) {
			if (vertex->agent == agent) {
				lastTime = std::max(lastTime, vertex->time);
			}
		} else {
			const auto& edge = std::get<EdgeConstraint>(constraint);
			if (edge.agent == agent) {
				lastTime = std::max(lastTime, edge.departureTime + 1);
			}
		}
	}
	return lastTime;
}

bool goalIsSafeAfterArrival(
	const ConstraintTreeNode& node,
	AgentId agent,
	const Point2D& goal,
	TimeStep arrivalTime
) {
	for (const Constraint& constraint : node.constraints) {
		const auto* vertex = std::get_if<VertexConstraint>(&constraint);
		if (vertex != nullptr
			&& vertex->agent == agent
			&& vertex->vertex == goal
			&& vertex->time >= arrivalTime) {
			return false;
		}
	}
	return true;
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
	return low_level(graph, start, goal, 0, ConstraintTreeNode{});
}

std::vector<Point2D> low_level(
	const GridGraph& graph,
	const Point2D& start,
	const Point2D& goal,
	AgentId agent,
	const ConstraintTreeNode& node
) {
	if (!graph.isWalkable(start.x, start.y)
		|| !graph.isWalkable(goal.x, goal.y)
		|| node.forbidsVertex(agent, start, 0)) {
		return {};
	}

	const TimedPoint initial{start, 0};
	const std::size_t timeHorizon = lastConstraintTime(node, agent)
		+ std::max<std::size_t>(1, graph.getNodeCount());
	std::priority_queue<
		TimedOpenNode,
		std::vector<TimedOpenNode>,
		TimedHigherCostFirst
	> openSet;
	std::unordered_map<TimedPoint, std::size_t, TimedPointHash> costs;
	std::unordered_map<TimedPoint, TimedPoint, TimedPointHash> parents;

	costs.emplace(initial, 0);
	openSet.push({initial, 0, static_cast<std::size_t>(heuristic(start, goal))});

	while (!openSet.empty()) {
		const TimedOpenNode current = openSet.top();
		openSet.pop();

		const auto currentCost = costs.find(current.state);
		if (currentCost == costs.end() || current.cost != currentCost->second) {
			continue;
		}

		if (current.state.point == goal
			&& goalIsSafeAfterArrival(node, agent, goal, current.state.time)) {
			std::vector<Point2D> path;
			TimedPoint state = current.state;
			while (true) {
				path.push_back(state.point);
				if (state == initial) {
					break;
				}
				const auto parent = parents.find(state);
				if (parent == parents.end()) {
					return {};
				}
				state = parent->second;
			}
			std::reverse(path.begin(), path.end());
			return path;
		}

		if (current.state.time >= timeHorizon) {
			continue;
		}

		const TimeStep nextTime = current.state.time + 1;
		const auto considerNext = [&](const Point2D& nextPoint) {
			if (node.forbidsVertex(agent, nextPoint, nextTime)
				|| node.forbidsEdge(agent, current.state.point, nextPoint, current.state.time)) {
				return;
			}

			const TimedPoint nextState{nextPoint, nextTime};
			const std::size_t nextCost = current.cost + 1;
			const auto knownCost = costs.find(nextState);
			if (knownCost != costs.end() && nextCost >= knownCost->second) {
				return;
			}

			costs[nextState] = nextCost;
			parents[nextState] = current.state;
			openSet.push({
				nextState,
				nextCost,
				nextCost + static_cast<std::size_t>(heuristic(nextPoint, goal))
			});
		};

		for (const Point2D& neighbor : graph.getNeighbors(current.state.point)) {
			considerNext(neighbor);
		}
		considerNext(current.state.point);
	}

	return {};
}
