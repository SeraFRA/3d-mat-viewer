#include "dynamic_list.h"
#include "token.h"
#include "parser.h"
#include "evaluator.h"
#include "mesh.h"
#include "marching_cubes.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static int g_tests_run = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define ASSERT_TRUE(cond, msg) do { \
    g_tests_run++; \
    if (cond) { \
        g_tests_passed++; \
    } else { \
        g_tests_failed++; \
        printf("[FAIL] Line %d: %s\n", __LINE__, msg); \
    } \
} while(0)

#define ASSERT_NEAR(a, b, eps, msg) do { \
    g_tests_run++; \
    if (fabs((a) - (b)) <= (eps)) { \
        g_tests_passed++; \
    } else { \
        g_tests_failed++; \
        printf("[FAIL] Line %d: %s (expected %g, got %g, diff %g)\n", __LINE__, msg, (double)(b), (double)(a), fabs((a)-(b))); \
    } \
} while(0)

static void test_mesh_operations(void) {
    printf("[TEST] Mesh3D operations...\n");
    Mesh3D mesh;
    mesh3d_init(&mesh);

    ASSERT_TRUE(mesh.vertex_count == 0, "Initial vertex count 0");
    ASSERT_TRUE(mesh.index_count == 0, "Initial index count 0");

    Vec3 v1 = {1.0f, 2.0f, 3.0f};
    Vec3 n1 = {0.0f, 1.0f, 0.0f};
    uint32_t idx1 = 0;
    bool ok = mesh3d_add_vertex(&mesh, v1, n1, &idx1);
    ASSERT_TRUE(ok && idx1 == 0, "Add first vertex");

    Vec3 v2 = {4.0f, 5.0f, 6.0f};
    Vec3 n2 = {0.0f, 1.0f, 0.0f};
    uint32_t idx2 = 0;
    mesh3d_add_vertex(&mesh, v2, n2, &idx2);

    Vec3 v3 = {7.0f, 8.0f, 9.0f};
    Vec3 n3 = {0.0f, 1.0f, 0.0f};
    uint32_t idx3 = 0;
    mesh3d_add_vertex(&mesh, v3, n3, &idx3);

    ok = mesh3d_add_triangle(&mesh, idx1, idx2, idx3);
    ASSERT_TRUE(ok, "Add triangle");
    ASSERT_TRUE(mesh.vertex_count == 3, "Vertex count is 3");
    ASSERT_TRUE(mesh.index_count == 3, "Index count is 3");

    mesh3d_free(&mesh);
    ASSERT_TRUE(mesh.vertices == NULL && mesh.vertex_count == 0, "Mesh freed cleanly");
}

static void test_normal_calculation(void) {
    printf("[TEST] Marching cubes gradient normals...\n");
    ParsedFunction pf = {0};
    char err[128];
    parse_expression("x^2 + y^2 + z^2 - 1", &pf, err, sizeof(err));

    // For a sphere, the normal at (1, 0, 0) should be (1, 0, 0)
    Vec3 n_x = marching_cubes_compute_normal(&pf, 1.0, 0.0, 0.0, 1e-4);
    ASSERT_NEAR(n_x.x, 1.0f, 1e-3, "Normal at (1, 0, 0) X");
    ASSERT_NEAR(n_x.y, 0.0f, 1e-3, "Normal at (1, 0, 0) Y");
    ASSERT_NEAR(n_x.z, 0.0f, 1e-3, "Normal at (1, 0, 0) Z");

    // Normal at (0, 1, 0) should be (0, 1, 0)
    Vec3 n_y = marching_cubes_compute_normal(&pf, 0.0, 1.0, 0.0, 1e-4);
    ASSERT_NEAR(n_y.x, 0.0f, 1e-3, "Normal at (0, 1, 0) X");
    ASSERT_NEAR(n_y.y, 1.0f, 1e-3, "Normal at (0, 1, 0) Y");
    ASSERT_NEAR(n_y.z, 0.0f, 1e-3, "Normal at (0, 1, 0) Z");

    // Normal at (0, 0, 1) should be (0, 0, 1)
    Vec3 n_z = marching_cubes_compute_normal(&pf, 0.0, 0.0, 1.0, 1e-4);
    ASSERT_NEAR(n_z.x, 0.0f, 1e-3, "Normal at (0, 0, 1) X");
    ASSERT_NEAR(n_z.y, 0.0f, 1e-3, "Normal at (0, 0, 1) Y");
    ASSERT_NEAR(n_z.z, 1.0f, 1e-3, "Normal at (0, 0, 1) Z");

    parsed_function_free(&pf);
}

