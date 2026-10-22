#ifndef MARCHING_CUBES_H
#define MARCHING_CUBES_H

#include "parser.h"
#include "evaluator.h"
#include "mesh.h"
#include <stdbool.h>

/**
 * @brief Compute the normalized gradient normal vector of f(x, y, z) at a given point.
 * Uses central finite differences: n = normalize(df/dx, df/dy, df/dz).
 */
Vec3 marching_cubes_compute_normal(const ParsedFunction *pf, double x, double y, double z, double step);

/**
 * @brief Extract an isosurface f(x, y, z) = isolevel within a specified 3D bounding box.
 *
 * @param pf Parsed mathematical function
 * @param isolevel Value at which the surface is extracted (typically 0.0)
 * @param min_x Minimum x bound
 * @param max_x Maximum x bound
 * @param res_x Number of cells along x (grid points will be res_x + 1)
 * @param min_y Minimum y bound
 * @param max_y Maximum y bound
 * @param res_y Number of cells along y
 * @param min_z Minimum z bound
 * @param max_z Maximum z bound
 * @param res_z Number of cells along z
 * @param out_mesh Output 3D mesh (will be populated with vertices, normals, and triangle indices)
 * @return true if extraction succeeded, false on memory error
 */
bool marching_cubes_extract(
    const ParsedFunction *pf,
    double isolevel,
    double min_x, double max_x, size_t res_x,
    double min_y, double max_y, size_t res_y,
    double min_z, double max_z, size_t res_z,
    Mesh3D *out_mesh
);

/**
 * @brief Extract an isosurface chunk at integer chunk coordinates (cx, cy, cz) in infinite space.
 *
 * @param pf Parsed mathematical function
 * @param isolevel Isosurface threshold (typically 0.0)
 * @param cx Integer chunk coordinate along X
 * @param cy Integer chunk coordinate along Y
 * @param cz Integer chunk coordinate along Z
 * @param chunk_size World space dimensions of each chunk edge (e.g. 2.0 or 4.0)
 * @param resolution Number of voxels along each edge of the chunk (e.g. 16 or 32)
 * @param out_mesh Output 3D mesh
 * @return true on success
 */
bool marching_cubes_extract_chunk(
    const ParsedFunction *pf,
    double isolevel,
    int cx, int cy, int cz,
    double chunk_size,
    size_t resolution,
    Mesh3D *out_mesh
);

#endif /* MARCHING_CUBES_H */
