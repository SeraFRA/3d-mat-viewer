#include "viewer.h"
#include "marching_cubes.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static const char *PRESETS[] = {
    "x^2 + y^2 + z^2 - 1",                                  // 1: Sfera
    "sin(x) * cos(y) + sin(y) * cos(z) + sin(z) * cos(x)",  // 2: Gyroid (Spazio periodico infinito)
    "x^2 - y^2 - z",                                        // 3: Sella (Paraboloide iperbolico)
    "(x^2 + y^2 + z^2 + 0.8^2 - 0.3^2)^2 - 4 * 0.8^2 * (x^2 + y^2)", // 4: Toro
    "sqrt(x^2 + y^2) - cos(z)",                             // 5: Onda circolare
    "x^2 + y^2 - z^2"                                       // 6: Cono doppio
};

static const size_t NUM_PRESETS = sizeof(PRESETS) / sizeof(PRESETS[0]);

static ViewerState g_viewer_state;
static HWND g_hwnd = NULL;
static HDC g_hdc = NULL;
static HGLRC g_hrc = NULL;
static bool g_running = false;
static int g_window_width = 1024;
static int g_window_height = 768;

void viewer_state_init(ViewerState *state) {
    if (!state) return;
    memset(state, 0, sizeof(ViewerState));

    state->camera.yaw = 45.0f;
    state->camera.pitch = 30.0f;
    state->camera.distance = 5.0f;
    state->camera.target[0] = 0.0f;
    state->camera.target[1] = 0.0f;
    state->camera.target[2] = 0.0f;

    state->show_grid = true;
    state->wireframe = false;
    state->current_preset = 0;
    state->bounds = 2.5;
    state->resolution = 28;

    mesh3d_init(&state->current_mesh);
    parsed_function_init(&state->current_pf);
}

void viewer_state_free(ViewerState *state) {
    if (!state) return;
    mesh3d_free(&state->current_mesh);
    parsed_function_free(&state->current_pf);
}

bool viewer_set_function(ViewerState *state, const char *formula) {
    if (!state || !formula) return false;

    strncpy(state->current_formula, formula, sizeof(state->current_formula) - 1);
    state->current_formula[sizeof(state->current_formula) - 1] = '\0';

    char err_msg[256] = {0};
    ParseStatus st = parse_expression(formula, &state->current_pf, err_msg, sizeof(err_msg));
    if (st != PARSE_SUCCESS) {
        printf("[VIEWER] Parse error on '%s': %s\n", formula, err_msg);
        return false;
    }

    mesh3d_clear(&state->current_mesh);

    double b = state->bounds;
    size_t res = state->resolution;

    // Special bound adjustments for periodic or larger functions
    if (strstr(formula, "sin") || strstr(formula, "cos")) {
        b = 4.0;
    }

    bool ok = marching_cubes_extract(
        &state->current_pf, 0.0,
        -b, b, res,
        -b, b, res,
        -b, b, res,
        &state->current_mesh
    );

    if (g_hwnd) {
        char title_buf[512];
        snprintf(title_buf, sizeof(title_buf),
                 "[3D Engine] f(x,y,z)=0: \"%s\" | %zu vert, %zu tri | Keys: [1-6] Funct, [Space] Wire, [G] Grid, [R] Reset",
                 formula, state->current_mesh.vertex_count, state->current_mesh.index_count / 3);
        SetWindowTextA(g_hwnd, title_buf);
    }

    return ok;
}

