#include "dynamic_list.h"
#include "token.h"
#include "parser.h"
#include "evaluator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void evaluate_and_display(ParsedFunction *pf, double x, double y, double z) {
    double res = 0.0;
    EvalStatus est = eval_function_3d(pf, x, y, z, &res);
    if (est == EVAL_SUCCESS) {
        printf("  f(%g, %g, %g) = %g\n", x, y, z, res);
    } else {
        printf("  f(%g, %g, %g) -> Evaluation Error: %s\n", x, y, z, eval_status_to_string(est));
    }
}

static void process_expression(const char *expr) {
    ParsedFunction pf = {0};
    char err_msg[256] = {0};

    printf("\n======================================================\n");
    printf("Parsing expression: \"%s\"\n", expr);
    printf("======================================================\n");

    ParseStatus status = parse_expression(expr, &pf, err_msg, sizeof(err_msg));
    if (status != PARSE_SUCCESS) {
        printf("[ERROR] Parsing failed: %s (code %d)\n", err_msg, status);
        parsed_function_free(&pf);
        return;
    }

    // Display parsed tokens dynamic list
    parsed_function_print_tokens(&pf);
    printf("\n");

    // Display detected variables dynamic list
    parsed_function_print_variables(&pf);
    printf("\n");

    // Display postfix operations dynamic list
    parsed_function_print_instructions(&pf);
    printf("\n");

    // Sample evaluations in 3D space
    printf("=== 3D SPACE EVALUATIONS ===\n");
    evaluate_and_display(&pf, 0.0, 0.0, 0.0);
    evaluate_and_display(&pf, 1.0, 0.0, 0.0);
    evaluate_and_display(&pf, 0.0, 1.0, 0.0);
    evaluate_and_display(&pf, 0.0, 0.0, 1.0);
    evaluate_and_display(&pf, 1.0, 1.0, 1.0);
    evaluate_and_display(&pf, 2.0, -1.0, 0.5);

    parsed_function_free(&pf);
}

int main(int argc, char **argv) {
    printf("======================================================\n");
    printf("  3D ENGINE CORE - MATHEMATICAL FUNCTION PARSER (C)  \n");
    printf("======================================================\n");

    if (argc > 1) {
        // Run with command line arguments
        for (int i = 1; i < argc; i++) {
            process_expression(argv[i]);
        }
        return 0;
    }

    // Default demonstration functions
    const char *demos[] = {
        "x^2 + y^2 + z^2 - 1",                     // Sphere
        "sqrt(x^2 + y^2) - z",                     // Cone
        "sin(x) * cos(y) + sin(y) * cos(z) + sin(z) * cos(x)", // Gyroid-like approximation
        "x^2 - y^2 - z"                            // Hyperbolic paraboloid (saddle)
    };

    size_t num_demos = sizeof(demos) / sizeof(demos[0]);
    for (size_t i = 0; i < num_demos; i++) {
        process_expression(demos[i]);
    }

    printf("\n[INFO] You can also pass expressions as arguments, e.g.:\n");
    printf("       ./engine_parser \"x^2 + y^2 + z^2 - 9\"\n");

    return 0;
}
