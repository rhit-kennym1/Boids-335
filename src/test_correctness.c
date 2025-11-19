#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <raylib.h>
#include <string.h>
#include <omp.h>
#include "boids.h"

#define TEST_FRAMES 100
#define TEST_BOIDS 50
#define WIDTH 1920
#define HEIGHT 1200
#define EPSILON 1e-1  

extern void updateBoid(Boid* boid, Boid** flock, int flockSize);
extern void updateAllBoids(Boid** flock, int flockSize);

// stores state of a boid for comparison
typedef struct {
    Vector2 origin;
    float rotation;
    Vector2 velocity;
    float angularVelocity;
    Vector2 positions[3];
} BoidState;

// copy boid state
void copyBoidState(Boid* src, BoidState* dst) {
    dst->origin = src->origin;
    dst->rotation = src->rotation;
    dst->velocity = src->velocity;
    dst->angularVelocity = src->angularVelocity;
    for (int i = 0; i < 3; i++) {
        dst->positions[i] = src->positions[i];
    }
}

// compare floats with epsilon tolerance
int floatsEqual(float a, float b, float epsilon) {
    return fabsf(a - b) < epsilon;
}

// compare vectors
int vectorsEqual(Vector2 a, Vector2 b, float epsilon) {
    return floatsEqual(a.x, b.x, epsilon) && floatsEqual(a.y, b.y, epsilon);
}

// compare boid states, gives verbose output optionally
int statesEqual(BoidState* a, BoidState* b, float epsilon, int verbose) {
    int match = 1;
    if (!vectorsEqual(a->origin, b->origin, epsilon)) {
        if (verbose) printf("Origin: (%.6f, %.6f) vs (%.6f, %.6f)\n", a->origin.x, a->origin.y, b->origin.x, b->origin.y);
        match = 0;
    }
    if (!floatsEqual(a->rotation, b->rotation, epsilon)) {
        if (verbose) printf("Rotation: %.6f vs %.6f\n", a->rotation, b->rotation);
        match = 0;
    }
    if (!vectorsEqual(a->velocity, b->velocity, epsilon)) {
        if (verbose) printf("Velocity: (%.6f, %.6f) vs (%.6f, %.6f)\n", a->velocity.x, a->velocity.y, b->velocity.x, b->velocity.y);
        match = 0;
    }
    if (!floatsEqual(a->angularVelocity, b->angularVelocity, epsilon)) {
        if (verbose) printf("Angular velocity: %.6f vs %.6f\n", a->angularVelocity, b->angularVelocity);
        match = 0;
    }
    for (int i = 0; i < 3; i++) {
        if (!vectorsEqual(a->positions[i], b->positions[i], epsilon)) {
            if (verbose) printf("Position[%d]: (%.6f, %.6f) vs (%.6f, %.6f)\n", i, a->positions[i].x, a->positions[i].y, b->positions[i].x, b->positions[i].y);
            match = 0;
        }
    }
    return match;
}

int main(void) {
    printf("BOIDS CORRECTNESS TEST\n\n");

    InitWindow(WIDTH, HEIGHT, "Correctness Test"); // open window
    SetTargetFPS(60);

    // set random seed so both flocks start same
    SetRandomSeed(42690);
    srand(42690);

    Vector2 initialPositions[TEST_BOIDS];
    float initialRotations[TEST_BOIDS];
    for (int i = 0; i < TEST_BOIDS; i++) {
        initialPositions[i] = (Vector2){GetRandomValue(0, WIDTH), GetRandomValue(0, HEIGHT)};
        initialRotations[i] = GetRandomValue(0, 6);
    }

    // create serial flock
    printf("Creating serial flock with %d boids\n", TEST_BOIDS);
    Boid* baselineFlock[TEST_BOIDS];
    double creationTime = GetTime();
    for (int i = 0; i < TEST_BOIDS; i++) {
        baselineFlock[i] = newBoid(initialPositions[i], (Vector2){20, 20}, initialRotations[i], 1);
        baselineFlock[i]->lastUpdate = creationTime;
    }

    // create parallel flock
    printf("Creating parallel flock\n");
    Boid* parallelFlock[TEST_BOIDS];
    for (int i = 0; i < TEST_BOIDS; i++) {
        parallelFlock[i] = newBoid(initialPositions[i], (Vector2){20, 20}, initialRotations[i], 1);
        parallelFlock[i]->lastUpdate = creationTime;
    }

    BoidState baselineStates[TEST_BOIDS];
    BoidState parallelStates[TEST_BOIDS];

    int totalTests = 0;
    int passedTests = 0;
    int failedFrames = 0;
    int firstFailureFrame = -1;
    int maxErrorsToShow = 3;

    printf("Running %d frames\n", TEST_FRAMES);

    for (int frame = 0; frame < TEST_FRAMES; frame++) {
        // serial update with 1 thread
        int oldThreads = omp_get_max_threads();
        omp_set_num_threads(1);
        updateAllBoids(baselineFlock, TEST_BOIDS);
        omp_set_num_threads(oldThreads);

        // parallel update
        updateAllBoids(parallelFlock, TEST_BOIDS);

        // copy states for comparison
        for (int i = 0; i < TEST_BOIDS; i++) {
            copyBoidState(baselineFlock[i], &baselineStates[i]);
            copyBoidState(parallelFlock[i], &parallelStates[i]);
        }

        int frameMatches = 1;
        int errorsShown = 0;
        for (int i = 0; i < TEST_BOIDS; i++) {
            totalTests++;
            int verbose = errorsShown < maxErrorsToShow;
            if (statesEqual(&baselineStates[i], &parallelStates[i], EPSILON, verbose)) {
                passedTests++;
            } else {
                if (frameMatches) {
                    printf("Frame %d mismatch\n", frame); // first error for this frame
                    frameMatches = 0;
                    failedFrames++;
                    if (firstFailureFrame == -1) firstFailureFrame = frame;
                }
                if (verbose) {
                    printf("Boid %d\n", i);
                    errorsShown++;
                } else if (errorsShown == maxErrorsToShow) {
                    printf("more errors not shown\n");
                    errorsShown++;
                }
            }
        }

        if (frameMatches && (frame % 10 == 0 || frame == TEST_FRAMES - 1)) {
            printf("Frame %d OK\n", frame); // some progress feedback
        }

        WaitTime(0.001); // tiny wait so window updates
    }

    // print results
    printf("\nRESULTS\n");
    printf("Total frames: %d\n", TEST_FRAMES);
    printf("Boids per frame: %d\n", TEST_BOIDS);
    printf("Total tests: %d\n", totalTests);
    printf("Passed: %d (%.2f%%)\n", passedTests, (passedTests * 100.0) / totalTests);
    printf("Failed: %d (%.2f%%)\n", totalTests - passedTests, ((totalTests - passedTests) * 100.0) / totalTests);
    printf("Frames with error: %d\n", failedFrames);
    if (firstFailureFrame >= 0) printf("First failure frame %d\n", firstFailureFrame);

    if (passedTests == totalTests) {
        printf("SUCCESS! parallel implementation matches serial\n");
    } else {
        printf("FAILURE! differences detected starting frame %d\n", firstFailureFrame);
    }

    // cleanup
    for (int i = 0; i < TEST_BOIDS; i++) {
        free(baselineFlock[i]->positions);
        free(baselineFlock[i]);
        free(parallelFlock[i]->positions);
        free(parallelFlock[i]);
    }

    CloseWindow(); // close window
    return (passedTests == totalTests) ? 0 : 1;
}
