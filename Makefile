# Makefile for Boids simulation

CC = gcc

# Compiler flags
CFLAGS = -I./raylib/src -O3 -march=native -fopenmp-simd -ffast-math -funroll-loops
LDFLAGS = -L./raylib/src -lraylib -ldl -pthread -lGL -lm

# Source files (automatic detection)
BASELINE_SRCS := $(wildcard src/*baseline*.c)
PARALLEL_SRCS := $(wildcard src/*parallel*.c)

# Output executables
BASELINE_BIN := boids_baseline
PARALLEL_BIN := boids_parallel

# Default target: build both
all: $(BASELINE_BIN) $(PARALLEL_BIN)

# Baseline build
$(BASELINE_BIN): $(BASELINE_SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Parallel build
$(PARALLEL_BIN): $(PARALLEL_SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Clean
clean:
	rm -f $(BASELINE_BIN) $(PARALLEL_BIN)