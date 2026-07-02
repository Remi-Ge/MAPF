CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude

SRC = src/main.cpp src/GridGraph.cpp src/AStar.cpp
OBJ = $(SRC:.cpp=.o)

APP = build

all: $(APP)

$(APP): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(APP)

clean:
	rm -f $(APP)