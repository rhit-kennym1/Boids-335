# Makefile for Boids simulation

CC = gcc

# Compiler flags
CFLAGS = -I./raylib/src -O3 -march=native -fopenmp -ffast-math -funroll-loops
LDFLAGS = -L./raylib/src -lraylib -ldl -pthread -lGL -lm -lgomp

# Source files
BASELINE_SRCS := src/boids_baseline.c src/main_baseline.c src/config.c
PARALLEL_SRCS := src/boids_parallel.c src/main_parallel.c src/config.c
METRICS_BASELINE_SRCS := src/main_metrics.c src/boids_baseline.c src/config.c
METRICS_PARALLEL_SRCS := src/main_metrics.c src/boids_parallel.c src/config.c
TEST_SRC := src/test.c src/boids_parallel.c src/config.c

# Output executables
BASELINE_BIN := boids_baseline
PARALLEL_BIN := boids_parallel
METRICS_BASELINE_BIN := metrics_baseline
METRICS_PARALLEL_BIN := metrics_parallel
TEST_BIN := test_correctness

# Default target
all: $(BASELINE_BIN) $(PARALLEL_BIN)

# Baseline build
$(BASELINE_BIN): $(BASELINE_SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Parallel build
$(PARALLEL_BIN): $(PARALLEL_SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Metrics build - baseline (serial, no OpenMP in update loop)
$(METRICS_BASELINE_BIN): $(METRICS_BASELINE_SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Metrics build - parallel (uses updateAllBoids)
$(METRICS_PARALLEL_BIN): $(METRICS_PARALLEL_SRCS)
	$(CC) $(CFLAGS) -DUPDATE_ALL_BOIDS $^ -o $@ $(LDFLAGS)

# Test build
$(TEST_BIN): $(TEST_SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Run test
test: $(TEST_BIN)
	./$(TEST_BIN)

# Run metrics comparison
compare: $(METRICS_BASELINE_BIN) $(METRICS_PARALLEL_BIN)
	@rm -f speedup_data.txt
	@echo "=== Running BASELINE (serial) ==="
	@OMP_NUM_THREADS=1 ./$(METRICS_BASELINE_BIN)
	@echo ""
	@echo "=== Running PARALLEL (4 threads) ==="
	@OMP_NUM_THREADS=4 ./$(METRICS_PARALLEL_BIN)
	@echo ""
	@echo "=== Running PARALLEL (8 threads) ==="
	@OMP_NUM_THREADS=8 ./$(METRICS_PARALLEL_BIN)
	@echo ""
	@bash calculate_speedup.sh
	@rm -f speedup_data.txt

# Clean
clean:
	rm -f $(BASELINE_BIN) $(PARALLEL_BIN) $(METRICS_BASELINE_BIN) $(METRICS_PARALLEL_BIN) $(TEST_BIN)