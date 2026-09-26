#include "cbs.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
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

bool parseInteger(const char* text, int& value) {
	char* end = nullptr;
	const long parsed = std::strtol(text, &end, 10);
	if (end == text || *end != '\0'
		|| parsed < 0 || parsed > 1000000) {
		return false;
	}
	value = static_cast<int>(parsed);
	return true;
}

} // namespace

int main(int argc, char* argv[]) {
	int width = 7;
	int height = 7;
	std::vector<Point2D> obstacles = {{1, 1}, {5, 5}};
	std::vector<AgentTask> agents = {
		{{0, 3}, {6, 3}},
		{{3, 0}, {3, 6}}
	};

	if (argc > 1) {
		if (std::string(argv[1]) != "--solve") {
			std::cerr << "Usage: mapf_demo [--solve width height obstacle-count "
				"obstacle-x obstacle-y ... agent-count start-x start-y goal-x goal-y ...]\n";
			return 2;
		}

		int obstacleCount = 0;
		int agentCount = 0;
		int cursor = 2;
		if (argc < 6 || !parseInteger(argv[cursor++], width)
			|| !parseInteger(argv[cursor++], height)
			|| !parseInteger(argv[cursor++], obstacleCount)
			|| width < 1 || height < 1 || width > 40 || height > 40
			|| obstacleCount > width * height) {
			std::cerr << "Invalid grid dimensions or obstacle count.\n";
			return 2;
		}

		obstacles.clear();
		for (int index = 0; index < obstacleCount; ++index) {
			int x = 0;
			int y = 0;
			if (cursor + 1 >= argc || !parseInteger(argv[cursor++], x)
				|| !parseInteger(argv[cursor++], y)) {
				std::cerr << "Invalid obstacle coordinates.\n";
				return 2;
			}
			obstacles.push_back({x, y});
		}

		if (cursor >= argc || !parseInteger(argv[cursor++], agentCount)
			|| agentCount < 1 || agentCount > 8
			|| argc != cursor + agentCount * 4) {
			std::cerr << "Invalid agent count or coordinates.\n";
			return 2;
		}

		agents.clear();
		for (int index = 0; index < agentCount; ++index) {
			int startX = 0;
			int startY = 0;
			int goalX = 0;
			int goalY = 0;
			if (!parseInteger(argv[cursor++], startX)
				|| !parseInteger(argv[cursor++], startY)
				|| !parseInteger(argv[cursor++], goalX)
				|| !parseInteger(argv[cursor++], goalY)) {
				std::cerr << "Invalid agent coordinates.\n";
				return 2;
			}
			agents.push_back({{startX, startY}, {goalX, goalY}});
		}
	}

	GridGraph graph(width, height);
	for (const Point2D& obstacle : obstacles) {
		if (obstacle.x < 0 || obstacle.x >= width || obstacle.y < 0 || obstacle.y >= height) {
			std::cerr << "Obstacle is outside the grid.\n";
			return 2;
		}
		graph.setObstacle(obstacle.x, obstacle.y, true);
	}

	for (const AgentTask& agent : agents) {
		if (!graph.isWalkable(agent.start.x, agent.start.y)
			|| !graph.isWalkable(agent.goal.x, agent.goal.y)) {
			std::cerr << "An agent start or goal is blocked or outside the grid.\n";
			return 2;
		}
	}

	const auto result = solve_cbs(graph, agents);
	if (!result.has_value()) {
		std::cerr << "CBS did not find a conflict-free solution.\n";
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
