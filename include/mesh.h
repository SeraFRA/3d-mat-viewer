#ifndef MESH_H
#define MESH_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    float *vertices;        // [x0, y0, z0, x1, y1, z1, ...]
    float *normals;         // [nx0, ny0, nz0, nx1, ny1, nz1, ...]
    uint32_t *indices;      // [i0, i1, i2, ...]
    size_t vertex_count;
    size_t vertex_capacity;
    size_t index_count;
    size_t index_capacity;
} Mesh3D;

/**
 * @brief Initialize an empty 3D mesh.
 */
void mesh3d_init(Mesh3D *mesh);

/**
 * @brief Free all memory allocated by the mesh.
 */
void mesh3d_free(Mesh3D *mesh);

/**
 * @brief Clear vertex and index counts without freeing underlying buffers.
 */
void mesh3d_clear(Mesh3D *mesh);

/**
 * @brief Pre-allocate buffer capacity for vertices and triangle indices.
 */
bool mesh3d_reserve(Mesh3D *mesh, size_t vertex_capacity, size_t index_capacity);

/**
 * @brief Add a single vertex with position and normal to the mesh.
 */
bool mesh3d_add_vertex(Mesh3D *mesh, Vec3 pos, Vec3 normal, uint32_t *out_index);

/**
 * @brief Add a triangle composed of 3 vertex indices.
 */
bool mesh3d_add_triangle(Mesh3D *mesh, uint32_t i0, uint32_t i1, uint32_t i2);

/**
 * @brief Export the 3D mesh to a standard Wavefront .OBJ file (vertices, normals, faces).
 */
bool mesh3d_export_obj(const Mesh3D *mesh, const char *filepath);

#endif /* MESH_H */
