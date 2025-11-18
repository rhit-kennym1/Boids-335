#include <raylib.h>
#include <rlgl.h>
#include <stdlib.h>

#include "boids.h"
#include "config.h"

#define TITLE "Boids Example"

int main(int argc, char* argv[]) {
	InitWindow(WIDTH, HEIGHT, TITLE);
	rlDisableBackfaceCulling();
	SetTargetFPS(FPS);
    SetRandomSeed(42690);
    srand(42690);
	
	int BOIDS = NUM_BOIDS;
	if (argc > 1) {
        BOIDS = atoi(argv[1]);
    }

	Boid* flock[BOIDS];

	for (int i = 0; i < BOIDS; i++)
		flock[i] = newBoid((Vector2){GetRandomValue(0, WIDTH), GetRandomValue(0, HEIGHT)}, (Vector2){20, 20}, GetRandomValue(0, 6), 1);

	while (!WindowShouldClose()){
		updateAllBoids(flock, BOIDS);

		BeginDrawing();
		ClearBackground(RAYWHITE);

		for (int i = 0; i < BOIDS; i++)
			drawBoid(flock[i]);

		EndDrawing();
	}

	for (int i = 0; i < BOIDS; i++) {
		free(flock[i]->positions);
		free(flock[i]);
	}

	CloseWindow();

	return 0;
}