#include "dynamic_list.h"
#include "token.h"
#include "parser.h"
#include "evaluator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static int g_tests_run = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define ASSERT_TRUE(condition, msg) do { \
    g_tests_run++; \
    if (condition) { \
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

/* ----------------- Unit Test 1: Dynamic List ----------------- */
static void test_dynamic_list(void) {
    printf("[TEST] DynamicList basic operations & resizing...\n");
    DynamicList list;
    bool ok = dynamic_list_init(&list, sizeof(int), 2);
    ASSERT_TRUE(ok, "Init dynamic list");
    ASSERT_TRUE(list.count == 0, "Initial count is 0");
    ASSERT_TRUE(list.capacity == 2, "Initial capacity is 2");

    int v1 = 10, v2 = 20, v3 = 30;
    dynamic_list_push(&list, &v1);
    dynamic_list_push(&list, &v2);
    dynamic_list_push(&list, &v3); // triggers resize

    ASSERT_TRUE(list.count == 3, "Count is 3 after 3 pushes");
    ASSERT_TRUE(list.capacity >= 3, "Capacity resized");

    int *p1 = (int *)dynamic_list_get(&list, 0);
    int *p2 = (int *)dynamic_list_get(&list, 1);
    int *p3 = (int *)dynamic_list_get(&list, 2);
    ASSERT_TRUE(p1 && *p1 == 10, "First element is 10");
    ASSERT_TRUE(p2 && *p2 == 20, "Second element is 20");
    ASSERT_TRUE(p3 && *p3 == 30, "Third element is 30");

    int popped = 0;
    ok = dynamic_list_pop(&list, &popped);
    ASSERT_TRUE(ok && popped == 30, "Popped element is 30");
    ASSERT_TRUE(list.count == 2, "Count after pop is 2");

    dynamic_list_free(&list);
    ASSERT_TRUE(list.data == NULL && list.count == 0, "List cleared after free");
}

/* ----------------- Unit Test 2: Arithmetic Precedence ----------------- */
static void test_arithmetic_precedence(void) {
    printf("[TEST] Arithmetic precedence and associativity...\n");
    ParsedFunction pf = {0};
    char err[128];

    // Precedence: 2 + 3 * 4 = 14
    ParseStatus st = parse_expression("2 + 3 * 4", &pf, err, sizeof(err));
    ASSERT_TRUE(st == PARSE_SUCCESS, "Parse '2 + 3 * 4'");
    double res = 0.0;
    EvalStatus est = eval_function(&pf, NULL, 0, &res);
    ASSERT_TRUE(est == EVAL_SUCCESS, "Eval '2 + 3 * 4'");
    ASSERT_NEAR(res, 14.0, 1e-9, "2 + 3 * 4 == 14");
    parsed_function_free(&pf);

    // Parentheses: (2 + 3) * 4 = 20
    st = parse_expression("(2 + 3) * 4", &pf, err, sizeof(err));
    ASSERT_TRUE(st == PARSE_SUCCESS, "Parse '(2 + 3) * 4'");
    est = eval_function(&pf, NULL, 0, &res);
    ASSERT_TRUE(est == EVAL_SUCCESS, "Eval '(2 + 3) * 4'");
    ASSERT_NEAR(res, 20.0, 1e-9, "(2 + 3) * 4 == 20");
    parsed_function_free(&pf);

    // Exponentiation right-associativity: 2 ^ 3 ^ 2 = 2 ^ 9 = 512
    st = parse_expression("2 ^ 3 ^ 2", &pf, err, sizeof(err));
    ASSERT_TRUE(st == PARSE_SUCCESS, "Parse '2 ^ 3 ^ 2'");
    est = eval_function(&pf, NULL, 0, &res);
    ASSERT_TRUE(est == EVAL_SUCCESS, "Eval '2 ^ 3 ^ 2'");
    ASSERT_NEAR(res, 512.0, 1e-9, "2 ^ 3 ^ 2 == 512");
    parsed_function_free(&pf);
}

/* ----------------- Unit Test 3: Unary Minus and Negation ----------------- */
static void test_unary_minus(void) {
    printf("[TEST] Unary minus expressions...\n");
    ParsedFunction pf = {0};
    char err[128];

    // -5 + 8 = 3
    ParseStatus st = parse_expression("-5 + 8", &pf, err, sizeof(err));
    ASSERT_TRUE(st == PARSE_SUCCESS, "Parse '-5 + 8'");
    double res = 0.0;
    EvalStatus est = eval_function(&pf, NULL, 0, &res);
    ASSERT_TRUE(est == EVAL_SUCCESS, "Eval '-5 + 8'");
    ASSERT_NEAR(res, 3.0, 1e-9, "-5 + 8 == 3");
    parsed_function_free(&pf);

    // 4 * -2 = -8
    st = parse_expression("4 * -2", &pf, err, sizeof(err));
    ASSERT_TRUE(st == PARSE_SUCCESS, "Parse '4 * -2'");
    est = eval_function(&pf, NULL, 0, &res);
    ASSERT_TRUE(est == EVAL_SUCCESS, "Eval '4 * -2'");
    ASSERT_NEAR(res, -8.0, 1e-9, "4 * -2 == -8");
    parsed_function_free(&pf);

    // -(-x) with x = 7 -> 7
    st = parse_expression("-(-x)", &pf, err, sizeof(err));
    ASSERT_TRUE(st == PARSE_SUCCESS, "Parse '-(-x)'");
    ASSERT_TRUE(pf.variables.count == 1, "Detected 1 variable 'x'");
    est = eval_function_3d(&pf, 7.0, 0.0, 0.0, &res);
    ASSERT_TRUE(est == EVAL_SUCCESS, "Eval '-(-x)' with x=7");
    ASSERT_NEAR(res, 7.0, 1e-9, "-(-7) == 7");
    parsed_function_free(&pf);
}

/* ----------------- Unit Test 4: Variable Extraction & Dynamic Lists ----------------- */
static void test_variable_extraction_and_lists(void) {
    printf("[TEST] Variable extraction in dynamic list...\n");
    ParsedFunction pf = {0};
    char err[128];

    // Sphere equation: x^2 + y^2 + z^2 - 1
    ParseStatus st = parse_expression("x^2 + y^2 + z^2 - 1", &pf, err, sizeof(err));
    ASSERT_TRUE(st == PARSE_SUCCESS, "Parse sphere 'x^2 + y^2 + z^2 - 1'");
    ASSERT_TRUE(pf.variables.count == 3, "Sphere has 3 variables");

    int idx_x = parsed_function_find_var_index(&pf, "x");
    int idx_y = parsed_function_find_var_index(&pf, "y");
    int idx_z = parsed_function_find_var_index(&pf, "z");
    ASSERT_TRUE(idx_x >= 0, "Found 'x'");
    ASSERT_TRUE(idx_y >= 0, "Found 'y'");
    ASSERT_TRUE(idx_z >= 0, "Found 'z'");

    // Tokens dynamic list should contain tokens
    ASSERT_TRUE(pf.tokens.count > 0, "Tokens list populated");
    // Instructions dynamic list should contain RPN operations
    ASSERT_TRUE(pf.instructions.count > 0, "Instructions list populated");

    // Evaluate on sphere boundary: (1, 0, 0) -> 0.0
    double res = 0.0;
    EvalStatus est = eval_function_3d(&pf, 1.0, 0.0, 0.0, &res);
    ASSERT_TRUE(est == EVAL_SUCCESS, "Eval sphere at (1, 0, 0)");
    ASSERT_NEAR(res, 0.0, 1e-9, "f(1, 0, 0) == 0");

    // Evaluate at origin: (0, 0, 0) -> -1.0
    est = eval_function_3d(&pf, 0.0, 0.0, 0.0, &res);
    ASSERT_TRUE(est == EVAL_SUCCESS, "Eval sphere at (0, 0, 0)");
    ASSERT_NEAR(res, -1.0, 1e-9, "f(0, 0, 0) == -1");

    // Evaluate outside sphere: (1, 1, 1) -> 2.0
    est = eval_function_3d(&pf, 1.0, 1.0, 1.0, &res);
    ASSERT_TRUE(est == EVAL_SUCCESS, "Eval sphere at (1, 1, 1)");
    ASSERT_NEAR(res, 2.0, 1e-9, "f(1, 1, 1) == 2");

    parsed_function_free(&pf);
}

/* ----------------- Unit Test 5: Math Functions and Constants ----------------- */
static void test_math_functions(void) {
    printf("[TEST] Mathematical functions & constants...\n");
    ParsedFunction pf = {0};
    char err[128];

    // sin(x)^2 + cos(x)^2 = 1.0
    ParseStatus st = parse_expression("sin(x)^2 + cos(x)^2", &pf, err, sizeof(err));
    ASSERT_TRUE(st == PARSE_SUCCESS, "Parse 'sin(x)^2 + cos(x)^2'");
    double res = 0.0;
    eval_function_3d(&pf, 1.234, 0.0, 0.0, &res);
    ASSERT_NEAR(res, 1.0, 1e-9, "Pythagorean identity for x=1.234");
    parsed_function_free(&pf);

    // sqrt(x^2 + y^2) - z (Cone)
    st = parse_expression("sqrt(x^2 + y^2) - z", &pf, err, sizeof(err));
    ASSERT_TRUE(st == PARSE_SUCCESS, "Parse cone 'sqrt(x^2 + y^2) - z'");
    eval_function_3d(&pf, 3.0, 4.0, 5.0, &res);
    ASSERT_NEAR(res, 0.0, 1e-9, "Cone at (3, 4, 5) -> 0");
    parsed_function_free(&pf);

    // Constant pi: sin(pi / 2) = 1.0
    st = parse_expression("sin(pi / 2)", &pf, err, sizeof(err));
    ASSERT_TRUE(st == PARSE_SUCCESS, "Parse 'sin(pi / 2)'");
    eval_function(&pf, NULL, 0, &res);
    ASSERT_NEAR(res, 1.0, 1e-9, "sin(pi/2) == 1");
    parsed_function_free(&pf);

    // 2-arg function: min(x, y) + max(y, z)
    st = parse_expression("min(x, y) + max(y, z)", &pf, err, sizeof(err));
    ASSERT_TRUE(st == PARSE_SUCCESS, "Parse min and max");
    eval_function_3d(&pf, 2.0, 5.0, 8.0, &res);
    // min(2, 5) = 2, max(5, 8) = 8 -> 10
    ASSERT_NEAR(res, 10.0, 1e-9, "min(2, 5) + max(5, 8) == 10");
    parsed_function_free(&pf);
}

/* ----------------- Unit Test 6: Implicit Multiplication ----------------- */
static void test_implicit_multiplication(void) {
    printf("[TEST] Implicit multiplication...\n");
    ParsedFunction pf = {0};
    char err[128];

    // 2x + 3y - 4z
    ParseStatus st = parse_expression("2x + 3y - 4z", &pf, err, sizeof(err));
    ASSERT_TRUE(st == PARSE_SUCCESS, "Parse '2x + 3y - 4z'");
    double res = 0.0;
    eval_function_3d(&pf, 1.0, 2.0, 3.0, &res);
    // 2(1) + 3(2) - 4(3) = 2 + 6 - 12 = -4
    ASSERT_NEAR(res, -4.0, 1e-9, "2(1) + 3(2) - 4(3) == -4");
    parsed_function_free(&pf);

    // (x + 1)(y - 1)
    st = parse_expression("(x + 1)(y - 1)", &pf, err, sizeof(err));
    ASSERT_TRUE(st == PARSE_SUCCESS, "Parse '(x + 1)(y - 1)'");
    eval_function_3d(&pf, 3.0, 5.0, 0.0, &res);
    // (3 + 1) * (5 - 1) = 4 * 4 = 16
    ASSERT_NEAR(res, 16.0, 1e-9, "(3 + 1)(5 - 1) == 16");
    parsed_function_free(&pf);
}

/* ----------------- Unit Test 7: 3D Grid Evaluation ----------------- */
static void test_3d_grid_evaluation(void) {
    printf("[TEST] 3D grid scalar field evaluation...\n");
    ParsedFunction pf = {0};
    char err[128];

    // Sphere: x^2 + y^2 + z^2 - 1
    ParseStatus st = parse_expression("x^2 + y^2 + z^2 - 1", &pf, err, sizeof(err));
    ASSERT_TRUE(st == PARSE_SUCCESS, "Parse for 3D grid");

    const size_t nx = 5, ny = 5, nz = 5;
    double grid[5 * 5 * 5];
    EvalStatus est = eval_function_grid_3d(&pf, -1.0, 1.0, nx, -1.0, 1.0, ny, -1.0, 1.0, nz, grid);
    ASSERT_TRUE(est == EVAL_SUCCESS, "Grid 3D evaluation successful");

    // Center of grid: ix=2, iy=2, iz=2 corresponds to (0, 0, 0)
    size_t center_idx = 2 * (ny * nz) + 2 * nz + 2;
    ASSERT_NEAR(grid[center_idx], -1.0, 1e-9, "Grid center at (0, 0, 0) is -1.0");

    // Corner of grid: ix=4, iy=4, iz=4 corresponds to (1, 1, 1) -> 1^2 + 1^2 + 1^2 - 1 = 2.0
    size_t corner_idx = 4 * (ny * nz) + 4 * nz + 4;
    ASSERT_NEAR(grid[corner_idx], 2.0, 1e-9, "Grid corner at (1, 1, 1) is 2.0");

    parsed_function_free(&pf);
}

/* ----------------- Unit Test 8: Error Handling ----------------- */
static void test_error_handling(void) {
    printf("[TEST] Error handling & edge cases...\n");
    ParsedFunction pf = {0};
    char err[128];

    // Empty input
    ParseStatus st = parse_expression("", &pf, err, sizeof(err));
    ASSERT_TRUE(st == PARSE_ERROR_EMPTY_INPUT, "Detect empty string");

    // Mismatched open parenthesis
    st = parse_expression("(x + 2 * y", &pf, err, sizeof(err));
    ASSERT_TRUE(st == PARSE_ERROR_MISMATCHED_PAREN, "Detect missing closing parenthesis");

    // Mismatched closing parenthesis
    st = parse_expression("x + 2 * y)", &pf, err, sizeof(err));
    ASSERT_TRUE(st == PARSE_ERROR_MISMATCHED_PAREN, "Detect extra closing parenthesis");

    // Invalid character
    st = parse_expression("x @ y", &pf, err, sizeof(err));
    ASSERT_TRUE(st == PARSE_ERROR_INVALID_CHAR, "Detect invalid character '@'");

    // Division by zero in evaluator
    st = parse_expression("x / y", &pf, err, sizeof(err));
    ASSERT_TRUE(st == PARSE_SUCCESS, "Parse 'x / y'");
    double res = 0.0;
    EvalStatus est = eval_function_3d(&pf, 10.0, 0.0, 0.0, &res);
    ASSERT_TRUE(est == EVAL_ERROR_DIVISION_BY_ZERO, "Detect division by zero at runtime");
    parsed_function_free(&pf);
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    printf("=========================================\n");
    printf("   3D ENGINE C PARSER - UNIT TEST SUITE  \n");
    printf("=========================================\n");

    test_dynamic_list();
    test_arithmetic_precedence();
    test_unary_minus();
    test_variable_extraction_and_lists();
    test_math_functions();
    test_implicit_multiplication();
    test_3d_grid_evaluation();
    test_error_handling();

    printf("\n=========================================\n");
    printf("RESULTS: %d / %d passed", g_tests_passed, g_tests_run);
    if (g_tests_failed > 0) {
        printf(" (%d FAILED)\n", g_tests_failed);
        printf("=========================================\n");
        return 1;
    } else {
        printf(" (ALL TESTS PASSED!)\n");
        printf("=========================================\n");
        return 0;
    }
}
