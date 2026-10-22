#include "marching_cubes.h"
#include "marching_cubes_tables.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

Vec3 marching_cubes_compute_normal(const ParsedFunction *pf, double x, double y, double z, double step) {
    double h = (step > 1e-7) ? step : 1e-4;

    double fx_plus = 0.0, fx_minus = 0.0;
    eval_function_3d(pf, x + h, y, z, &fx_plus);
    eval_function_3d(pf, x - h, y, z, &fx_minus);
    double dx = (fx_plus - fx_minus) / (2.0 * h);

    double fy_plus = 0.0, fy_minus = 0.0;
    eval_function_3d(pf, x, y + h, z, &fy_plus);
    eval_function_3d(pf, x, y - h, z, &fy_minus);
    double dy = (fy_plus - fy_minus) / (2.0 * h);

    double fz_plus = 0.0, fz_minus = 0.0;
    eval_function_3d(pf, x, y, z + h, &fz_plus);
    eval_function_3d(pf, x, y, z - h, &fz_minus);
    double dz = (fz_plus - fz_minus) / (2.0 * h);

    double len = sqrt(dx * dx + dy * dy + dz * dz);
    Vec3 n;
    if (len > 1e-12) {
        n.x = (float)(dx / len);
        n.y = (float)(dy / len);
        n.z = (float)(dz / len);
    } else {
        n.x = 0.0f;
        n.y = 1.0f;
        n.z = 0.0f;
    }
    return n;
}

static inline Vec3 interpolate_edge(Vec3 p1, Vec3 p2, double v1, double v2, double isolevel) {
    double diff = v2 - v1;
    float t = 0.5f;
    if (fabs(diff) > 1e-12) {
        t = (float)((isolevel - v1) / diff);
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
    }

    Vec3 result;
    result.x = p1.x + t * (p2.x - p1.x);
    result.y = p1.y + t * (p2.y - p1.y);
    result.z = p1.z + t * (p2.z - p1.z);
    return result;
}

bool marching_cubes_extract(
    const ParsedFunction *pf,
    double isolevel,
    double min_x, double max_x, size_t res_x,
    double min_y, double max_y, size_t res_y,
    double min_z, double max_z, size_t res_z,
    Mesh3D *out_mesh
) {
    if (!pf || !out_mesh || res_x == 0 || res_y == 0 || res_z == 0) {
        return false;
    }

    size_t gx = res_x + 1;
    size_t gy = res_y + 1;
    size_t gz = res_z + 1;
    size_t total_samples = gx * gy * gz;

    double *grid = (double *)malloc(total_samples * sizeof(double));
    if (!grid) {
        return false;
    }

    // Step 1: Pre-evaluate scalar field across grid points
    EvalStatus est = eval_function_grid_3d(pf, min_x, max_x, gx, min_y, max_y, gy, min_z, max_z, gz, grid);
    if (est != EVAL_SUCCESS) {
        free(grid);
        return false;
    }

    double dx = (max_x - min_x) / (double)res_x;
    double dy = (max_y - min_y) / (double)res_y;
    double dz = (max_z - min_z) / (double)res_z;
    double normal_step = fmin(dx, fmin(dy, dz)) * 0.25;
    if (normal_step < 1e-5) normal_step = 1e-4;

    // Corner offset multipliers mapping to 8 corners of MC_CORNER_OFFSETS
    static const size_t corner_ix[8] = {0, 1, 1, 0, 0, 1, 1, 0};
    static const size_t corner_iy[8] = {0, 0, 1, 1, 0, 0, 1, 1};
    static const size_t corner_iz[8] = {0, 0, 0, 0, 1, 1, 1, 1};

    // Step 2: Iterate over every cube cell
    for (size_t ix = 0; ix < res_x; ix++) {
        for (size_t iy = 0; iy < res_y; iy++) {
            for (size_t iz = 0; iz < res_z; iz++) {
                double corner_val[8];
                Vec3 corner_pos[8];
                int cube_index = 0;

                for (int c = 0; c < 8; c++) {
                    size_t cx = ix + corner_ix[c];
                    size_t cy = iy + corner_iy[c];
                    size_t cz = iz + corner_iz[c];
                    size_t g_idx = cx * (gy * gz) + cy * gz + cz;

                    corner_val[c] = grid[g_idx];
                    corner_pos[c].x = (float)(min_x + (double)cx * dx);
                    corner_pos[c].y = (float)(min_y + (double)cy * dy);
                    corner_pos[c].z = (float)(min_z + (double)cz * dz);

                    if (corner_val[c] < isolevel) {
                        cube_index |= (1 << c);
                    }
                }

                uint16_t edge_flags = MC_EDGE_TABLE[cube_index];
                if (edge_flags == 0) {
                    continue; // Cube is entirely inside or entirely outside
                }

                // Calculate vertices on intersected edges
                Vec3 edge_vertices[12];
                for (int e = 0; e < 12; e++) {
                    if (edge_flags & (1 << e)) {
                        int c1 = MC_EDGE_VERTICES[e][0];
                        int c2 = MC_EDGE_VERTICES[e][1];
                        edge_vertices[e] = interpolate_edge(
                            corner_pos[c1], corner_pos[c2],
                            corner_val[c1], corner_val[c2],
                            isolevel
                        );
                    }
                }

                // Emit triangles from the triangulation table
                for (int t = 0; MC_TRI_TABLE[cube_index][t] != -1; t += 3) {
                    int e0 = MC_TRI_TABLE[cube_index][t];
                    int e1 = MC_TRI_TABLE[cube_index][t + 1];
                    int e2 = MC_TRI_TABLE[cube_index][t + 2];

                    Vec3 v0 = edge_vertices[e0];
                    Vec3 v1 = edge_vertices[e1];
                    Vec3 v2 = edge_vertices[e2];

                    Vec3 n0 = marching_cubes_compute_normal(pf, v0.x, v0.y, v0.z, normal_step);
                    Vec3 n1 = marching_cubes_compute_normal(pf, v1.x, v1.y, v1.z, normal_step);
                    Vec3 n2 = marching_cubes_compute_normal(pf, v2.x, v2.y, v2.z, normal_step);

                    uint32_t i0, i1, i2;
                    mesh3d_add_vertex(out_mesh, v0, n0, &i0);
                    mesh3d_add_vertex(out_mesh, v1, n1, &i1);
                    mesh3d_add_vertex(out_mesh, v2, n2, &i2);

                    mesh3d_add_triangle(out_mesh, i0, i1, i2);
                }
            }
        }
    }

    free(grid);
    return true;
}

bool marching_cubes_extract_chunk(
    const ParsedFunction *pf,
    double isolevel,
    int cx, int cy, int cz,
    double chunk_size,
    size_t resolution,
    Mesh3D *out_mesh
) {
    if (chunk_size <= 0.0 || resolution == 0) {
        return false;
    }

    double min_x = (double)cx * chunk_size;
    double max_x = min_x + chunk_size;

    double min_y = (double)cy * chunk_size;
    double max_y = min_y + chunk_size;

    double min_z = (double)cz * chunk_size;
    double max_z = min_z + chunk_size;

    return marching_cubes_extract(
        pf, isolevel,
        min_x, max_x, resolution,
        min_y, max_y, resolution,
        min_z, max_z, resolution,
        out_mesh
    );
}
