#include <raylib.h>
#include <rlgl.h>
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include "boids.h"

#define FPS 60
#define WIDTH 1920
#define HEIGHT 1200
#define TITLE "Boids Performance Metrics"
#define BOIDS 5000
#define BENCHMARK_FRAMES 100

typedef struct {
    Vector2 v0, v1, v2;
} Triangle;

int main(void) {

    // Print thread info
    #ifdef _OPENMP
        printf("OpenMP enabled (threads: %d)\n", omp_get_max_threads());
    #else
        printf("OpenMP not enabled\n");
    #endif

    InitWindow(WIDTH, HEIGHT, TITLE);
    rlDisableBackfaceCulling();
    SetTargetFPS(FPS);

    Boid* flock[BOIDS];
    Triangle triangles[BOIDS];

    // Init boids
    for (int i = 0; i < BOIDS; i++) {
        flock[i] = newBoid(
            (Vector2){GetRandomValue(0, WIDTH), GetRandomValue(0, HEIGHT) },
            (Vector2){20, 20 },
            GetRandomValue(0, 6), 1);
    }

    int frameCount = 0;
    double totalUpdate = 0.0;
    double totalCompute = 0.0;
    double totalRender = 0.0;

    double start = GetTime();
    double lastUpdate = GetTime();

    double avgFPS = 0.0;
    double avgUpdate = 0.0;
    double avgCompute = 0.0;
    double avgRender = 0.0;

    while (!WindowShouldClose()) {

        double frameStart = GetTime();

        // UPDATE
        double t0 = GetTime();

        #ifdef UPDATE_ALL_BOIDS
            updateAllBoids(flock, BOIDS);
        #else
            for (int i = 0; i < BOIDS; i++) {
                updateBoid(flock[i], flock, BOIDS);
            }
        #endif

        double t1 = GetTime();
        totalUpdate += (t1 - t0);

        // COMPUTE
        double t2 = GetTime();

        #pragma omp parallel for schedule(static)
            for (int i = 0; i < BOIDS; i++) {
                Boid* b = flock[i];
                triangles[i].v0 = (Vector2){b->positions[0].x + b->origin.x,
                                             b->positions[0].y + b->origin.y};
                triangles[i].v1 = (Vector2){b->positions[1].x + b->origin.x,
                                             b->positions[1].y + b->origin.y};
                triangles[i].v2 = (Vector2){b->positions[2].x + b->origin.x,
                                             b->positions[2].y + b->origin.y};
            }

        double t3 = GetTime();
        totalCompute += (t3 - t2);

        // RENDER
        double t4 = GetTime();

        BeginDrawing();
        ClearBackground(RAYWHITE);

        for (int i = 0; i < BOIDS; i++) {
            DrawTriangle(triangles[i].v0, triangles[i].v1, triangles[i].v2, BLUE);
        }

        DrawText(TextFormat("FPS: %.1f", avgFPS), 10, 10, 20, RED);
        DrawText(TextFormat("Update: %.3f ms", avgUpdate), 10, 35, 20, RED);
        DrawText(TextFormat("Compute: %.3f ms",avgCompute),10, 60, 20, RED);
        DrawText(TextFormat("Render: %.3f ms", avgRender), 10, 85, 20, RED);
        DrawText(TextFormat("Boids: %d", BOIDS), 10, 110, 20, RED);

        #ifdef _OPENMP
            DrawText(TextFormat("Threads: %d", omp_get_max_threads()), 10, 135, 20, RED);
        #endif

        EndDrawing();

        double t5 = GetTime();
        totalRender += (t5 - t4);

        frameCount++;

        // Update displayed metrics every second
        double now = GetTime();
        if (now - lastUpdate >= 1.0) {
            double elapsed = now - start;
            avgFPS = frameCount / elapsed;
            avgUpdate = (totalUpdate  / frameCount) * 1000.0;
            avgCompute = (totalCompute / frameCount) * 1000.0;
            avgRender = (totalRender  / frameCount) * 1000.0;
            lastUpdate = now;
        }

        // Benchmark completed
        if (frameCount == BENCHMARK_FRAMES) {

            double total = GetTime() - start;

            #ifdef _OPENMP
                printf("Threads: %d\n", omp_get_max_threads());
            #else
                printf("Threads: 1\n");
            #endif

            printf("Time: %.2f s\n", total);
            printf("FPS: %.2f\n", frameCount / total);
            printf("Avg frame: %.3f ms\n", (total / frameCount) * 1000.0);
            printf("Update: %.3f ms\n", avgUpdate);
            printf("Compute: %.3f ms\n", avgCompute);
            printf("Render: %.3f ms\n", avgRender);

            FILE* fp = fopen("speedup_data.txt", "a");
            if (fp) {
                #ifdef _OPENMP
                    fprintf(fp, "%d %.6f %.6f %.6f\n",
                            omp_get_max_threads(), avgUpdate, avgCompute, avgRender);
                #else
                    fprintf(fp, "1 %.6f %.6f %.6f\n",
                            avgUpdate, avgCompute, avgRender);
                #endif
                fclose(fp);
            }
        }
    }

    // Cleanup
    for (int i = 0; i < BOIDS; i++) {
        free(flock[i]->positions);
        free(flock[i]);
    }

    CloseWindow();
    return 0;
}