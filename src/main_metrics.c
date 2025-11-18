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
#define BENCHMARK_FRAMES 600

typedef struct {
	Vector2 v0, v1, v2;
} Triangle;

int main(void) {
	#ifdef _OPENMP
	printf("OpenMP enabled - Threads: %d\n", omp_get_max_threads());
	#else
	printf("WARNING: OpenMP NOT enabled!\n");
	#endif
	
	InitWindow(WIDTH, HEIGHT, TITLE);
	rlDisableBackfaceCulling();
	SetTargetFPS(FPS);
	
	Boid* flock[BOIDS];
	Triangle triangles[BOIDS];
	
	// Initialize boids
	for (int i = 0; i < BOIDS; i++)
		flock[i] = newBoid((Vector2){GetRandomValue(0, WIDTH), GetRandomValue(0, HEIGHT)}, 
		                   (Vector2){20, 20}, GetRandomValue(0, 6), 1);
	
	// Performance tracking variables
	int frameCount = 0;
	double totalUpdateTime = 0.0;
	double totalComputeTime = 0.0;
	double totalRenderTime = 0.0;
	double benchmarkStart = GetTime();
	
	// For displaying metrics on screen
	double lastPrintTime = GetTime();
	double avgFPS = 0.0;
	double avgUpdateMs = 0.0;
	double avgComputeMs = 0.0;
	double avgRenderMs = 0.0;
	
	while (!WindowShouldClose()) {
		double frameStart = GetTime();
		
		// === UPDATE PHASE ===
		double updateStart = GetTime();
		
		#ifdef UPDATE_ALL_BOIDS
		updateAllBoids(flock, BOIDS);
		#else
		for (int i = 0; i < BOIDS; i++)
			updateBoid(flock[i], flock, BOIDS);
		#endif
		
		double updateEnd = GetTime();
		totalUpdateTime += (updateEnd - updateStart);
		
		// === COMPUTE PHASE (prepare triangles) - PARALLELIZED ===
		double computeStart = GetTime();
		
		#pragma omp parallel for schedule(static)
		for (int i = 0; i < BOIDS; i++) {
			Boid* boid = flock[i];
			triangles[i].v0 = (Vector2){boid->positions[0].x + boid->origin.x, 
			                            boid->positions[0].y + boid->origin.y};
			triangles[i].v1 = (Vector2){boid->positions[1].x + boid->origin.x, 
			                            boid->positions[1].y + boid->origin.y};
			triangles[i].v2 = (Vector2){boid->positions[2].x + boid->origin.x, 
			                            boid->positions[2].y + boid->origin.y};
		}
		
		double computeEnd = GetTime();
		totalComputeTime += (computeEnd - computeStart);
		
		// === RENDER PHASE (must be serial for OpenGL) ===
		double renderStart = GetTime();
		
		BeginDrawing();
		ClearBackground(RAYWHITE);
		
		// Serial drawing - OpenGL requires this
		for (int i = 0; i < BOIDS; i++)
			DrawTriangle(triangles[i].v0, triangles[i].v1, triangles[i].v2, BLUE);
		
		// Display metrics on screen
		DrawText(TextFormat("FPS: %.1f", avgFPS), 10, 10, 20, RED);
		DrawText(TextFormat("Update: %.3f ms", avgUpdateMs), 10, 35, 20, RED);
		DrawText(TextFormat("Compute: %.3f ms", avgComputeMs), 10, 60, 20, RED);
		DrawText(TextFormat("Render: %.3f ms", avgRenderMs), 10, 85, 20, RED);
		DrawText(TextFormat("Boids: %d", BOIDS), 10, 110, 20, RED);
		#ifdef _OPENMP
		DrawText(TextFormat("Threads: %d", omp_get_max_threads()), 10, 135, 20, RED);
		#endif
		
		EndDrawing();
		
		double renderEnd = GetTime();
		totalRenderTime += (renderEnd - renderStart);
		
		frameCount++;
		
		// Update displayed metrics every second
		double now = GetTime();
		if (now - lastPrintTime >= 1.0) {
			double elapsed = now - benchmarkStart;
			avgFPS = frameCount / elapsed;
			avgUpdateMs = (totalUpdateTime / frameCount) * 1000.0;
			avgComputeMs = (totalComputeTime / frameCount) * 1000.0;
			avgRenderMs = (totalRenderTime / frameCount) * 1000.0;
			lastPrintTime = now;
		}
		
		// Print final statistics after benchmark period
		if (frameCount == BENCHMARK_FRAMES) {
			double totalTime = GetTime() - benchmarkStart;
			
			// Recalculate final averages
			avgUpdateMs = (totalUpdateTime / frameCount) * 1000.0;
			avgComputeMs = (totalComputeTime / frameCount) * 1000.0;
			avgRenderMs = (totalRenderTime / frameCount) * 1000.0;
			double avgFrameMs = (totalTime / frameCount) * 1000.0;
			
			printf("\n=== BENCHMARK RESULTS (%d frames) ===\n", BENCHMARK_FRAMES);
			#ifdef _OPENMP
			printf("Threads:           %d\n", omp_get_max_threads());
			#else
			printf("Threads:           1 (serial)\n");
			#endif
			printf("Total time:        %.2f seconds\n", totalTime);
			printf("Average FPS:       %.2f\n", frameCount / totalTime);
			printf("Average frame:     %.3f ms\n", avgFrameMs);
			printf("  - Update:        %.3f ms (%.1f%%)\n", avgUpdateMs, (totalUpdateTime/totalTime)*100);
			printf("  - Compute:       %.3f ms (%.1f%%)\n", avgComputeMs, (totalComputeTime/totalTime)*100);
			printf("  - Render:        %.3f ms (%.1f%%)\n", avgRenderMs, (totalRenderTime/totalTime)*100);
			printf("Update rate:       %.2f updates/sec\n", frameCount / totalUpdateTime);
			
			// Write speedup data to file for later analysis
			FILE* fp = fopen("speedup_data.txt", "a");
			if (fp) {
				#ifdef _OPENMP
				fprintf(fp, "%d %.6f %.6f %.6f\n", omp_get_max_threads(), avgUpdateMs, avgComputeMs, avgRenderMs);
				#else
				fprintf(fp, "1 %.6f %.6f %.6f\n", avgUpdateMs, avgComputeMs, avgRenderMs);
				#endif
				fclose(fp);
			}
			
			printf("=====================================\n\n");
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