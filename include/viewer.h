#ifndef VIEWER_H
#define VIEWER_H

#include "parser.h"
#include "mesh.h"
#include <stdbool.h>

typedef struct {
    float yaw;          // Horizontal rotation angle in degrees
    float pitch;        // Vertical rotation angle in degrees
    float distance;     // Distance from target
    float target[3];    // Center of focus (look-at point)
    bool is_dragging_rot;
    bool is_dragging_pan;
    int last_mouse_x;
    int last_mouse_y;
} Camera3D;

typedef struct {
    Camera3D camera;
    Mesh3D current_mesh;
    ParsedFunction current_pf;
    char current_formula[256];
    bool show_grid;
    bool wireframe;
    int current_preset;
    double bounds;      // Half-extent of bounding box (e.g. 2.0 -> [-2, 2])
    size_t resolution;  // Marching cubes voxel resolution along each axis
} ViewerState;

/**
 * @brief Initialize the 3D viewer state.
 */
void viewer_state_init(ViewerState *state);

/**
 * @brief Free all resources in the viewer state.
 */
void viewer_state_free(ViewerState *state);

/**
 * @brief Update the active mathematical function, re-parse, and re-extract 3D isosurface mesh.
 */
bool viewer_set_function(ViewerState *state, const char *formula);

/**
 * @brief Run the interactive 3D native OpenGL window.
 * Blocks until the user closes the window.
 *
 * @param initial_formula Mathematical formula to display at startup
 * @return 0 on normal exit, non-zero on error
 */
int viewer_run(const char *initial_formula);

#endif /* VIEWER_H */
