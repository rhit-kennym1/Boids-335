#include <raylib.h>
#include <rlgl.h>
#include <stdlib.h>

#include "boids.h"

#define FPS 60
#define WIDTH 1920
#define HEIGHT 1200
#define TITLE "Boids Example"
#define BOIDS 128

int main(void) {
	InitWindow(WIDTH, HEIGHT, TITLE);
	rlDisableBackfaceCulling();
	SetTargetFPS(FPS);

	Boid* flock[BOIDS];

	for (int i = 0; i < BOIDS; i++)
		flock[i] = newBoid((Vector2){GetRandomValue(0, WIDTH), GetRandomValue(0, HEIGHT)}, (Vector2){20, 20}, GetRandomValue(0, 6), 1);

	while (!WindowShouldClose()){
		for (int i = 0; i < BOIDS; i++)
			updateBoid(flock[i], flock, BOIDS);

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