#ifndef RAYLIB_STUB_H
#define RAYLIB_STUB_H

#include <stddef.h>

/* Minimal raylib stubs used for unit testing boid logic. */

typedef struct Vector2 { float x; float y; } Vector2;

/* Color placeholders */
#define BLUE 0
#define RAYWHITE 1

/* Functions used by the boids implementation. Tests provide implementations where needed. */

/* Time in seconds (tests will provide this function). */
extern double GetTime(void);

/* Drawing stub (no-op for tests). */
static inline void DrawTriangle(Vector2 a, Vector2 b, Vector2 c, int color) { (void)a; (void)b; (void)c; (void)color; }

/* Random/value stubs used by mains but not by unit tests; provide simple signatures. */
static inline int GetRandomValue(int min, int max) { (void)min; (void)max; return 0; }
static inline void SetRandomSeed(int seed) { (void)seed; }

/* Windowing / control stubs (no-op) */
static inline void InitWindow(int w, int h, const char* title) { (void)w; (void)h; (void)title; }
static inline void CloseWindow(void) {}
static inline void SetTargetFPS(int fps) { (void)fps; }
static inline void rlDisableBackfaceCulling(void) {}
static inline int WindowShouldClose(void) { return 1; }

#endif /* RAYLIB_STUB_H */
