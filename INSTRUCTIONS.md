# Boids Simulation - Compilation and Execution Instructions

## Prerequisites
- GCC compiler with OpenMP support
- Raylib library (included in `<repo>/raylib/src`)
- Make utility

# Installing raylib
Ubuntu/WSL2
https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux
```bash
sudo apt install build-essential git
sudo apt install libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev
```

After cloning the github repo, cd into it and run the following commands.
```bash
git clone --depth 1 https://github.com/raysan5/raylib.git raylib
cd raylib/src/
make PLATFORM=PLATFORM_DESKTOP # To make the static version.
```

## Compilation

The project uses a Makefile with multiple targets. All compilation is done from the project root directory.

### Build All Main Executables
```bash
make all
```
This builds:
- `boids_baseline` - Serial version
- `boids_parallel` - Parallel version with OpenMP

### Build (And run?) Individual Components

```bash
make # Build both serial and parallel version
make boids_baseline # Build serial version only
make boids_parallel # Build parallel version only
make test_correctness # Build and run correctness test
make compare # Build and run the timing tests
make validate # Build and run both correctness and timing tests one after the other
```

### Clean Build Artifacts
```bash
make clean
```

## Execution

### Running the Simulations

**Baseline (Serial) Version:**
```bash
./boids_baseline [num_boids]
```

**Parallel Version:**
```bash
./boids_parallel [num_boids]
```

The `[num_boids]` parameter is optional (default: 512).

Example:
```bash
./boids_parallel 1024
```

### Running Tests

**Correctness Test:**
```bash
make test
```
Verifies that the parallel implementation produces identical results to the serial version.

**Performance Comparison:**
```bash
make compare
```
Runs detailed performance metrics comparing serial vs parallel with 4 and 8 threads. Can be modified for any number of threads. Only generates an average to put in the .txt file after running for a 100 frames (Refer to BENCHMARK_FRAMES to modify).

**Full Validation:**
```bash
make validate
```
Runs correctness test followed by performance comparison.

## Expected Output

### Simulation Window
- Opens a 1920x1200 window
- Displays boids as blue triangles
- Runs at 60 FPS target
- Close window with ESC or window close button

## Troubleshooting

**If #includes <raylib.h> not found:**
- Check that raylib is present in `./raylib/src`

**If execution fails:**
- Ensure display is available (X11 for Linux)
- Check OpenMP thread limit: `echo $OMP_NUM_THREADS`

## Help

For a list of all available Make targets:
```bash
make help
```