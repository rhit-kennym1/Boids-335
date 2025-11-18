# Makefile for Boids simulation

CC = gcc

# Compiler flags
CFLAGS = -I./raylib/src -O3 -march=native -fopenmp-simd -ffast-math -funroll-loops
LDFLAGS = -L./raylib/src -lraylib -ldl -pthread -lGL -lm

# Source files
BASELINE_SRCS := $(wildcard src/*baseline*.c) src/config.c
PARALLEL_SRCS := $(wildcard src/*parallel*.c) src/config.c
TEST_SRC := src/test.c src/boids.c src/config.c  # add boids.c & config.c for compilation

# Output executables
BASELINE_BIN := boids_baseline
PARALLEL_BIN := boids_parallel
TEST_BIN := test_correctness

# Default target
all: $(BASELINE_BIN) $(PARALLEL_BIN)

# Baseline build
$(BASELINE_BIN): $(BASELINE_SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Parallel build
$(PARALLEL_BIN): $(PARALLEL_SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Test build
$(TEST_BIN): $(TEST_SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Run test
test: $(TEST_BIN)
	./$(TEST_BIN)

# Clean
clean:
	rm -f $(BASELINE_BIN) $(PARALLEL_BIN) $(TEST_BIN)
