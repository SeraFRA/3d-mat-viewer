#include "evaluator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX_EVAL_STACK 256

const char *eval_status_to_string(EvalStatus status) {
    switch (status) {
        case EVAL_SUCCESS: return "Success";
        case EVAL_ERROR_STACK_UNDERFLOW: return "Stack underflow (missing operand)";
        case EVAL_ERROR_STACK_OVERFLOW: return "Stack overflow";
        case EVAL_ERROR_DIVISION_BY_ZERO: return "Division by zero";
        case EVAL_ERROR_MATH_DOMAIN: return "Math domain error (e.g. sqrt of negative)";
        case EVAL_ERROR_INVALID_VARIABLE_INDEX: return "Invalid variable index";
        case EVAL_ERROR_UNKNOWN_OPCODE: return "Unknown opcode";
        default: return "Unknown error";
    }
}

static bool str_case_eq(const char *s1, const char *s2) {
    if (!s1 || !s2) return false;
    while (*s1 && *s2) {
        if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2)) {
            return false;
        }
        s1++;
        s2++;
    }
    return (*s1 == '\0' && *s2 == '\0');
}

static double eval_func1(MathFunc1Arg id, double arg, EvalStatus *status) {
    switch (id) {
        case FUNC1_SIN:   return sin(arg);
        case FUNC1_COS:   return cos(arg);
        case FUNC1_TAN:   return tan(arg);
        case FUNC1_ASIN:
            if (arg < -1.0 || arg > 1.0) *status = EVAL_ERROR_MATH_DOMAIN;
            return asin(arg);
        case FUNC1_ACOS:
            if (arg < -1.0 || arg > 1.0) *status = EVAL_ERROR_MATH_DOMAIN;
            return acos(arg);
        case FUNC1_ATAN:  return atan(arg);
        case FUNC1_SINH:  return sinh(arg);
        case FUNC1_COSH:  return cosh(arg);
        case FUNC1_TANH:  return tanh(arg);
        case FUNC1_EXP:   return exp(arg);
        case FUNC1_LOG:
            if (arg <= 0.0) *status = EVAL_ERROR_MATH_DOMAIN;
            return log(arg);
        case FUNC1_LOG10:
            if (arg <= 0.0) *status = EVAL_ERROR_MATH_DOMAIN;
            return log10(arg);
        case FUNC1_SQRT:
            if (arg < 0.0) *status = EVAL_ERROR_MATH_DOMAIN;
            return sqrt(arg);
        case FUNC1_CBRT:  return cbrt(arg);
        case FUNC1_ABS:   return fabs(arg);
        case FUNC1_FLOOR: return floor(arg);
        case FUNC1_CEIL:  return ceil(arg);
        default:
            *status = EVAL_ERROR_UNKNOWN_OPCODE;
            return 0.0;
    }
}

static double eval_func2(MathFunc2Arg id, double a, double b, EvalStatus *status) {
    (void)status;
    switch (id) {
        case FUNC2_MIN:   return (a < b) ? a : b;
        case FUNC2_MAX:   return (a > b) ? a : b;
        case FUNC2_ATAN2: return atan2(a, b);
        case FUNC2_POW:   return pow(a, b);
        default:
            *status = EVAL_ERROR_UNKNOWN_OPCODE;
            return 0.0;
    }
}

