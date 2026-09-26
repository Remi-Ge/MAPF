CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -pedantic

TEST_SOURCES := $(wildcard tests/test_*.cpp)
TEST_BINARIES := $(patsubst tests/%.cpp,build/%,$(TEST_SOURCES))
PROJECT_SOURCES := $(filter-out src/main.cpp,$(wildcard src/*.cpp))
PROJECT_HEADERS := $(wildcard src/*.hpp)

.PHONY: build demo serve test clean
.DEFAULT_GOAL := build

build: build/mapf_demo

build/mapf_demo: src/main.cpp $(PROJECT_SOURCES) $(PROJECT_HEADERS) | build/
	$(CXX) $(CXXFLAGS) -Isrc src/main.cpp $(PROJECT_SOURCES) -o $@

demo: build/mapf_demo | build/
	./build/mapf_demo > build/result.json
	@echo "CBS result written to build/result.json"

serve: build/mapf_demo
	node server.js

test: $(TEST_BINARIES)
	@if [ -z "$(TEST_BINARIES)" ]; then \
		echo "No tests found in tests/ (expected test_*.cpp)." >&2; \
		exit 1; \
	fi
	@set -e; for test_binary in $(TEST_BINARIES); do \
		echo "Running $$test_binary..."; \
		"$$test_binary"; \
	done

build/test_%: tests/test_%.cpp $(PROJECT_SOURCES) $(PROJECT_HEADERS) | build/
	$(CXX) $(CXXFLAGS) -Isrc $< $(PROJECT_SOURCES) -o $@

build/:
	mkdir -p $@

clean:
	rm -rf build