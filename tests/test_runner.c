#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Use our raylib stub when compiling tests */
#include "raylib.h"

/* Pull in boid declarations */
#include "../src/boids.h"

/* Provide deterministic GetTime for tests */
double GetTime(void) { return 0.0; }

/* Simple test helpers */
static int tests_run = 0;
static int tests_failed = 0;

void expect_eqf(const char* name, float a, float b, float tol) {
    tests_run++;
    if (fabsf(a - b) > tol) {
        printf("[FAIL] %s: got %f expected %f\n", name, a, b);
        tests_failed++;
    } else {
        printf("[ OK ] %s\n", name);
    }
}

void expect_eq_vec2(const char* name, Vector2 a, Vector2 b, float tol) {
    tests_run++;
    if (fabsf(a.x - b.x) > tol || fabsf(a.y - b.y) > tol) {
        printf("[FAIL] %s: got (%f,%f) expected (%f,%f)\n", name, a.x, a.y, b.x, b.y);
        tests_failed++;
    } else {
        printf("[ OK ] %s\n", name);
    }
}

/* Forward declarations of functions in boids_baseline.c (non-static) */
float distance(Vector2 v1, Vector2 v2);
float getRotation(Vector2 v1, Vector2 v2);
void rotateBoid(Boid* boid, float theta);
Boid* newBoid(Vector2 origin, Vector2 velocity, float rotation, float angularVelocity);

/* Tests */
void test_distance(void) {
    Vector2 a = {0.0f, 0.0f};
    Vector2 b = {3.0f, 4.0f};
    float d = distance(a, b);
    expect_eqf("distance 3-4-5", d, 5.0f, 1e-6f);
}

void test_getRotation(void) {
    /* Choose v1=(0,0), v2=(0,1). Expect atan2f(-delta.x, delta.y) == M_PI */
    Vector2 v1 = {0.0f, 0.0f};
    Vector2 v2 = {0.0f, 1.0f};
    float r = getRotation(v1, v2);
    expect_eqf("getRotation vertical down", r, M_PI, 1e-5f);
}

void test_rotateBoid(void) {
    Boid b;
    Vector2* positions = malloc(sizeof(Vector2)*3);
    positions[0] = (Vector2){0.0f, -5.0f};
    positions[1] = (Vector2){-5.0f, 5.0f};
    positions[2] = (Vector2){5.0f, 5.0f};

    b.origin = (Vector2){0.0f, 0.0f};
    b.rotation = 0.0f;
    b.positions = positions;
    b.velocity = (Vector2){0.0f, 0.0f};
    b.angularVelocity = 0.0f;
    b.lastUpdate = 0.0;

    /* Rotate by 90 degrees (pi/2) */
    rotateBoid(&b, M_PI/2.0f);

    /* Expected positions after 90deg rotation: (x',y') = (cos*t x - sin*t y, sin*t x + cos*t y) */
    float c = cosf(M_PI/2.0f);
    float s = sinf(M_PI/2.0f);

    Vector2 e0 = { c*0.0f - s*(-5.0f), s*0.0f + c*(-5.0f) };
    Vector2 e1 = { c*(-5.0f) - s*(5.0f), s*(-5.0f) + c*(5.0f) };
    Vector2 e2 = { c*(5.0f) - s*(5.0f), s*(5.0f) + c*(5.0f) };

    expect_eq_vec2("rotateBoid pos0", b.positions[0], e0, 1e-4f);
    expect_eq_vec2("rotateBoid pos1", b.positions[1], e1, 1e-4f);
    expect_eq_vec2("rotateBoid pos2", b.positions[2], e2, 1e-4f);

    /* rotation should be fmod(0 + pi/2, 2pi) */
    expect_eqf("rotateBoid rotation", b.rotation, fmodf(M_PI/2.0f, 2.0f*M_PI), 1e-5f);

    free(positions);
}

void test_newBoid(void) {
    Vector2 orig = {10.0f, 10.0f};
    Vector2 vel = {1.0f, 1.0f};
    float rot = M_PI/2.0f;

    Boid* b = newBoid(orig, vel, rot, 1.0f);
    if (!b) {
        printf("[FAIL] newBoid allocation\n");
        tests_run++;
        tests_failed++;
        return;
    }

    expect_eq_vec2("newBoid origin", b->origin, orig, 1e-6f);
    expect_eqf("newBoid rotation", b->rotation, fmodf(rot, 2.0f*M_PI), 1e-5f);

    free(b->positions);
    free(b);
}

int main(void) {
    printf("Running unit tests...\n");

    test_distance();
    test_getRotation();
    test_rotateBoid();
    test_newBoid();

    printf("\nTests run: %d, Failures: %d\n", tests_run, tests_failed);

    return tests_failed == 0 ? 0 : 1;
}
