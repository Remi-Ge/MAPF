#pragma once

#include <cstddef>
#include <functional>
#include <vector>

template <typename NodeId>
class IGraph {
public:
    virtual std::vector<NodeId> getNeighbors(const NodeId& node) const = 0;
};

struct Point2D {
    int x;
    int y;

    bool operator==(const Point2D& other) const {
        return x == other.x && y == other.y;
    }
    bool operator!=(const Point2D& other) const {
        return !(*this == other);
    }
};

struct Point2DHash {
    std::size_t operator()(const Point2D& point) const noexcept {
        const std::size_t xHash = std::hash<int>{}(point.x);
        const std::size_t yHash = std::hash<int>{}(point.y);
        return xHash ^ (yHash + 0x9e3779b9U + (xHash << 6) + (xHash >> 2));
    }
};

class GridGraph : public IGraph<Point2D> {
private:
    int width;
    int height;
    std::vector<bool> obstacles;

    bool isValid(int x, int y) const {
        return x >= 0 && x < width && y >= 0 && y < height;
    }

public:
    GridGraph(int width, int height)
        : width(width), height(height), obstacles(width * height, false) {}

    void setObstacle(int x, int y, bool isObstacle) {
        if (isValid(x, y)) {
            obstacles[y * width + x] = isObstacle;
        }
    }

    bool isWalkable(int x, int y) const {
        if (!isValid(x, y)) return false;
        return !obstacles[y * width + x];
    }

    std::vector<Point2D> getNeighbors(const Point2D& node) const override {
        std::vector<Point2D> neighbors;
        neighbors.reserve(4);
        static const Point2D directions[] = {
            {1, 0}, {0, 1}, {-1, 0}, {0, -1}
        };
        for (const auto& dir : directions) {
            Point2D next{node.x + dir.x, node.y + dir.y};
            if (isWalkable(next.x, next.y)) {
                neighbors.push_back(next);
            }
        }
        return neighbors;
    }
};