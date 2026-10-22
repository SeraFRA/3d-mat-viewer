#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "parser.h"
#include <stdbool.h>

typedef enum {
    EVAL_SUCCESS = 0,
    EVAL_ERROR_STACK_UNDERFLOW,
    EVAL_ERROR_STACK_OVERFLOW,
    EVAL_ERROR_DIVISION_BY_ZERO,
    EVAL_ERROR_MATH_DOMAIN,
    EVAL_ERROR_INVALID_VARIABLE_INDEX,
    EVAL_ERROR_UNKNOWN_OPCODE
} EvalStatus;

/**
 * @brief Human readable string for evaluation status.
 */
const char *eval_status_to_string(EvalStatus status);

/**
 * @brief Evaluate the parsed function with an array of variable values indexed by their Variable.index.
 *
 * @param pf Parsed function containing RPN instructions
 * @param var_values Array of values for variables, sized pf->variables.count
 * @param var_count Number of elements in var_values
 * @param out_result Pointer to store the computed double result
 * @return EvalStatus EVAL_SUCCESS on success
 */
EvalStatus eval_function(const ParsedFunction *pf, const double *var_values, size_t var_count, double *out_result);

/**
 * @brief Convenience evaluator for 3D functions f(x, y, z).
 * Looks up variables named "x", "y", "z" (case-insensitive) and binds them.
 *
 * @param pf Parsed function
 * @param x Value for variable x
 * @param y Value for variable y
 * @param z Value for variable z
 * @param out_result Pointer to store the computed double result
 * @return EvalStatus EVAL_SUCCESS on success
 */
EvalStatus eval_function_3d(const ParsedFunction *pf, double x, double y, double z, double *out_result);

/**
 * @brief Evaluate function over a 3D grid [nx * ny * nz], populating output buffer.
 * Contiguous row-major array: out_grid[ix * (ny*nz) + iy * nz + iz].
 *
 * @param pf Parsed function
 * @param min_x Minimum x bound
 * @param max_x Maximum x bound
 * @param nx Number of samples along x
 * @param min_y Minimum y bound
 * @param max_y Maximum y bound
 * @param ny Number of samples along y
 * @param min_z Minimum z bound
 * @param max_z Maximum z bound
 * @param nz Number of samples along z
 * @param out_grid Preallocated buffer of size (nx * ny * nz * sizeof(double))
 * @return EvalStatus EVAL_SUCCESS on success
 */
EvalStatus eval_function_grid_3d(
    const ParsedFunction *pf,
    double min_x, double max_x, size_t nx,
    double min_y, double max_y, size_t ny,
    double min_z, double max_z, size_t nz,
    double *out_grid
);

#endif /* EVALUATOR_H */
