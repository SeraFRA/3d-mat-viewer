#include "dynamic_list.h"
#include "token.h"
#include "parser.h"
#include "evaluator.h"
#include "mesh.h"
#include "marching_cubes.h"
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

static void process_expression(const char *expr, const char *export_obj_path) {
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

    // Marching cubes 3D isosurface extraction demo
    printf("\n=== 3D MARCHING CUBES ISOSURFACE EXTRACTION ===\n");
    Mesh3D mesh;
    mesh3d_init(&mesh);

    bool ok = marching_cubes_extract(&pf, 0.0, -2.0, 2.0, 24, -2.0, 2.0, 24, -2.0, 2.0, 24, &mesh);
    if (ok) {
        printf("  Extracted Mesh: %zu vertices, %zu triangles (bounds [-2, 2], res 24x24x24)\n",
               mesh.vertex_count, mesh.index_count / 3);

        if (export_obj_path) {
            if (mesh3d_export_obj(&mesh, export_obj_path)) {
                printf("  [SUCCESS] Exported 3D mesh to '%s'\n", export_obj_path);
            } else {
                printf("  [ERROR] Failed to export OBJ to '%s'\n", export_obj_path);
            }
        }
    } else {
        printf("  [ERROR] Marching Cubes extraction failed\n");
    }

    mesh3d_free(&mesh);
    parsed_function_free(&pf);
}

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    printf("======================================================\n");
    printf("  3D ENGINE CORE - MATHEMATICAL FUNCTION PARSER & MC  \n");
    printf("======================================================\n");

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            process_expression(argv[i], NULL);
        }
        return 0;
    }

    // Default demonstration functions
    process_expression("x^2 + y^2 + z^2 - 1", "build/demo_sphere.obj");
    process_expression("sin(x) * cos(y) + sin(y) * cos(z) + sin(z) * cos(x)", "build/demo_gyroid.obj");
    process_expression("x^2 - y^2 - z", "build/demo_saddle.obj");

    printf("\n[INFO] You can pass any expression to parse, e.g.:\n");
    printf("       ./engine_parser \"x^2 + y^2 + z^2 - 4\"\n");

    return 0;
}