static void test_sphere_extraction(void) {
    printf("[TEST] Marching cubes sphere triangulation...\n");
    ParsedFunction pf = {0};
    char err[128];
    parse_expression("x^2 + y^2 + z^2 - 1", &pf, err, sizeof(err));

    Mesh3D mesh;
    mesh3d_init(&mesh);

    bool ok = marching_cubes_extract(&pf, 0.0, -1.5, 1.5, 20, -1.5, 1.5, 20, -1.5, 1.5, 20, &mesh);
    ASSERT_TRUE(ok, "Sphere extraction call returned true");
    ASSERT_TRUE(mesh.vertex_count > 100, "Sphere has vertices generated");
    ASSERT_TRUE(mesh.index_count > 100, "Sphere has triangles generated");
    ASSERT_TRUE(mesh.index_count % 3 == 0, "Indices form whole triangles");

    printf("  Extracted sphere: %zu vertices, %zu triangles\n", mesh.vertex_count, mesh.index_count / 3);

    // Verify all vertices lie near radius 1.0
    double max_err = 0.0;
    for (size_t i = 0; i < mesh.vertex_count; i++) {
        float vx = mesh.vertices[i * 3 + 0];
        float vy = mesh.vertices[i * 3 + 1];
        float vz = mesh.vertices[i * 3 + 2];
        double radius = sqrt(vx * vx + vy * vy + vz * vz);
        double err_r = fabs(radius - 1.0);
        if (err_r > max_err) max_err = err_r;

        // Normal must be unit length
        float nx = mesh.normals[i * 3 + 0];
        float ny = mesh.normals[i * 3 + 1];
        float nz = mesh.normals[i * 3 + 2];
        double norm_len = sqrt(nx * nx + ny * ny + nz * nz);
        ASSERT_NEAR(norm_len, 1.0, 1e-2, "Normal vector is normalized");
    }

    ASSERT_TRUE(max_err < 0.05, "Sphere vertex radial error within resolution tolerance");
    printf("  Max sphere radial error: %g\n", max_err);

    mesh3d_free(&mesh);
    parsed_function_free(&pf);
}

static void test_plane_extraction(void) {
    printf("[TEST] Marching cubes flat plane extraction...\n");
    ParsedFunction pf = {0};
    char err[128];
    parse_expression("z", &pf, err, sizeof(err));

    Mesh3D mesh;
    mesh3d_init(&mesh);

    // Extract z = 0 plane
    marching_cubes_extract(&pf, 0.0, -2.0, 2.0, 10, -2.0, 2.0, 10, -2.0, 2.0, 10, &mesh);
    ASSERT_TRUE(mesh.vertex_count > 0, "Plane extracted vertices");

    for (size_t i = 0; i < mesh.vertex_count; i++) {
        float vz = mesh.vertices[i * 3 + 2];
        ASSERT_NEAR(vz, 0.0f, 1e-4, "Plane vertex Z is exactly 0");

        float nz = mesh.normals[i * 3 + 2];
        ASSERT_NEAR(nz, 1.0f, 1e-3, "Plane normal points in +Z direction");
    }

    mesh3d_free(&mesh);
    parsed_function_free(&pf);
}

static void test_empty_space_and_chunks(void) {
    printf("[TEST] Marching cubes empty space and chunking...\n");
    ParsedFunction pf = {0};
    char err[128];
    parse_expression("x^2 + y^2 + z^2 - 1", &pf, err, sizeof(err));

    Mesh3D mesh;
    mesh3d_init(&mesh);

    // Chunk far away from the sphere (e.g. [10, 12] in world space)
    bool ok = marching_cubes_extract_chunk(&pf, 0.0, 5, 5, 5, 2.0, 16, &mesh);
    ASSERT_TRUE(ok, "Far chunk extraction succeeded");
    ASSERT_TRUE(mesh.vertex_count == 0, "Far chunk contains 0 vertices");
    ASSERT_TRUE(mesh.index_count == 0, "Far chunk contains 0 triangles");

    // Chunk at origin (cx=0, cy=0, cz=0, chunk_size=2.0 -> bounds [0, 2])
    // The sphere intersects this octant!
    ok = marching_cubes_extract_chunk(&pf, 0.0, 0, 0, 0, 2.0, 16, &mesh);
    ASSERT_TRUE(ok, "Origin chunk extraction succeeded");
    ASSERT_TRUE(mesh.vertex_count > 0, "Origin chunk intersects sphere");

    mesh3d_free(&mesh);
    parsed_function_free(&pf);
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    printf("=========================================\n");
    printf("  MARCHING CUBES 3D ENGINE - UNIT TESTS  \n");
    printf("=========================================\n");

    test_mesh_operations();
    test_normal_calculation();
    test_sphere_extraction();
    test_plane_extraction();
    test_empty_space_and_chunks();

    printf("\n=========================================\n");
    printf("RESULTS: %d / %d passed", g_tests_passed, g_tests_run);
    if (g_tests_failed > 0) {
        printf(" (%d FAILED)\n", g_tests_failed);
        printf("=========================================\n");
        return 1;
    } else {
        printf(" (ALL MARCHING CUBES TESTS PASSED!)\n");
        printf("=========================================\n");
        return 0;
    }
}
