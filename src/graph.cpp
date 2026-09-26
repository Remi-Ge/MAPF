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
        // the list will contain at most 4 neighbors (up, down, left, right)
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