static void draw_axes(float length) {
    glDisable(GL_LIGHTING);
    glLineWidth(2.5f);

    glBegin(GL_LINES);
    // X Axis - Red
    glColor3f(1.0f, 0.2f, 0.2f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(length, 0.0f, 0.0f);

    // Y Axis - Green
    glColor3f(0.2f, 1.0f, 0.2f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, length, 0.0f);

    // Z Axis - Blue
    glColor3f(0.2f, 0.4f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, length);
    glEnd();

    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
}

static void draw_infinite_grid(float size, int steps) {
    glDisable(GL_LIGHTING);
    glLineWidth(1.0f);

    float step = (2.0f * size) / (float)steps;

    glBegin(GL_LINES);
    for (int i = -steps / 2; i <= steps / 2; i++) {
        float pos = i * step;
        // Fade lines further from the center
        float dist = fabs(pos) / size;
        float alpha = 0.25f * (1.0f - dist * 0.5f);

        if (i == 0) {
            glColor3f(0.4f, 0.4f, 0.4f); // Center axis darker
        } else {
            glColor3f(alpha, alpha, alpha);
        }

        // Lines parallel to X (on Z plane)
        glVertex3f(-size, 0.0f, pos);
        glVertex3f(size, 0.0f, pos);

        // Lines parallel to Z
        glVertex3f(pos, 0.0f, -size);
        glVertex3f(pos, 0.0f, size);
    }
    glEnd();

    glEnable(GL_LIGHTING);
}

static void render_scene(ViewerState *state) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)g_window_width / (float)(g_window_height > 0 ? g_window_height : 1);
    gluPerspective(45.0, aspect, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Camera view transform
    float rad_yaw = (float)(state->camera.yaw * M_PI / 180.0f);
    float rad_pitch = (float)(state->camera.pitch * M_PI / 180.0f);

    float eye_x = state->camera.target[0] + state->camera.distance * cosf(rad_pitch) * sinf(rad_yaw);
    float eye_y = state->camera.target[1] + state->camera.distance * sinf(rad_pitch);
    float eye_z = state->camera.target[2] + state->camera.distance * cosf(rad_pitch) * cosf(rad_yaw);

    gluLookAt(
        eye_x, eye_y, eye_z,
        state->camera.target[0], state->camera.target[1], state->camera.target[2],
        0.0f, 1.0f, 0.0f
    );

    // Directional light position
    GLfloat light_pos[] = {3.0f, 6.0f, 4.0f, 0.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

    // Draw reference infinite grid & 3D axes
    if (state->show_grid) {
        draw_infinite_grid(10.0f, 40);
        draw_axes(state->bounds + 0.5f);
    }

    // Render the mathematical 3D isosurface mesh
    if (state->current_mesh.index_count > 0) {
        if (state->wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_LIGHTING);
            glColor3f(0.0f, 0.9f, 0.7f);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_LIGHTING);
            glColor3f(0.2f, 0.65f, 0.95f); // Vibrant scientific cyan/blue
        }

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);

        glVertexPointer(3, GL_FLOAT, 0, state->current_mesh.vertices);
        glNormalPointer(GL_FLOAT, 0, state->current_mesh.normals);

        glDrawElements(GL_TRIANGLES, (GLsizei)state->current_mesh.index_count, GL_UNSIGNED_INT, state->current_mesh.indices);

        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);

        if (state->wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_LIGHTING);
        }
    }
}

static LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    ViewerState *st = &g_viewer_state;

    switch (uMsg) {
        case WM_SIZE: {
            g_window_width = LOWORD(lParam);
            g_window_height = HIWORD(lParam);
            glViewport(0, 0, g_window_width, g_window_height);
            return 0;
        }
        case WM_LBUTTONDOWN: {
            st->camera.is_dragging_rot = true;
            st->camera.last_mouse_x = LOWORD(lParam);
            st->camera.last_mouse_y = HIWORD(lParam);
            SetCapture(hwnd);
            return 0;
        }
        case WM_LBUTTONUP: {
            st->camera.is_dragging_rot = false;
            ReleaseCapture();
            return 0;
        }
        case WM_RBUTTONDOWN: {
            st->camera.is_dragging_pan = true;
            st->camera.last_mouse_x = LOWORD(lParam);
            st->camera.last_mouse_y = HIWORD(lParam);
            SetCapture(hwnd);
            return 0;
        }
        case WM_RBUTTONUP: {
            st->camera.is_dragging_pan = false;
            ReleaseCapture();
            return 0;
        }
        case WM_MOUSEMOVE: {
            int mx = LOWORD(lParam);
            int my = HIWORD(lParam);
            int dx = mx - st->camera.last_mouse_x;
            int dy = my - st->camera.last_mouse_y;

            if (st->camera.is_dragging_rot) {
                st->camera.yaw += dx * 0.4f;
                st->camera.pitch += dy * 0.4f;
                if (st->camera.pitch > 89.0f) st->camera.pitch = 89.0f;
                if (st->camera.pitch < -89.0f) st->camera.pitch = -89.0f;
            } else if (st->camera.is_dragging_pan) {
                float rad_yaw = (float)(st->camera.yaw * M_PI / 180.0f);
                float factor = st->camera.distance * 0.002f;
                st->camera.target[0] -= (dx * cosf(rad_yaw) + dy * sinf(rad_yaw)) * factor;
                st->camera.target[2] -= (-dx * sinf(rad_yaw) + dy * cosf(rad_yaw)) * factor;
            }

            st->camera.last_mouse_x = mx;
            st->camera.last_mouse_y = my;
            return 0;
        }
        case WM_MOUSEWHEEL: {
            short delta = GET_WHEEL_DELTA_WPARAM(wParam);
            st->camera.distance -= (float)delta * 0.003f;
            if (st->camera.distance < 0.5f) st->camera.distance = 0.5f;
            if (st->camera.distance > 50.0f) st->camera.distance = 50.0f;
            return 0;
        }
        case WM_KEYDOWN: {
            if (wParam == VK_SPACE) {
                st->wireframe = !st->wireframe;
            } else if (wParam == 'G') {
                st->show_grid = !st->show_grid;
            } else if (wParam == 'R') {
                st->camera.yaw = 45.0f;
                st->camera.pitch = 30.0f;
                st->camera.distance = 5.0f;
                st->camera.target[0] = 0.0f;
                st->camera.target[1] = 0.0f;
                st->camera.target[2] = 0.0f;
            } else if (wParam >= '1' && wParam < '1' + (int)NUM_PRESETS) {
                int idx = (int)(wParam - '1');
                st->current_preset = idx;
                viewer_set_function(st, PRESETS[idx]);
            } else if (wParam == VK_ESCAPE) {
                PostQuitMessage(0);
            }
            return 0;
        }
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }
}

