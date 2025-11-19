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

typedef struct {
    Vector2 origin;
    float rotation;
    Vector2 velocity;
    float angularVelocity;
    Vector2 positions[3];
} BoidState;

void copyBoidState(Boid* src, BoidState* dst) {
    dst->origin = src->origin;
    dst->rotation = src->rotation;
    dst->velocity = src->velocity;
    dst->angularVelocity = src->angularVelocity;
    for (int i = 0; i < 3; i++) {
        dst->positions[i] = src->positions[i];
    }
}

int floatsEqual(float a, float b, float epsilon) {
    return fabsf(a - b) < epsilon;
}

int vectorsEqual(Vector2 a, Vector2 b, float epsilon) {
    return floatsEqual(a.x, b.x, epsilon) && floatsEqual(a.y, b.y, epsilon);
}

int statesEqual(BoidState* a, BoidState* b, float epsilon, int verbose) {
    int match = 1;
    
    if (!vectorsEqual(a->origin, b->origin, epsilon)) {
        if (verbose) {
            printf("    Origin: (%.6f, %.6f) vs (%.6f, %.6f), diff: (%.6f, %.6f)\n",
                   a->origin.x, a->origin.y, b->origin.x, b->origin.y,
                   fabsf(a->origin.x - b->origin.x), fabsf(a->origin.y - b->origin.y));
        }
        match = 0;
    }
    if (!floatsEqual(a->rotation, b->rotation, epsilon)) {
        if (verbose) {
            printf("    Rotation: %.6f vs %.6f, diff: %.6f\n", 
                   a->rotation, b->rotation, fabsf(a->rotation - b->rotation));
        }
        match = 0;
    }
    if (!vectorsEqual(a->velocity, b->velocity, epsilon)) {
        if (verbose) {
            printf("    Velocity: (%.6f, %.6f) vs (%.6f, %.6f)\n",
                   a->velocity.x, a->velocity.y, b->velocity.x, b->velocity.y);
        }
        match = 0;
    }
    if (!floatsEqual(a->angularVelocity, b->angularVelocity, epsilon)) {
        if (verbose) {
            printf("    Angular velocity: %.6f vs %.6f\n",
                   a->angularVelocity, b->angularVelocity);
        }
        match = 0;
    }
    
    for (int i = 0; i < 3; i++) {
        if (!vectorsEqual(a->positions[i], b->positions[i], epsilon)) {
            if (verbose) {
                printf("    Position[%d]: (%.6f, %.6f) vs (%.6f, %.6f)\n",
                       i, a->positions[i].x, a->positions[i].y, 
                       b->positions[i].x, b->positions[i].y);
            }
            match = 0;
        }
    }
    
    return match;
}

int main(void) {
    printf("BOIDS CORRECTNESS TEST\n\n");
    
    InitWindow(WIDTH, HEIGHT, "Correctness Test");
    SetTargetFPS(60);
    
    // Generate same random values for both flocks
    SetRandomSeed(42690);
    srand(42690);
    Vector2 initialPositions[TEST_BOIDS];
    float initialRotations[TEST_BOIDS];
    
    for (int i = 0; i < TEST_BOIDS; i++) {
        initialPositions[i] = (Vector2){GetRandomValue(0, WIDTH), GetRandomValue(0, HEIGHT)};
        initialRotations[i] = GetRandomValue(0, 6);
    }
    
    // Create serial flock
    printf("Creating serial flock with %d boids\n", TEST_BOIDS);
    Boid* baselineFlock[TEST_BOIDS];
    double creationTime = GetTime();
    for (int i = 0; i < TEST_BOIDS; i++) {
        baselineFlock[i] = newBoid(initialPositions[i], (Vector2){20, 20}, initialRotations[i], 1);
        baselineFlock[i]->lastUpdate = creationTime;
    }
    
    // Create parallel flock with same initial conditions
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
    
    printf("\nRunning %d frames\n", TEST_FRAMES);
    printf("========================================\n");
    
    for (int frame = 0; frame < TEST_FRAMES; frame++) {
        // Run serial version with 1 thread
        int old_threads = omp_get_max_threads();
        omp_set_num_threads(1);
        updateAllBoids(baselineFlock, TEST_BOIDS);
        omp_set_num_threads(old_threads);
        
        // Run parallel version with all threads
        updateAllBoids(parallelFlock, TEST_BOIDS);
        
        // Save states
        for (int i = 0; i < TEST_BOIDS; i++) {
            copyBoidState(baselineFlock[i], &baselineStates[i]);
            copyBoidState(parallelFlock[i], &parallelStates[i]);
        }
        
        // Compare
        int frameMatches = 1;
        int errorsShown = 0;
        for (int i = 0; i < TEST_BOIDS; i++) {
            totalTests++;
            int verbose = (errorsShown < maxErrorsToShow);
            if (statesEqual(&baselineStates[i], &parallelStates[i], EPSILON, verbose)) {
                passedTests++;
            } else {
                if (frameMatches) {
                    printf("Frame %d: MISMATCH\n", frame);
                    frameMatches = 0;
                    failedFrames++;
                    if (firstFailureFrame == -1) {
                        firstFailureFrame = frame;
                    }
                }
                if (verbose) {
                    printf("  Boid %d:\n", i);
                    errorsShown++;
                } else if (errorsShown == maxErrorsToShow) {
                    printf("  ... (more errors not shown)\n");
                    errorsShown++;
                }
            }
        }
        
        if (frameMatches && (frame % 10 == 0 || frame == TEST_FRAMES - 1)) {
            printf("Frame %d: OK\n", frame);
        }
        
        WaitTime(0.001);
    }
    
    printf("========================================\n\n");
    printf("RESULTS\n");
    printf("Total frames: %d\n", TEST_FRAMES);
    printf("Boids per frame: %d\n", TEST_BOIDS);
    printf("Total tests: %d\n", totalTests);
    printf("Passed: %d (%.2f%%)\n", passedTests, (passedTests * 100.0) / totalTests);
    printf("Failed: %d (%.2f%%)\n", totalTests - passedTests, ((totalTests - passedTests) * 100.0) / totalTests);
    printf("Frames with error: %d\n", failedFrames);
    if (firstFailureFrame >= 0) {
        printf("First failure: Frame %d\n", firstFailureFrame);
    }
    
    if (passedTests == totalTests) {
        printf("\nSUCCESS: Parallel implementation matches serial\n");
    } else {
        printf("\nFAILURE: Differences detected\n");
        if (firstFailureFrame == 0) {
            printf("Note: Errors from frame 0 suggest initialization problem\n");
        } else {
            printf("Note: Errors starting frame %d suggest race condition\n", firstFailureFrame);
        }
    }
    
    for (int i = 0; i < TEST_BOIDS; i++) {
        free(baselineFlock[i]->positions);
        free(baselineFlock[i]);
        free(parallelFlock[i]->positions);
        free(parallelFlock[i]);
    }
    
    CloseWindow();
    
    return (passedTests == totalTests) ? 0 : 1;
}