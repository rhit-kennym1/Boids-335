CC = gcc
# Define M_PI for sources that rely on it
CFLAGS = -I. -I../src -Wall -Wextra -std=c11 -pedantic -DM_PI=3.14159265358979323846

UNIT_SRCS = ../src/boids_baseline.c test_runner.c
DRIVER = driver.c

ALL = run_tests sim_baseline sim_parallel

.PHONY: all clean test compare

all: $(ALL)

run_tests: $(UNIT_SRCS)
	$(CC) $(CFLAGS) $(UNIT_SRCS) -lm -o $@

sim_baseline: $(DRIVER) ../src/boids_baseline.c
	$(CC) $(CFLAGS) $(DRIVER) ../src/boids_baseline.c -lm -o $@

sim_parallel: $(DRIVER) ../src/boids_parallel.c
	$(CC) $(CFLAGS) $(DRIVER) ../src/boids_parallel.c -lm -o $@

test: run_tests
	./run_tests

compare: sim_baseline sim_parallel
	./sim_baseline > baseline.out
	./sim_parallel > parallel.out
	printf "Comparing outputs...\n"
	@if diff -u baseline.out parallel.out > diff.out; then \
		echo "Outputs identical (parallel implementation matches baseline)."; \
		rm -f diff.out; \
		exit 0; \
	else \
		echo "Differences found between baseline and parallel (see diff.out)."; \
		exit 1; \
	fi

clean:
	rm -f run_tests sim_baseline sim_parallel baseline.out parallel.out diff.out