int viewer_run(const char *initial_formula) {
    viewer_state_init(&g_viewer_state);

    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASSEXA wc;
    memset(&wc, 0, sizeof(WNDCLASSEXA));
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = window_proc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "3DMathEngineViewerClass";

    if (!RegisterClassExA(&wc)) {
        printf("[ERROR] Failed to register window class\n");
        return 1;
    }

    g_hwnd = CreateWindowExA(
        0, wc.lpszClassName,
        "3D Math Engine - Infinite Space Viewer",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        g_window_width, g_window_height,
        NULL, NULL, hInstance, NULL
    );

    if (!g_hwnd) {
        printf("[ERROR] Failed to create window\n");
        return 1;
    }

    g_hdc = GetDC(g_hwnd);

    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;

    int pixel_format = ChoosePixelFormat(g_hdc, &pfd);
    SetPixelFormat(g_hdc, pixel_format, &pfd);

    g_hrc = wglCreateContext(g_hdc);
    wglMakeCurrent(g_hdc, g_hrc);

    // OpenGL configuration
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    GLfloat light_ambient[] = {0.2f, 0.2f, 0.25f, 1.0f};
    GLfloat light_diffuse[] = {0.85f, 0.85f, 0.85f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);

    glEnable(GL_NORMALIZE);
    glClearColor(0.08f, 0.09f, 0.12f, 1.0f); // Dark space background

    // Load initial function
    const char *start_formula = (initial_formula && strlen(initial_formula) > 0)
        ? initial_formula
        : PRESETS[0];

    viewer_set_function(&g_viewer_state, start_formula);

    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);

    g_running = true;
    MSG msg;

    printf("\n======================================================\n");
    printf("  3D VIEWER RUNNING\n");
    printf("  Controls:\n");
    printf("  - Left Click + Drag: Rotate Camera (Orbit)\n");
    printf("  - Right Click + Drag: Pan Camera\n");
    printf("  - Scroll Wheel: Zoom In / Out\n");
    printf("  - [Spacebar]: Toggle Solid / Wireframe\n");
    printf("  - [G]: Toggle Infinite Grid\n");
    printf("  - [R]: Reset Camera\n");
    printf("  - Keys [1] - [6]: Switch Mathematical Formulas\n");
    printf("======================================================\n");

    while (g_running) {
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                g_running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

        render_scene(&g_viewer_state);
        SwapBuffers(g_hdc);
        Sleep(10); // ~100 FPS cap to keep CPU/GPU usage at ~0-1%
    }

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(g_hrc);
    ReleaseDC(g_hwnd, g_hdc);
    DestroyWindow(g_hwnd);

    viewer_state_free(&g_viewer_state);
    return 0;
}
