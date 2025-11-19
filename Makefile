# Makefile for Boids simulation

CC = gcc

# Compiler flags
CFLAGS = -I./raylib/src -fopenmp -O3
LDFLAGS = -L./raylib/src -lraylib -ldl -pthread -lGL -lm -lgomp

# Source files
BASELINE_SRCS = src/boids_baseline.c src/main_baseline.c src/config.c
PARALLEL_SRCS = src/boids_parallel.c src/main_parallel.c src/config.c
METRICS_BASELINE_SRCS = src/main_metrics.c src/boids_baseline.c src/config.c
METRICS_PARALLEL_SRCS = src/main_metrics.c src/boids_parallel.c src/config.c
TEST_SRCS = src/test_correctness.c src/boids_parallel.c src/config.c

# Output executables
BASELINE_BIN = boids_baseline
PARALLEL_BIN = boids_parallel
METRICS_BASELINE_BIN = metrics_baseline
METRICS_PARALLEL_BIN = metrics_parallel
TEST_BIN = test_correctness

# default
all: $(BASELINE_BIN) $(PARALLEL_BIN)

# build baseline
$(BASELINE_BIN): $(BASELINE_SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# build parallel
$(PARALLEL_BIN): $(PARALLEL_SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# metrics baseline
$(METRICS_BASELINE_BIN): $(METRICS_BASELINE_SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# metrics parallel
$(METRICS_PARALLEL_BIN): $(METRICS_PARALLEL_SRCS)
	$(CC) $(CFLAGS) -DUPDATE_ALL_BOIDS $^ -o $@ $(LDFLAGS)

# correctness test
$(TEST_BIN): $(TEST_SRCS)
	$(CC) $(CFLAGS) -DUPDATE_ALL_BOIDS $^ -o $@ $(LDFLAGS)

# run correctness test
test: $(TEST_BIN)
	@echo running correctness test
	@./$(TEST_BIN)
	@echo

# run metrics
compare: $(METRICS_BASELINE_BIN) $(METRICS_PARALLEL_BIN)
	@rm -f speedup_data.txt
	@echo running baseline serial
	@OMP_NUM_THREADS=1 ./$(METRICS_BASELINE_BIN)
	@echo running parallel 4 threads
	@OMP_NUM_THREADS=4 ./$(METRICS_PARALLEL_BIN)
	@echo running parallel 8 threads
	@OMP_NUM_THREADS=8 ./$(METRICS_PARALLEL_BIN)
	@bash calculate_speedup.sh
	@rm -f speedup_data.txt

# full validation
validate: test compare
	@echo validation done

# clean
clean:
	rm -f $(BASELINE_BIN) $(PARALLEL_BIN) $(METRICS_BASELINE_BIN) $(METRICS_PARALLEL_BIN) $(TEST_BIN)

# help
help:
	@echo available targets
	@echo "  make all - build baseline and parallel"
	@echo "  make test - run correctness test"
	@echo "  make compare - run performance metrics"
	@echo "  make validate - test then compare"
	@echo "  make clean - remove all binaries"
	@echo "  make $(BASELINE_BIN) - build baseline"
	@echo "  make $(PARALLEL_BIN) - build parallel"
	@echo "  make $(METRICS_BASELINE_BIN) - build metrics baseline"
	@echo "  make $(METRICS_PARALLEL_BIN) - build metrics parallel"
	@echo "  make $(TEST_BIN) - build correctness test"

.PHONY: all test compare validate clean help