EvalStatus eval_function(const ParsedFunction *pf, const double *var_values, size_t var_count, double *out_result) {
    if (!pf || !out_result) {
        return EVAL_ERROR_STACK_UNDERFLOW;
    }

    if (pf->instructions.count == 0) {
        return EVAL_ERROR_STACK_UNDERFLOW;
    }

    double stack[MAX_EVAL_STACK];
    size_t top = 0;

    for (size_t i = 0; i < pf->instructions.count; i++) {
        const Instruction *instr = (const Instruction *)dynamic_list_get(&pf->instructions, i);
        if (!instr) continue;

        switch (instr->opcode) {
            case OP_LOAD_NUM: {
                if (top >= MAX_EVAL_STACK) return EVAL_ERROR_STACK_OVERFLOW;
                stack[top++] = instr->number;
                break;
            }
            case OP_LOAD_VAR: {
                int idx = instr->variable.index;
                if (idx < 0 || (size_t)idx >= var_count || !var_values) {
                    return EVAL_ERROR_INVALID_VARIABLE_INDEX;
                }
                if (top >= MAX_EVAL_STACK) return EVAL_ERROR_STACK_OVERFLOW;
                stack[top++] = var_values[idx];
                break;
            }
            case OP_ADD: {
                if (top < 2) return EVAL_ERROR_STACK_UNDERFLOW;
                double b = stack[--top];
                double a = stack[--top];
                stack[top++] = a + b;
                break;
            }
            case OP_SUB: {
                if (top < 2) return EVAL_ERROR_STACK_UNDERFLOW;
                double b = stack[--top];
                double a = stack[--top];
                stack[top++] = a - b;
                break;
            }
            case OP_MUL: {
                if (top < 2) return EVAL_ERROR_STACK_UNDERFLOW;
                double b = stack[--top];
                double a = stack[--top];
                stack[top++] = a * b;
                break;
            }
            case OP_DIV: {
                if (top < 2) return EVAL_ERROR_STACK_UNDERFLOW;
                double b = stack[--top];
                double a = stack[--top];
                if (fabs(b) < 1e-15) {
                    return EVAL_ERROR_DIVISION_BY_ZERO;
                }
                stack[top++] = a / b;
                break;
            }
            case OP_POW: {
                if (top < 2) return EVAL_ERROR_STACK_UNDERFLOW;
                double b = stack[--top];
                double a = stack[--top];
                stack[top++] = pow(a, b);
                break;
            }
            case OP_NEG: {
                if (top < 1) return EVAL_ERROR_STACK_UNDERFLOW;
                stack[top - 1] = -stack[top - 1];
                break;
            }
            case OP_FUNC_1ARG: {
                if (top < 1) return EVAL_ERROR_STACK_UNDERFLOW;
                EvalStatus st = EVAL_SUCCESS;
                double res = eval_func1(instr->func1.id, stack[top - 1], &st);
                if (st != EVAL_SUCCESS) return st;
                stack[top - 1] = res;
                break;
            }
            case OP_FUNC_2ARG: {
                if (top < 2) return EVAL_ERROR_STACK_UNDERFLOW;
                double b = stack[--top];
                double a = stack[--top];
                EvalStatus st = EVAL_SUCCESS;
                double res = eval_func2(instr->func2.id, a, b, &st);
                if (st != EVAL_SUCCESS) return st;
                stack[top++] = res;
                break;
            }
            default:
                return EVAL_ERROR_UNKNOWN_OPCODE;
        }
    }

    if (top != 1) {
        return EVAL_ERROR_STACK_UNDERFLOW;
    }

    *out_result = stack[0];
    return EVAL_SUCCESS;
}

EvalStatus eval_function_3d(const ParsedFunction *pf, double x, double y, double z, double *out_result) {
    if (!pf) return EVAL_ERROR_STACK_UNDERFLOW;

    size_t var_count = pf->variables.count;
    double var_values[32]; // supports up to 32 variables, typical 3D is 3 (x, y, z)
    if (var_count > 32) return EVAL_ERROR_STACK_OVERFLOW;

    for (size_t i = 0; i < var_count; i++) {
        const Variable *v = (const Variable *)dynamic_list_get(&pf->variables, i);
        if (!v) {
            var_values[i] = 0.0;
            continue;
        }
        if (str_case_eq(v->name, "x")) {
            var_values[i] = x;
        } else if (str_case_eq(v->name, "y")) {
            var_values[i] = y;
        } else if (str_case_eq(v->name, "z")) {
            var_values[i] = z;
        } else {
            var_values[i] = 0.0;
        }
    }

    return eval_function(pf, var_values, var_count, out_result);
}

EvalStatus eval_function_grid_3d(
    const ParsedFunction *pf,
    double min_x, double max_x, size_t nx,
    double min_y, double max_y, size_t ny,
    double min_z, double max_z, size_t nz,
    double *out_grid
) {
    if (!pf || !out_grid || nx == 0 || ny == 0 || nz == 0) {
        return EVAL_ERROR_STACK_UNDERFLOW;
    }

    int x_slot = -1;
    int y_slot = -1;
    int z_slot = -1;

    for (size_t i = 0; i < pf->variables.count; i++) {
        const Variable *v = (const Variable *)dynamic_list_get(&pf->variables, i);
        if (!v) continue;
        if (str_case_eq(v->name, "x")) x_slot = v->index;
        else if (str_case_eq(v->name, "y")) y_slot = v->index;
        else if (str_case_eq(v->name, "z")) z_slot = v->index;
    }

    size_t var_count = pf->variables.count;
    double var_values[32];
    for (size_t i = 0; i < var_count; i++) var_values[i] = 0.0;

    double dx = (nx > 1) ? (max_x - min_x) / (double)(nx - 1) : 0.0;
    double dy = (ny > 1) ? (max_y - min_y) / (double)(ny - 1) : 0.0;
    double dz = (nz > 1) ? (max_z - min_z) / (double)(nz - 1) : 0.0;

    size_t out_idx = 0;
    for (size_t ix = 0; ix < nx; ix++) {
        double vx = min_x + (double)ix * dx;
        if (x_slot >= 0) var_values[x_slot] = vx;

        for (size_t iy = 0; iy < ny; iy++) {
            double vy = min_y + (double)iy * dy;
            if (y_slot >= 0) var_values[y_slot] = vy;

            for (size_t iz = 0; iz < nz; iz++) {
                double vz = min_z + (double)iz * dz;
                if (z_slot >= 0) var_values[z_slot] = vz;

                double res = 0.0;
                EvalStatus st = eval_function(pf, var_values, var_count, &res);
                if (st != EVAL_SUCCESS) {
                    res = NAN;
                }
                out_grid[out_idx++] = res;
            }
        }
    }

    return EVAL_SUCCESS;
}
