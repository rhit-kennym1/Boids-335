Testing framework for Boids-335

This tests directory contains a small unit test harness for core boid logic.

Files:
- `raylib.h` - minimal stub of raylib symbols used by `boids_baseline.c` so unit tests can compile without the full raylib dependency.
- `test_runner.c` - simple test runner with a few unit tests (distance, getRotation, rotateBoid, newBoid).
- `Makefile` - builds and runs the tests.

Usage:

From the project root:

```bash
cd tests
make
make test
```

Notes:
- The tests compile `src/boids_baseline.c` alongside the runner, so they exercise the same implementation used by the baseline program.
- The `raylib.h` here intentionally contains only the minimal stubs required for compilation; it is not the full raylib.
- Add more tests to `tests/test_runner.c` to expand coverage.
