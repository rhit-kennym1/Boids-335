#include <raylib.h>
#include <rlgl.h>
#include <stdlib.h>
#include <stdio.h>

#include "boids.h"

#define FPS 360
#define TITLE "Boids Example"
#define DEFAULT_BOIDS 1024
#define DEFAULT_VELOCITY 20
#define DEFAULT_ANGULAR_VELOCITY 1

int main(int argc, char** argv) {
    // Initial window size (can be overridden by command-line args)
    int WIDTH = (argc > 1) ? atoi(argv[1]) : 1920;
    int HEIGHT = (argc > 2) ? atoi(argv[2]) : 1200;
    int BOIDS = (argc > 3) ? atoi(argv[3]) : DEFAULT_BOIDS;

    // Make window resizable
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WIDTH, HEIGHT, TITLE);
    rlDisableBackfaceCulling();
    SetTargetFPS(FPS);

    // Allocate boid array dynamically
    Boid** flock = malloc(sizeof(Boid*) * BOIDS);

    // Initial spawn
    for (int i = 0; i < BOIDS; i++) {
        flock[i] = newBoid(
            (Vector2){GetRandomValue(0, GetScreenWidth()), GetRandomValue(0, GetScreenHeight())},
            (Vector2){DEFAULT_VELOCITY, DEFAULT_VELOCITY},
            GetRandomValue(0, 6),
            DEFAULT_ANGULAR_VELOCITY
        );
    }

    while (!WindowShouldClose()) {
        // Update boids
        for (int i = 0; i < BOIDS; i++) {
            updateBoid(flock[i], flock, BOIDS);
        }

        // Drawing
        BeginDrawing();
        ClearBackground(RAYWHITE);

        for (int i = 0; i < BOIDS; i++) {
            drawBoid(flock[i]);
        }

        EndDrawing();

        // Optional: dynamically print current window size (for debugging)
        // printf("Window: %d x %d\n", GetScreenWidth(), GetScreenHeight());
    }

    // Cleanup
    for (int i = 0; i < BOIDS; i++) {
        free(flock[i]->positions);
        free(flock[i]);
    }
    free(flock);

    CloseWindow();
    return 0;
}