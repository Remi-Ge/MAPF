CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -pedantic

TEST_SOURCES := $(wildcard tests/test_*.cpp)
TEST_BINARIES := $(patsubst tests/%.cpp,build/%,$(TEST_SOURCES))

.PHONY: build test clean
.DEFAULT_GOAL := build

build: build/main.o

build/main.o: src/main.cpp | build/
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TEST_BINARIES)
	@if [ -z "$(TEST_BINARIES)" ]; then \
		echo "No tests found in tests/ (expected test_*.cpp)." >&2; \
		exit 1; \
	fi
	@set -e; for test_binary in $(TEST_BINARIES); do \
		echo "Running $$test_binary..."; \
		"$$test_binary"; \
	done

build/test_%: tests/test_%.cpp src/graph.cpp | build/
	$(CXX) $(CXXFLAGS) -Isrc $< -o $@

build/:
	mkdir -p $@

clean:
	rm -rf build