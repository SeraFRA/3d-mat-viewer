#include "dynamic_list.h"
#include "token.h"
#include "parser.h"
#include "evaluator.h"
#include "mesh.h"
#include "marching_cubes.h"
#include "viewer.h"
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

static void process_expression_cli(const char *expr, const char *export_obj_path) {
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

    parsed_function_print_tokens(&pf);
    printf("\n");
    parsed_function_print_variables(&pf);
    printf("\n");
    parsed_function_print_instructions(&pf);
    printf("\n");

    printf("=== 3D SPACE EVALUATIONS ===\n");
    evaluate_and_display(&pf, 0.0, 0.0, 0.0);
    evaluate_and_display(&pf, 1.0, 0.0, 0.0);
    evaluate_and_display(&pf, 0.0, 1.0, 0.0);
    evaluate_and_display(&pf, 0.0, 0.0, 1.0);
    evaluate_and_display(&pf, 1.0, 1.0, 1.0);
    evaluate_and_display(&pf, 2.0, -1.0, 0.5);

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
            }
        }
    }

    mesh3d_free(&mesh);
    parsed_function_free(&pf);
}

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    // Check if CLI batch mode is requested via --cli
    if (argc > 1 && (strcmp(argv[1], "--cli") == 0 || strcmp(argv[1], "-c") == 0)) {
        printf("======================================================\n");
        printf("  3D ENGINE CORE - CLI BATCH PARSER & MC              \n");
        printf("======================================================\n");
        for (int i = 2; i < argc; i++) {
            process_expression_cli(argv[i], NULL);
        }
        if (argc == 2) {
            process_expression_cli("x^2 + y^2 + z^2 - 1", "build/demo_sphere.obj");
            process_expression_cli("sin(x) * cos(y) + sin(y) * cos(z) + sin(z) * cos(x)", "build/demo_gyroid.obj");
        }
        return 0;
    }

    // Launch interactive 3D viewer
    const char *initial_formula = (argc > 1) ? argv[1] : "x^2 + y^2 + z^2 - 1";
    return viewer_run(initial_formula);
}
