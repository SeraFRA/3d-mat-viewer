#include "mesh.h"
#include <stdlib.h>
#include <string.h>

#define DEFAULT_MESH_CAPACITY 256

void mesh3d_init(Mesh3D *mesh) {
    if (!mesh) return;
    memset(mesh, 0, sizeof(Mesh3D));
}

void mesh3d_free(Mesh3D *mesh) {
    if (!mesh) return;
    if (mesh->vertices) {
        free(mesh->vertices);
        mesh->vertices = NULL;
    }
    if (mesh->normals) {
        free(mesh->normals);
        mesh->normals = NULL;
    }
    if (mesh->indices) {
        free(mesh->indices);
        mesh->indices = NULL;
    }
    mesh->vertex_count = 0;
    mesh->vertex_capacity = 0;
    mesh->index_count = 0;
    mesh->index_capacity = 0;
}

void mesh3d_clear(Mesh3D *mesh) {
    if (!mesh) return;
    mesh->vertex_count = 0;
    mesh->index_count = 0;
}

bool mesh3d_reserve(Mesh3D *mesh, size_t vertex_capacity, size_t index_capacity) {
    if (!mesh) return false;

    if (vertex_capacity > mesh->vertex_capacity) {
        float *new_v = (float *)realloc(mesh->vertices, vertex_capacity * 3 * sizeof(float));
        float *new_n = (float *)realloc(mesh->normals, vertex_capacity * 3 * sizeof(float));
        if (!new_v || !new_n) {
            if (new_v) mesh->vertices = new_v;
            if (new_n) mesh->normals = new_n;
            return false;
        }
        mesh->vertices = new_v;
        mesh->normals = new_n;
        mesh->vertex_capacity = vertex_capacity;
    }

    if (index_capacity > mesh->index_capacity) {
        uint32_t *new_idx = (uint32_t *)realloc(mesh->indices, index_capacity * sizeof(uint32_t));
        if (!new_idx) {
            return false;
        }
        mesh->indices = new_idx;
        mesh->index_capacity = index_capacity;
    }

    return true;
}

bool mesh3d_add_vertex(Mesh3D *mesh, Vec3 pos, Vec3 normal, uint32_t *out_index) {
    if (!mesh) return false;

    if (mesh->vertex_count >= mesh->vertex_capacity) {
        size_t new_cap = (mesh->vertex_capacity == 0) ? DEFAULT_MESH_CAPACITY : mesh->vertex_capacity * 2;
        if (!mesh3d_reserve(mesh, new_cap, mesh->index_capacity)) {
            return false;
        }
    }

    size_t idx = mesh->vertex_count;
    mesh->vertices[idx * 3 + 0] = pos.x;
    mesh->vertices[idx * 3 + 1] = pos.y;
    mesh->vertices[idx * 3 + 2] = pos.z;

    mesh->normals[idx * 3 + 0] = normal.x;
    mesh->normals[idx * 3 + 1] = normal.y;
    mesh->normals[idx * 3 + 2] = normal.z;

    mesh->vertex_count++;
    if (out_index) {
        *out_index = (uint32_t)idx;
    }
    return true;
}

bool mesh3d_add_triangle(Mesh3D *mesh, uint32_t i0, uint32_t i1, uint32_t i2) {
    if (!mesh) return false;

    if (mesh->index_count + 3 > mesh->index_capacity) {
        size_t new_cap = (mesh->index_capacity == 0) ? DEFAULT_MESH_CAPACITY * 3 : mesh->index_capacity * 2;
        if (!mesh3d_reserve(mesh, mesh->vertex_capacity, new_cap)) {
            return false;
        }
    }

    mesh->indices[mesh->index_count + 0] = i0;
    mesh->indices[mesh->index_count + 1] = i1;
    mesh->indices[mesh->index_count + 2] = i2;
    mesh->index_count += 3;
    return true;
}

#include <stdio.h>

bool mesh3d_export_obj(const Mesh3D *mesh, const char *filepath) {
    if (!mesh || !filepath) return false;

    FILE *f = fopen(filepath, "w");
    if (!f) return false;

    fprintf(f, "# 3D Math Engine Generated Mesh\n");
    fprintf(f, "# Vertices: %zu, Triangles: %zu\n\n", mesh->vertex_count, mesh->index_count / 3);

    // Write vertices
    for (size_t i = 0; i < mesh->vertex_count; i++) {
        fprintf(f, "v %f %f %f\n",
            mesh->vertices[i * 3 + 0],
            mesh->vertices[i * 3 + 1],
            mesh->vertices[i * 3 + 2]
        );
    }

    // Write normals
    for (size_t i = 0; i < mesh->vertex_count; i++) {
        fprintf(f, "vn %f %f %f\n",
            mesh->normals[i * 3 + 0],
            mesh->normals[i * 3 + 1],
            mesh->normals[i * 3 + 2]
        );
    }

    // Write triangle faces (1-based indices in OBJ format: v//vn)
    for (size_t i = 0; i < mesh->index_count; i += 3) {
        uint32_t idx0 = mesh->indices[i + 0] + 1;
        uint32_t idx1 = mesh->indices[i + 1] + 1;
        uint32_t idx2 = mesh->indices[i + 2] + 1;
        fprintf(f, "f %u//%u %u//%u %u//%u\n", idx0, idx0, idx1, idx1, idx2, idx2);
    }

    fclose(f);
    return true;
}
