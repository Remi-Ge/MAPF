#include "AStar.h"

#include <map>
#include <queue>
#include <algorithm>

std::vector<Node> AStar::aStar(const GridGraph& graph, Node start, Node goal) {
    struct Comparator {
        bool operator()(const std::pair<int, Node>& a, const std::pair<int, Node>& b) const {
            return a.first > b.first;
        }
    };

    std::priority_queue<std::pair<int, Node>, std::vector<std::pair<int, Node>>, Comparator> openSet;
    openSet.push({0, start});
    std::map<Node, Node> cameFrom;
    std::map<Node, int> gScore;
    gScore[start] = 0;

    while (!openSet.empty()) {
        Node currentNode = openSet.top().second;
        openSet.pop();

        if (currentNode == goal) {
            std::vector<Node> path;
            Node node = currentNode;
            while (!(node == start)) {
                path.push_back(node);
                node = cameFrom[node];
            }
            path.push_back(start);
            std::reverse(path.begin(), path.end());
            return path;
        }

        for (Node neighbor : graph.getNeighbors(currentNode.x, currentNode.y)) {
            int tentativeG = gScore[currentNode] + 1;

            if (gScore.find(neighbor) == gScore.end() || tentativeG < gScore[neighbor]) {
                cameFrom[neighbor] = currentNode;
                gScore[neighbor] = tentativeG;
                int dx = neighbor.x > goal.x ? neighbor.x - goal.x : goal.x - neighbor.x;
                int dy = neighbor.y > goal.y ? neighbor.y - goal.y : goal.y - neighbor.y;
                int fScore = tentativeG + dx + dy;
                openSet.push({fScore, neighbor});
            }
        }
    }

    return std::vector<Node>(); // No path found
}   