# pragma once

#include <string>

class GridGraph {
public:
    GridGraph(int width, int height);
    std::string toString();
private:
    int width;
    int height;
};