#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"
#include "../src/boids.h"

/* Provide a controllable GetTime() used by boids implementation */
static double current_time = 0.0;

double GetTime(void) { return current_time; }

int main(int argc, char** argv) {
    const int BOIDS = 128;
    const int STEPS = 50;
    const double dt = 1.0 / 60.0;

    Boid* flock[BOIDS];

    /* deterministic initial positions/rotations */
    for (int i = 0; i < BOIDS; i++) {
        float x = (float)((i * 31) % 1920);
        float y = (float)((i * 17) % 1200);
        float rot = (float)((i * 7) % 314) / 100.0f; /* some deterministic rotation */
        flock[i] = newBoid((Vector2){x, y}, (Vector2){20.0f, 20.0f}, rot, 1.0f);
    }

    for (int step = 0; step < STEPS; step++) {
        current_time += dt;
        for (int i = 0; i < BOIDS; i++) {
            updateBoid(flock[i], flock, BOIDS);
        }
    }

    /* Print final state for comparison */
    for (int i = 0; i < BOIDS; i++) {
        printf("%d %.6f %.6f %.6f\n", i, flock[i]->origin.x, flock[i]->origin.y, flock[i]->rotation);
    }

    for (int i = 0; i < BOIDS; i++) {
        free(flock[i]->positions);
        free(flock[i]);
    }

    return 0;
}
