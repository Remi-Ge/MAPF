#include "cbs.hpp"

#include <iostream>
#include <vector>

namespace {

void writePoint(const Point2D& point) {
	std::cout << '[' << point.x << ',' << point.y << ']';
}

void writePath(const std::vector<Point2D>& path) {
	std::cout << '[';
	for (std::size_t index = 0; index < path.size(); ++index) {
		if (index != 0) std::cout << ',';
		writePoint(path[index]);
	}
	std::cout << ']';
}

} // namespace

int main() {
	constexpr int width = 7;
	constexpr int height = 7;
	const std::vector<Point2D> obstacles = {{1, 1}, {5, 5}};

	GridGraph graph(width, height);
	for (const Point2D& obstacle : obstacles) {
		graph.setObstacle(obstacle.x, obstacle.y, true);
	}

	const std::vector<AgentTask> agents = {
		{{0, 3}, {6, 3}},
		{{3, 0}, {3, 6}}
	};
	const auto result = solve_cbs(graph, agents);
	if (!result.has_value()) {
		std::cerr << "CBS did not find a solution for the demo scenario.\n";
		return 1;
	}

	std::cout << "{\n  \"width\": " << width
			  << ",\n  \"height\": " << height
			  << ",\n  \"cost\": " << result->cost
			  << ",\n  \"obstacles\": [";
	for (std::size_t index = 0; index < obstacles.size(); ++index) {
		if (index != 0) std::cout << ',';
		writePoint(obstacles[index]);
	}

	std::cout << "],\n  \"agents\": [\n";
	for (std::size_t index = 0; index < agents.size(); ++index) {
		if (index != 0) std::cout << ",\n";
		std::cout << "    {\"name\": \"Robot " << index + 1
				  << "\", \"start\": ";
		writePoint(agents[index].start);
		std::cout << ", \"goal\": ";
		writePoint(agents[index].goal);
		std::cout << ", \"path\": ";
		writePath(result->pathsByAgent[index]);
		std::cout << '}';
	}
	std::cout << "\n  ]\n}\n";
	return 0;
}
