#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include "boids.h"
#include "config.h"
#include <raylib.h> // for SetRandomSeed

#define NUM_STEPS 10      // number of simulation steps to test
#define TOL 1e-3          // tolerance for float comparison
#define TEST_BOIDS 16     // small number of boids for correctness testing

// Compare two floats within tolerance
int floatEqual(float a, float b, float tol) {
    return fabsf(a - b) < tol;
}

// Compare two boids
int boidEqual(Boid* b1, Boid* b2, float tol) {
    if (!floatEqual(b1->origin.x, b2->origin.x, tol)) return 0;
    if (!floatEqual(b1->origin.y, b2->origin.y, tol)) return 0;
    if (!floatEqual(b1->rotation, b2->rotation, tol)) return 0;
    return 1;
}

// Copy flock (deep copy positions and metadata)
void copyFlock(Boid** src, Boid** dest, int n) {
    for (int i = 0; i < n; i++) {
        dest[i] = malloc(sizeof(Boid));
        dest[i]->origin = src[i]->origin;
        dest[i]->velocity = src[i]->velocity;
        dest[i]->rotation = src[i]->rotation;
        dest[i]->angularVelocity = src[i]->angularVelocity;
        dest[i]->lastUpdate = src[i]->lastUpdate;

        // copy positions array
        dest[i]->positions = malloc(sizeof(Vector2)*3);
        for (int j = 0; j < 3; j++)
            dest[i]->positions[j] = src[i]->positions[j];
    }
}

// Free a flock
void freeFlock(Boid** flock, int n) {
    for (int i = 0; i < n; i++) {
        free(flock[i]->positions);
        free(flock[i]);
    }
}

// Main correctness test
int main() {
    int BOIDS = TEST_BOIDS;

    // Seed RNGs for deterministic behavior
    SetRandomSeed(12345);
    srand(12345);

    // Initialize serial flock
    Boid** serialFlock = malloc(sizeof(Boid*) * BOIDS);
    for (int i = 0; i < BOIDS; i++)
        serialFlock[i] = newBoid(
            (Vector2){GetRandomValue(0, WIDTH), GetRandomValue(0, HEIGHT)},
            (Vector2){20, 20},
            GetRandomValue(0, 6),
            1
        );

    // Copy flock for parallel test
    Boid** parallelFlock = malloc(sizeof(Boid*) * BOIDS);
    copyFlock(serialFlock, parallelFlock, BOIDS);

    // Run NUM_STEPS serially
    for (int step = 0; step < NUM_STEPS; step++) {
        for (int i = 0; i < BOIDS; i++)
            updateBoid(serialFlock[i], serialFlock, BOIDS);
    }

    // Run NUM_STEPS in parallel
    for (int step = 0; step < NUM_STEPS; step++) {
        #pragma omp parallel for
        for (int i = 0; i < BOIDS; i++)
            updateBoid(parallelFlock[i], parallelFlock, BOIDS);
    }

    // Compare flocks
    int success = 1;
    for (int i = 0; i < BOIDS; i++) {
        if (!boidEqual(serialFlock[i], parallelFlock[i], TOL)) {
            printf("Boid %d mismatch!\n", i);
            success = 0;
        }
    }

    if (success)
        printf("✅ Correctness test passed: parallel matches serial!\n");
    else
        printf("❌ Correctness test FAILED!\n");

    // Cleanup
    freeFlock(serialFlock, BOIDS);
    freeFlock(parallelFlock, BOIDS);

    return success ? 0 : 1;
}
