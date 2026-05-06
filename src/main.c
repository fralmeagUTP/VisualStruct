#include "raylib.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <direct.h>

#include "algorithm_trace.h"
#include "app_state.h"
#include "code_viewer.h"
#include "grafo_code_viewer.h"
#include "grafo_pedagogy.h"
#include "grafo_trace.h"
#include "cola_prioridad_view.h"
#include "cola_view.h"
#include "lista_view.h"
#include "lista_circular_view.h"
#include "pila_view.h"
#include "sublista_view.h"
#include "ui.h"

/**
 * @file main.c
 * @brief Punto de entrada y orquestacion principal de la app VisualStruct.
 */

#define CODE_HISTORY_CAPACITY 65536

/** @brief Convierte el enum de estructura activa a texto legible para la UI. */
static const char *estructura_nombre(TipoEstructura tipo) {
    switch (tipo) {
    case ESTRUCTURA_PILA:
        return "Pila";
    case ESTRUCTURA_COLA:
        return "Cola";
    case ESTRUCTURA_COLA_PRIORIDAD:
        return "Cola de Prioridad";
    case ESTRUCTURA_LISTA:
        return "Lista Enlazada";
    case ESTRUCTURA_LISTA_CIRCULAR:
        return "Lista Circular";
    case ESTRUCTURA_SUBLISTA:
        return "Sublistas";
    case ESTRUCTURA_GRAFO:
        return "Grafo";
    default:
        return "N/A";
    }
}

/** @brief Convierte el enum de operacion a texto legible para el historial. */
static const char *operacion_nombre(TipoOperacion operacion) {
    switch (operacion) {
    case OPERACION_INICIALIZAR:
        return "Inicializar";
    case OPERACION_INSERTAR:
        return "Insertar";
    case OPERACION_INSERTAR_INICIO:
        return "Insertar Inicio";
    case OPERACION_INSERTAR_FINAL:
        return "Insertar Final";
    case OPERACION_ELIMINAR:
        return "Eliminar";
    case OPERACION_BUSCAR:
        return "Buscar";
    case OPERACION_INVERTIR:
        return "Invertir";
    case OPERACION_SUBLISTA_INSERTAR_HIJO:
        return "Insertar Hijo";
    case OPERACION_SUBLISTA_ELIMINAR_HIJO:
        return "Eliminar Hijo";
    case OPERACION_VACIAR:
        return "Vaciar";
    default:
        return "Ninguna";
    }
}

/** @brief Agrega una nueva entrada de snippet al historial acumulado de codigo. */
static void append_code_history(char *history, size_t capacity, int *entries, const AppState *app,
                                const char *snippet) {
    char block[3072];
    size_t used;
    int written;

    if (history == NULL || entries == NULL || app == NULL || capacity == 0) {
        return;
    }

    if (snippet == NULL || snippet[0] == '\0') {
        snippet = "// Snippet no disponible";
    }

    written = snprintf(block, sizeof(block),
                       "/* Paso %d | %s | %s | %s */\n"
                       "// %s\n"
                       "%s\n\n",
                       *entries + 1, estructura_nombre(app->estructura_activa),
                       operacion_nombre(app->operacion_actual),
                       app->ultima_operacion_ok ? "OK" : "Error", app->mensaje_operacion, snippet);
    if (written < 0) {
        return;
    }

    used = strlen(history);
    if (used + (size_t)written + 1 >= capacity) {
        snprintf(history, capacity, "/* Historial reiniciado: limite alcanzado */\n\n");
        used = strlen(history);
    }

    if (used < capacity - 1) {
        snprintf(history + used, capacity - used, "%s", block);
        (*entries)++;
    }
}

/** @brief Crea una vista previa multilinea limitada para paneles compactos. */
static void build_compact_preview(const char *source, char *dest, size_t capacity, int max_lines) {
    int lines = 1;
    size_t used = 0;
    bool truncated = false;

    if (dest == NULL || capacity == 0) {
        return;
    }
    dest[0] = '\0';
    if (source == NULL || source[0] == '\0' || max_lines <= 0) {
        return;
    }

    while (*source != '\0' && used + 1 < capacity) {
        if (*source == '\r') {
            source++;
            continue;
        }
        if (*source == '\n') {
            if (lines >= max_lines) {
                truncated = true;
                break;
            }
            lines++;
        }
        dest[used++] = *source++;
    }
    if (*source != '\0') {
        truncated = true;
    }
    dest[used] = '\0';

    if (truncated && used + 6 < capacity) {
        snprintf(dest + used, capacity - used, "\n...");
    }
}

/** @brief Retorna la estructura asociada a un atajo numerico, o -1 si no aplica. */
static int estructura_from_shortcut(void) {
    if (IsKeyPressed(KEY_ONE)) {
        return ESTRUCTURA_PILA;
    }
    if (IsKeyPressed(KEY_TWO)) {
        return ESTRUCTURA_COLA;
    }
    if (IsKeyPressed(KEY_THREE)) {
        return ESTRUCTURA_LISTA;
    }
    if (IsKeyPressed(KEY_FOUR)) {
        return ESTRUCTURA_COLA_PRIORIDAD;
    }
    if (IsKeyPressed(KEY_FIVE)) {
        return ESTRUCTURA_LISTA_CIRCULAR;
    }
    if (IsKeyPressed(KEY_SIX)) {
        return ESTRUCTURA_SUBLISTA;
    }
    if (IsKeyPressed(KEY_SEVEN)) {
        return ESTRUCTURA_GRAFO;
    }
    return -1;
}

typedef enum {
    INPUT_NONE = 0,
    INPUT_VALOR,
    INPUT_PRIORIDAD,
    INPUT_GRAFO_ORIGEN,
    INPUT_GRAFO_DESTINO,
    INPUT_GRAFO_PESO
} InputFocus;

typedef enum {
    SCREEN_HOME_ROOT = 0,
    SCREEN_HOME_SECUENCIALES,
    SCREEN_HOME_GRAFOS,
    SCREEN_VISUALIZER,
    SCREEN_HELP
} ScreenMode;

/* 0 Construccion, 1 Recorridos, 2 Caminos, 3 MST */
static int g_grafo_ui_mode = 0;
static int g_grafo_recorrido_algo = GRAFO_ALGO_BFS;
static int g_grafo_camino_algo = GRAFO_ALGO_DIJKSTRA;

#define E2E_VISUAL_STAGE_COUNT 9

typedef struct {
    bool enabled;
    char output_dir[260];
    int stage;
    int frame_in_stage;
    int capture_frame;
    int advance_frame;
    int captures;
    bool finished;
} E2EVisualMode;

/** @brief Crea una ruta de carpetas de forma incremental (equivalente a mkdir -p). */
static void e2e_visual_ensure_directory(const char *path) {
    char buffer[260];
    size_t len;
    size_t i;

    if (path == NULL || path[0] == '\0') {
        return;
    }

    snprintf(buffer, sizeof(buffer), "%s", path);
    len = strlen(buffer);
    if (len == 0) {
        return;
    }
    for (i = 0; i < len; i++) {
        if (buffer[i] == '\\' || buffer[i] == '/') {
            char saved = buffer[i];
            buffer[i] = '\0';
            if (strlen(buffer) > 0) {
                _mkdir(buffer);
            }
            buffer[i] = saved;
        }
    }
    _mkdir(buffer);
}

/** @brief Retorna nombre corto de stage para archivo de evidencia visual. */
static const char *e2e_visual_stage_name(int stage) {
    switch (stage) {
    case 0:
        return "home_root";
    case 1:
        return "home_grafos";
    case 2:
        return "grafo_construccion";
    case 3:
        return "grafo_bfs";
    case 4:
        return "grafo_dfs";
    case 5:
        return "grafo_dijkstra";
    case 6:
        return "grafo_bellman";
    case 7:
        return "grafo_prim";
    case 8:
        return "grafo_kruskal";
    default:
        return "desconocido";
    }
}

/** @brief Lee argumentos CLI para activar modo de captura E2E visual. */
static void e2e_visual_parse_args(E2EVisualMode *e2e, int argc, char **argv) {
    int i;
    if (e2e == NULL) {
        return;
    }

    memset(e2e, 0, sizeof(*e2e));
    snprintf(e2e->output_dir, sizeof(e2e->output_dir), "artifacts/e2e_visual");
    e2e->stage = -1;
    e2e->capture_frame = 16;
    e2e->advance_frame = 24;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--e2e-visual") == 0) {
            e2e->enabled = true;
        } else if (strncmp(argv[i], "--e2e-out=", 10) == 0) {
            snprintf(e2e->output_dir, sizeof(e2e->output_dir), "%s", argv[i] + 10);
            e2e->enabled = true;
        } else if (strcmp(argv[i], "--e2e-out") == 0 && (i + 1) < argc) {
            i++;
            snprintf(e2e->output_dir, sizeof(e2e->output_dir), "%s", argv[i]);
            e2e->enabled = true;
        }
    }

    if (e2e->enabled) {
        e2e_visual_ensure_directory(e2e->output_dir);
    }
}

/** @brief Retorna una descripcion corta para la estructura seleccionada. */
static const char *estructura_descripcion(TipoEstructura tipo) {
    switch (tipo) {
    case ESTRUCTURA_PILA:
        return "Estructura LIFO.\nLos ultimos en entrar\nson los primeros en salir.";
    case ESTRUCTURA_COLA:
        return "Estructura FIFO.\nLos primeros en entrar\nson los primeros en salir.";
    case ESTRUCTURA_LISTA:
        return "Coleccion de nodos\nenlazados en secuencia.\nTamano dinamico.";
    case ESTRUCTURA_COLA_PRIORIDAD:
        return "Cada nodo tiene\nprioridad y sale\nprimero el menor valor.";
    case ESTRUCTURA_LISTA_CIRCULAR:
        return "Nodos enlazados\nen ciclo cerrado:\nel ultimo apunta al primero.";
    case ESTRUCTURA_SUBLISTA:
        return "Cada nodo padre\ntiene una lista\nde nodos hijo.";
    case ESTRUCTURA_GRAFO:
        return "Vertices y aristas\npara recorridos y\nalgoritmos de caminos.";
    default:
        return "";
    }
}

/** @brief Dibuja texto multilínea simple separando por salto de línea. */
static void draw_ui_multiline(const char *text, float x, float y, float font_size, float spacing,
                              Color color) {
    const char *start = text;
    const char *line_end;
    char line[192];
    int line_index = 0;
    size_t len;

    while (start != NULL && *start != '\0') {
        line_end = strchr(start, '\n');
        len = (line_end == NULL) ? strlen(start) : (size_t)(line_end - start);
        if (len >= sizeof(line)) {
            len = sizeof(line) - 1;
        }
        memcpy(line, start, len);
        line[len] = '\0';

        ui_draw_text(line, x, y + line_index * (font_size + 5.0f), font_size, spacing, color,
                     false);

        line_index++;
        if (line_end == NULL) {
            break;
        }
        start = line_end + 1;
    }
}

/** @brief Dibuja una tarjeta de estructura y retorna true si se presiona Visualizar. */
static void draw_home_icon(TipoEstructura tipo, Rectangle icon_circle) {
    float cx = icon_circle.x + icon_circle.width * 0.5f;
    float cy = icon_circle.y + icon_circle.height * 0.5f;
    Color c = (Color){28, 74, 136, 255};

    switch (tipo) {
    case ESTRUCTURA_PILA:
        DrawRectangleRounded((Rectangle){cx - 16.0f, cy - 22.0f, 32.0f, 11.0f}, 0.25f, 6,
                             Fade(c, 0.18f));
        DrawRectangleRoundedLinesEx((Rectangle){cx - 16.0f, cy - 22.0f, 32.0f, 11.0f}, 0.25f, 6,
                                    1.8f, c);
        DrawRectangleRounded((Rectangle){cx - 16.0f, cy - 7.0f, 32.0f, 11.0f}, 0.25f, 6,
                             Fade(c, 0.18f));
        DrawRectangleRoundedLinesEx((Rectangle){cx - 16.0f, cy - 7.0f, 32.0f, 11.0f}, 0.25f, 6,
                                    1.8f, c);
        DrawRectangleRounded((Rectangle){cx - 16.0f, cy + 8.0f, 32.0f, 11.0f}, 0.25f, 6,
                             Fade(c, 0.18f));
        DrawRectangleRoundedLinesEx((Rectangle){cx - 16.0f, cy + 8.0f, 32.0f, 11.0f}, 0.25f, 6,
                                    1.8f, c);
        break;
    case ESTRUCTURA_COLA:
        DrawRectangleRoundedLinesEx((Rectangle){cx - 25.0f, cy - 13.0f, 16.0f, 26.0f}, 0.2f, 6,
                                    2.0f, c);
        DrawRectangleRoundedLinesEx((Rectangle){cx - 5.0f, cy - 13.0f, 16.0f, 26.0f}, 0.2f, 6,
                                    2.0f, c);
        DrawLineEx((Vector2){cx - 36.0f, cy}, (Vector2){cx - 28.0f, cy}, 2.0f, c);
        DrawTriangle((Vector2){cx - 28.0f, cy}, (Vector2){cx - 33.0f, cy - 4.0f},
                     (Vector2){cx - 33.0f, cy + 4.0f}, c);
        DrawLineEx((Vector2){cx + 22.0f, cy}, (Vector2){cx + 30.0f, cy}, 2.0f, c);
        DrawTriangle((Vector2){cx + 30.0f, cy}, (Vector2){cx + 25.0f, cy - 4.0f},
                     (Vector2){cx + 25.0f, cy + 4.0f}, c);
        break;
    case ESTRUCTURA_LISTA:
        DrawRectangleRounded((Rectangle){cx - 30.0f, cy - 6.0f, 10.0f, 12.0f}, 0.2f, 4,
                             Fade(c, 0.18f));
        DrawRectangleRounded((Rectangle){cx - 10.0f, cy - 6.0f, 10.0f, 12.0f}, 0.2f, 4,
                             Fade(c, 0.18f));
        DrawRectangleRounded((Rectangle){cx + 10.0f, cy - 6.0f, 10.0f, 12.0f}, 0.2f, 4,
                             Fade(c, 0.18f));
        DrawRectangleRoundedLinesEx((Rectangle){cx - 30.0f, cy - 6.0f, 10.0f, 12.0f}, 0.2f, 4,
                                    1.6f, c);
        DrawRectangleRoundedLinesEx((Rectangle){cx - 10.0f, cy - 6.0f, 10.0f, 12.0f}, 0.2f, 4,
                                    1.6f, c);
        DrawRectangleRoundedLinesEx((Rectangle){cx + 10.0f, cy - 6.0f, 10.0f, 12.0f}, 0.2f, 4,
                                    1.6f, c);
        DrawLineEx((Vector2){cx - 20.0f, cy}, (Vector2){cx - 10.0f, cy}, 1.8f, c);
        DrawLineEx((Vector2){cx, cy}, (Vector2){cx + 10.0f, cy}, 1.8f, c);
        break;
    case ESTRUCTURA_COLA_PRIORIDAD:
        DrawCircleV((Vector2){cx, cy - 18.0f}, 6.0f, Fade(c, 0.18f));
        DrawCircleLines((int)cx, (int)(cy - 18.0f), 6.0f, c);
        DrawRectangleRounded((Rectangle){cx - 30.0f, cy + 6.0f, 12.0f, 12.0f}, 0.2f, 4,
                             Fade(c, 0.18f));
        DrawRectangleRounded((Rectangle){cx - 6.0f, cy + 6.0f, 12.0f, 12.0f}, 0.2f, 4,
                             Fade(c, 0.18f));
        DrawRectangleRounded((Rectangle){cx + 18.0f, cy + 6.0f, 12.0f, 12.0f}, 0.2f, 4,
                             Fade(c, 0.18f));
        DrawRectangleRoundedLinesEx((Rectangle){cx - 30.0f, cy + 6.0f, 12.0f, 12.0f}, 0.2f, 4,
                                    1.5f, c);
        DrawRectangleRoundedLinesEx((Rectangle){cx - 6.0f, cy + 6.0f, 12.0f, 12.0f}, 0.2f, 4,
                                    1.5f, c);
        DrawRectangleRoundedLinesEx((Rectangle){cx + 18.0f, cy + 6.0f, 12.0f, 12.0f}, 0.2f, 4,
                                    1.5f, c);
        DrawLineEx((Vector2){cx, cy - 12.0f}, (Vector2){cx - 24.0f, cy + 6.0f}, 1.6f, c);
        DrawLineEx((Vector2){cx, cy - 12.0f}, (Vector2){cx, cy + 6.0f}, 1.6f, c);
        DrawLineEx((Vector2){cx, cy - 12.0f}, (Vector2){cx + 24.0f, cy + 6.0f}, 1.6f, c);
        break;
    case ESTRUCTURA_LISTA_CIRCULAR:
        DrawRectangleRounded((Rectangle){cx - 28.0f, cy - 6.0f, 10.0f, 12.0f}, 0.2f, 4,
                             Fade(c, 0.18f));
        DrawRectangleRounded((Rectangle){cx - 8.0f, cy - 6.0f, 10.0f, 12.0f}, 0.2f, 4,
                             Fade(c, 0.18f));
        DrawRectangleRounded((Rectangle){cx + 12.0f, cy - 6.0f, 10.0f, 12.0f}, 0.2f, 4,
                             Fade(c, 0.18f));
        DrawRectangleRoundedLinesEx((Rectangle){cx - 28.0f, cy - 6.0f, 10.0f, 12.0f}, 0.2f, 4,
                                    1.6f, c);
        DrawRectangleRoundedLinesEx((Rectangle){cx - 8.0f, cy - 6.0f, 10.0f, 12.0f}, 0.2f, 4,
                                    1.6f, c);
        DrawRectangleRoundedLinesEx((Rectangle){cx + 12.0f, cy - 6.0f, 10.0f, 12.0f}, 0.2f, 4,
                                    1.6f, c);
        DrawLineEx((Vector2){cx - 18.0f, cy}, (Vector2){cx - 8.0f, cy}, 1.8f, c);
        DrawLineEx((Vector2){cx + 2.0f, cy}, (Vector2){cx + 12.0f, cy}, 1.8f, c);
        DrawLineEx((Vector2){cx + 17.0f, cy - 10.0f}, (Vector2){cx - 23.0f, cy - 10.0f}, 1.6f, c);
        DrawTriangle((Vector2){cx - 23.0f, cy - 10.0f}, (Vector2){cx - 18.0f, cy - 14.0f},
                     (Vector2){cx - 18.0f, cy - 6.0f}, c);
        break;
    case ESTRUCTURA_SUBLISTA:
        DrawRectangleRounded((Rectangle){cx - 30.0f, cy - 10.0f, 20.0f, 14.0f}, 0.2f, 4,
                             Fade(c, 0.18f));
        DrawRectangleRoundedLinesEx((Rectangle){cx - 30.0f, cy - 10.0f, 20.0f, 14.0f}, 0.2f, 4,
                                    1.6f, c);
        DrawLineEx((Vector2){cx - 10.0f, cy - 3.0f}, (Vector2){cx + 2.0f, cy - 3.0f}, 1.8f, c);
        DrawTriangle((Vector2){cx + 2.0f, cy - 3.0f}, (Vector2){cx - 3.0f, cy - 7.0f},
                     (Vector2){cx - 3.0f, cy + 1.0f}, c);
        DrawRectangleRounded((Rectangle){cx + 8.0f, cy - 16.0f, 12.0f, 10.0f}, 0.2f, 4,
                             Fade(c, 0.18f));
        DrawRectangleRounded((Rectangle){cx + 8.0f, cy - 2.0f, 12.0f, 10.0f}, 0.2f, 4,
                             Fade(c, 0.18f));
        DrawRectangleRounded((Rectangle){cx + 8.0f, cy + 12.0f, 12.0f, 10.0f}, 0.2f, 4,
                             Fade(c, 0.18f));
        DrawRectangleRoundedLinesEx((Rectangle){cx + 8.0f, cy - 16.0f, 12.0f, 10.0f}, 0.2f, 4,
                                    1.5f, c);
        DrawRectangleRoundedLinesEx((Rectangle){cx + 8.0f, cy - 2.0f, 12.0f, 10.0f}, 0.2f, 4,
                                    1.5f, c);
        DrawRectangleRoundedLinesEx((Rectangle){cx + 8.0f, cy + 12.0f, 12.0f, 10.0f}, 0.2f, 4,
                                    1.5f, c);
        break;
    case ESTRUCTURA_GRAFO:
        DrawCircleLines((int)(cx - 16.0f), (int)(cy - 12.0f), 6.0f, c);
        DrawCircleLines((int)(cx + 16.0f), (int)(cy - 8.0f), 6.0f, c);
        DrawCircleLines((int)cx, (int)(cy + 16.0f), 6.0f, c);
        DrawLineEx((Vector2){cx - 10.0f, cy - 9.0f}, (Vector2){cx + 10.0f, cy - 7.0f}, 1.8f, c);
        DrawLineEx((Vector2){cx - 12.0f, cy - 6.0f}, (Vector2){cx - 2.0f, cy + 10.0f}, 1.8f, c);
        DrawLineEx((Vector2){cx + 12.0f, cy - 4.0f}, (Vector2){cx + 2.0f, cy + 10.0f}, 1.8f, c);
        break;
    default:
        break;
    }
}

/** @brief Dibuja una tarjeta de estructura y retorna true si se presiona Visualizar. */
static bool draw_home_card(Rectangle card, const char *title, const char *description,
                           TipoEstructura tipo, bool is_selected) {
    Rectangle icon_circle = {card.x + card.width * 0.5f - 44.0f, card.y + 22.0f, 88.0f, 88.0f};
    bool compact_card = card.height < 300.0f;
    float separator_y = compact_card ? (card.y + 162.0f) : (card.y + 168.0f);
    float description_y = compact_card ? (card.y + 170.0f) : (card.y + 182.0f);
    Rectangle cta = {card.x + 28.0f,
                     card.y + card.height - (compact_card ? 50.0f : 62.0f),
                     card.width - 56.0f,
                     38.0f};
    int title_width = ui_measure_text(title, 18.0f, 0.16f, true);

    if (is_selected) {
        DrawRectangleRounded((Rectangle){card.x - 3.0f, card.y - 3.0f, card.width + 6.0f,
                                         card.height + 6.0f},
                             0.05f, 10, Fade((Color){66, 120, 190, 255}, 0.12f));
    }
    DrawRectangleRounded(card, 0.045f, 10, is_selected ? (Color){243, 249, 255, 255}
                                                        : (Color){248, 251, 255, 255});
    DrawRectangleRoundedLinesEx(card, 0.045f, 10, is_selected ? 2.2f : 1.2f,
                                Fade((Color){24, 92, 158, 255}, is_selected ? 0.55f : 0.22f));

    DrawEllipse((int)(icon_circle.x + icon_circle.width * 0.5f),
                (int)(icon_circle.y + icon_circle.height * 0.5f), icon_circle.width * 0.5f,
                icon_circle.height * 0.5f, (Color){232, 240, 250, 255});
    DrawEllipseLines((int)(icon_circle.x + icon_circle.width * 0.5f),
                     (int)(icon_circle.y + icon_circle.height * 0.5f), icon_circle.width * 0.5f,
                     icon_circle.height * 0.5f, Fade((Color){24, 92, 158, 255}, 0.35f));
    draw_home_icon(tipo, icon_circle);

    ui_draw_text(title, card.x + (card.width - title_width) * 0.5f, card.y + 132.0f, 18.0f,
                 0.16f, (Color){17, 52, 104, 255}, true);
    DrawLine((int)card.x + 20, (int)separator_y, (int)(card.x + card.width - 20),
             (int)separator_y, Fade((Color){44, 92, 153, 255}, 0.18f));

    draw_ui_multiline(description, card.x + 24.0f, description_y, 14.0f, 0.16f,
                      (Color){36, 44, 58, 255});

    return ui_button(cta, "Visualizar", false);
}

/** @brief Dibuja un boton compacto para el panel lateral de grafos. */
static bool draw_graph_sidebar_button(Rectangle bounds, const char *label, bool active) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, bounds);
    float label_size = bounds.height >= 34.0f ? 15.0f : 14.0f;
    float label_spacing = 0.10f;
    int label_width = ui_measure_text(label, label_size, label_spacing, false);
    Color bg = active ? (Color){223, 234, 246, 255} : (Color){248, 250, 253, 255};
    Color border = active ? (Color){10, 43, 92, 255} : (Color){160, 177, 196, 255};

    if (hover) {
        bg = (Color){233, 241, 249, 255};
    }

    DrawRectangleRounded(bounds, 0.20f, 8, bg);
    DrawRectangleRoundedLinesEx(bounds, 0.20f, 8, 1.8f, border);
    DrawRectangleRounded((Rectangle){bounds.x + 1.0f, bounds.y + 1.0f, 5.0f, bounds.height - 2.0f},
                         0.50f, 8, active ? (Color){198, 165, 102, 255}
                                            : Fade((Color){17, 69, 132, 255}, hover ? 0.42f : 0.16f));
    ui_draw_text(label, bounds.x + (bounds.width - label_width) * 0.5f,
                 bounds.y + (bounds.height - label_size) * 0.5f - 1.0f, label_size, label_spacing,
                 active ? (Color){10, 43, 92, 255} : (Color){38, 48, 61, 255}, false);

    return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

/** @brief Abre el visualizador dejando activa la estructura de grafos. */
static void open_graph_visualizer(AppState *app, ScreenMode *mode) {
    app_state_set_estructura(app, ESTRUCTURA_GRAFO);
    *mode = SCREEN_VISUALIZER;
}

/** @brief Traduce el enum de algoritmo de grafos a un nombre corto para la portada. */
static const char *grafo_algoritmo_home_nombre(int algoritmo) {
    switch (algoritmo) {
    case GRAFO_ALGO_BFS:
        return "BFS";
    case GRAFO_ALGO_DFS:
        return "DFS";
    case GRAFO_ALGO_DIJKSTRA:
        return "Dijkstra";
    case GRAFO_ALGO_BELLMAN_FORD:
        return "Bellman-Ford";
    case GRAFO_ALGO_PRIM:
        return "Prim";
    case GRAFO_ALGO_KRUSKAL:
        return "Kruskal";
    default:
        return "Ninguno";
    }
}

/** @brief Construye una cadena corta con el orden de vertices del recorrido actual. */
static void grafo_formatear_orden_vertices(const GrafoController *gc, char *out, size_t out_size) {
    int i;
    int count;
    size_t used;

    if (out == NULL || out_size == 0) {
        return;
    }
    out[0] = '\0';
    if (gc == NULL || gc->script_vertices_count <= 0) {
        snprintf(out, out_size, "-");
        return;
    }

    count = gc->script_vertices_count;
    used = 0;
    for (i = 0; i < count; i++) {
        char chunk[20];
        int w;

        if (i == 0) {
            w = snprintf(chunk, sizeof(chunk), "V%d", gc->script_vertices[i]);
        } else {
            w = snprintf(chunk, sizeof(chunk), " -> V%d", gc->script_vertices[i]);
        }
        if (w <= 0) {
            continue;
        }
        if (used + (size_t)w + 1 >= out_size) {
            if (used + 4 < out_size) {
                snprintf(out + used, out_size - used, " ...");
            }
            return;
        }
        snprintf(out + used, out_size - used, "%s", chunk);
        used += (size_t)w;
    }
}

/** @brief Alinea el algoritmo seleccionado con el modo visual de grafo activo. */
static void sync_grafo_algoritmo_por_modo(AppState *app) {
    if (app == NULL || app->estructura_activa != ESTRUCTURA_GRAFO) {
        return;
    }

    if (g_grafo_ui_mode == 1) {
        app->grafo_algoritmo_seleccionado = g_grafo_recorrido_algo;
    } else if (g_grafo_ui_mode == 2) {
        app->grafo_algoritmo_seleccionado = g_grafo_camino_algo;
    } else if (g_grafo_ui_mode == 3) {
        if (app->grafo_algoritmo_seleccionado != GRAFO_ALGO_PRIM &&
            app->grafo_algoritmo_seleccionado != GRAFO_ALGO_KRUSKAL) {
            app->grafo_algoritmo_seleccionado = GRAFO_ALGO_PRIM;
        }
    }
}

/** @brief Mensaje contextual al cambiar de modo para evitar errores heredados confusos. */
static void grafo_set_modo_contexto(AppState *app) {
    if (app == NULL || app->estructura_activa != ESTRUCTURA_GRAFO) {
        return;
    }

    sync_grafo_algoritmo_por_modo(app);
    app->operacion_actual = OPERACION_NINGUNA;
    app->ultima_operacion_ok = true;

    if (g_grafo_ui_mode == 0) {
        snprintf(app->mensaje_operacion, sizeof(app->mensaje_operacion),
                 "Modo Construccion: inicializa, crea vertices y luego aristas");
    } else if (g_grafo_ui_mode == 1) {
        snprintf(app->mensaje_operacion, sizeof(app->mensaje_operacion),
                 "Modo Recorridos: define inicio y ejecuta %s",
                 g_grafo_recorrido_algo == GRAFO_ALGO_BFS ? "BFS" : "DFS");
    } else if (g_grafo_ui_mode == 2) {
        snprintf(app->mensaje_operacion, sizeof(app->mensaje_operacion),
                 "Modo Caminos: define origen/destino y ejecuta %s",
                 g_grafo_camino_algo == GRAFO_ALGO_DIJKSTRA ? "Dijkstra" : "Bellman-Ford");
    } else {
        snprintf(app->mensaje_operacion, sizeof(app->mensaje_operacion),
                 "Modo MST: ejecuta Prim o Kruskal sobre grafo no dirigido");
    }
}

/** @brief Construye una lista multilinea numerada de vertices del recorrido. */
static void grafo_formatear_orden_vertices_lista(const GrafoController *gc, char *out,
                                                 size_t out_size, int max_items) {
    int i;
    int limite;
    size_t used = 0;

    if (out == NULL || out_size == 0) {
        return;
    }
    out[0] = '\0';
    if (gc == NULL || gc->script_vertices_count <= 0) {
        snprintf(out, out_size, "Sin recorrido");
        return;
    }

    limite = gc->script_vertices_count;
    if (max_items > 0 && limite > max_items) {
        limite = max_items;
    }

    for (i = 0; i < limite; i++) {
        char line[32];
        int w = snprintf(line, sizeof(line), "%d) V%d", i + 1, gc->script_vertices[i]);
        if (w <= 0) {
            continue;
        }
        if (used + (size_t)w + 2 >= out_size) {
            break;
        }
        if (used > 0) {
            out[used++] = '\n';
        }
        snprintf(out + used, out_size - used, "%s", line);
        used += (size_t)w;
    }

    if (gc->script_vertices_count > limite && used + 8 < out_size) {
        snprintf(out + used, out_size - used, "\n...");
    }
}

/** @brief Preselecciona o ejecuta un algoritmo de grafos desde la portada. */
static void launch_graph_algorithm_from_home(AppState *app, ScreenMode *mode, int algoritmo) {
    open_graph_visualizer(app, mode);
    app->grafo_algoritmo_seleccionado = algoritmo;

    if (grafo_orden(app->grafo) == 0) {
        snprintf(app->mensaje_operacion, sizeof(app->mensaje_operacion),
                 "Algoritmo %s seleccionado. Crea el grafo o carga una demo para ejecutarlo.",
                 grafo_algoritmo_home_nombre(algoritmo));
        return;
    }

    app_state_operacion_grafo_ejecutar_algoritmo(app, algoritmo, app->grafo_vertice_inicio,
                                                 app->grafo_vertice_destino);
}

/** @brief Configura una etapa de escenario visual para captura E2E. */
static void e2e_visual_configure_stage(E2EVisualMode *e2e, AppState *app, ScreenMode *screen_mode,
                                       int *home_selected, bool *home_activate) {
    if (e2e == NULL || app == NULL || screen_mode == NULL || home_selected == NULL ||
        home_activate == NULL) {
        return;
    }

    *home_activate = false;
    app->grafo_controller_state.autoplay_activo = false;

    switch (e2e->stage) {
    case 0:
        *screen_mode = SCREEN_HOME_ROOT;
        *home_selected = 1;
        break;
    case 1:
        *screen_mode = SCREEN_HOME_GRAFOS;
        *home_selected = 0;
        break;
    case 2:
        open_graph_visualizer(app, screen_mode);
        app_state_set_valor(app, 10);
        app_state_grafo_cargar_demo(app);
        g_grafo_ui_mode = 0;
        grafo_set_modo_contexto(app);
        break;
    case 3:
        open_graph_visualizer(app, screen_mode);
        app_state_set_valor(app, 10);
        app_state_grafo_cargar_demo(app);
        g_grafo_ui_mode = 1;
        g_grafo_recorrido_algo = GRAFO_ALGO_BFS;
        grafo_set_modo_contexto(app);
        app_state_operacion_grafo_ejecutar_algoritmo(app, GRAFO_ALGO_BFS, app->grafo_vertice_inicio,
                                                     app->grafo_vertice_destino);
        break;
    case 4:
        open_graph_visualizer(app, screen_mode);
        app_state_set_valor(app, 10);
        app_state_grafo_cargar_demo(app);
        g_grafo_ui_mode = 1;
        g_grafo_recorrido_algo = GRAFO_ALGO_DFS;
        grafo_set_modo_contexto(app);
        app_state_operacion_grafo_ejecutar_algoritmo(app, GRAFO_ALGO_DFS, app->grafo_vertice_inicio,
                                                     app->grafo_vertice_destino);
        break;
    case 5:
        open_graph_visualizer(app, screen_mode);
        app_state_set_valor(app, 10);
        app_state_grafo_cargar_demo(app);
        g_grafo_ui_mode = 2;
        g_grafo_camino_algo = GRAFO_ALGO_DIJKSTRA;
        grafo_set_modo_contexto(app);
        app_state_operacion_grafo_ejecutar_algoritmo(app, GRAFO_ALGO_DIJKSTRA,
                                                     app->grafo_vertice_inicio,
                                                     app->grafo_vertice_destino);
        break;
    case 6:
        open_graph_visualizer(app, screen_mode);
        app_state_set_valor(app, 10);
        app_state_grafo_cargar_demo(app);
        g_grafo_ui_mode = 2;
        g_grafo_camino_algo = GRAFO_ALGO_BELLMAN_FORD;
        grafo_set_modo_contexto(app);
        app_state_operacion_grafo_ejecutar_algoritmo(app, GRAFO_ALGO_BELLMAN_FORD,
                                                     app->grafo_vertice_inicio,
                                                     app->grafo_vertice_destino);
        break;
    case 7:
        open_graph_visualizer(app, screen_mode);
        app_state_set_valor(app, 10);
        app_state_grafo_cargar_demo(app);
        g_grafo_ui_mode = 3;
        app->grafo_algoritmo_seleccionado = GRAFO_ALGO_PRIM;
        grafo_set_modo_contexto(app);
        app_state_operacion_grafo_ejecutar_algoritmo(app, GRAFO_ALGO_PRIM,
                                                     app->grafo_vertice_inicio,
                                                     app->grafo_vertice_destino);
        break;
    case 8:
        open_graph_visualizer(app, screen_mode);
        app_state_set_valor(app, 10);
        app_state_grafo_cargar_demo(app);
        g_grafo_ui_mode = 3;
        app->grafo_algoritmo_seleccionado = GRAFO_ALGO_KRUSKAL;
        grafo_set_modo_contexto(app);
        app_state_operacion_grafo_ejecutar_algoritmo(app, GRAFO_ALGO_KRUSKAL,
                                                     app->grafo_vertice_inicio,
                                                     app->grafo_vertice_destino);
        break;
    default:
        break;
    }
}

/** @brief Captura y avanza etapas del modo E2E visual. Retorna true cuando termina. */
static bool e2e_visual_after_frame(E2EVisualMode *e2e, AppState *app, ScreenMode *screen_mode,
                                   int *home_selected, bool *home_activate) {
    char shot_path[512];

    if (e2e == NULL || !e2e->enabled || e2e->finished) {
        return false;
    }

    if (e2e->stage < 0) {
        e2e->stage = 0;
        e2e->frame_in_stage = 0;
        e2e_visual_configure_stage(e2e, app, screen_mode, home_selected, home_activate);
        return false;
    }

    if (e2e->frame_in_stage == e2e->capture_frame) {
        snprintf(shot_path, sizeof(shot_path), "%s/%02d_%s.png", e2e->output_dir, e2e->stage + 1,
                 e2e_visual_stage_name(e2e->stage));
        TakeScreenshot(shot_path);
        e2e->captures++;
    }

    e2e->frame_in_stage++;
    if (e2e->frame_in_stage > e2e->advance_frame) {
        e2e->stage++;
        if (e2e->stage >= E2E_VISUAL_STAGE_COUNT) {
            FILE *manifest;
            char manifest_path[512];
            snprintf(manifest_path, sizeof(manifest_path), "%s/manifest.txt", e2e->output_dir);
            manifest = fopen(manifest_path, "w");
            if (manifest != NULL) {
                fprintf(manifest, "captures=%d\n", e2e->captures);
                fprintf(manifest, "stages=%d\n", E2E_VISUAL_STAGE_COUNT);
                fprintf(manifest, "status=ok\n");
                fclose(manifest);
            }
            e2e->finished = true;
            return true;
        }
        e2e->frame_in_stage = 0;
        e2e_visual_configure_stage(e2e, app, screen_mode, home_selected, home_activate);
    }
    return false;
}

/** @brief Dibuja la portada raiz para elegir categoria de estructuras. */
static void draw_home_root_screen(const UILayout *layout, ScreenMode *mode, int *home_selected,
                                  bool activate_selected) {
    Rectangle content = {layout->sidebar.x, layout->sidebar.y,
                         layout->bottom.x + layout->bottom.width - layout->sidebar.x,
                         layout->bottom.y + layout->bottom.height - layout->sidebar.y};
    Rectangle cards_area = {content.x + 60.0f, content.y + 120.0f, content.width - 120.0f,
                            340.0f};
    float gap = 26.0f;
    float card_w = (cards_area.width - gap) * 0.5f;
    Rectangle card_seq = {cards_area.x, cards_area.y, card_w, cards_area.height};
    Rectangle card_grafos = {cards_area.x + card_w + gap, cards_area.y, card_w, cards_area.height};

    DrawRectangleRounded(content, 0.02f, 8, Fade(WHITE, 0.45f));
    ui_draw_text("VISUALSTRUCT V2", content.x + 40.0f, content.y + 26.0f, 34.0f, 0.12f,
                 (Color){20, 58, 112, 255}, true);
    ui_draw_text("Seleccione una familia para comenzar", content.x + 42.0f, content.y + 72.0f,
                 20.0f, 0.14f, (Color){53, 66, 83, 255}, false);
    ui_draw_text("Atajos: 1 Secuenciales | 2 Grafos | Enter seleccionar | F1 Ayuda",
                 content.x + 42.0f, content.y + 96.0f, 16.0f, 0.12f,
                 (Color){53, 66, 83, 255}, false);

    if (draw_home_card(card_seq, "SECUENCIALES",
                       "Pilas, colas, listas\nenlazadas y sublistas\npara operaciones basicas.",
                       ESTRUCTURA_LISTA, *home_selected == 0) ||
        (activate_selected && *home_selected == 0)) {
        *mode = SCREEN_HOME_SECUENCIALES;
        *home_selected = 0;
    }

    if (draw_home_card(card_grafos, "GRAFOS",
                       "Vertices, aristas y\nalgoritmos clasicos de\nrecorrido y caminos.",
                       ESTRUCTURA_GRAFO, *home_selected == 1) ||
        (activate_selected && *home_selected == 1)) {
        *mode = SCREEN_HOME_GRAFOS;
        *home_selected = 0;
    }
}

/** @brief Dibuja submenu para estructuras secuenciales. */
static void draw_home_screen(const UILayout *layout, AppState *app, ScreenMode *mode,
                             int *home_selected, bool activate_selected) {
    Rectangle content = {layout->sidebar.x, layout->sidebar.y,
                         layout->bottom.x + layout->bottom.width - layout->sidebar.x,
                         layout->bottom.y + layout->bottom.height - layout->sidebar.y};
    Rectangle cards_area = {content.x + 20.0f, content.y + 98.0f, content.width - 40.0f, 360.0f};
    Rectangle info_box = {content.x + 20.0f, cards_area.y + cards_area.height + 20.0f,
                          content.width - 40.0f, 78.0f};
    float gap = 14.0f;
    float card_w = (cards_area.width - gap * 5.0f) / 6.0f;
    Rectangle card_pila = {cards_area.x, cards_area.y, card_w, cards_area.height};
    Rectangle card_cola = {card_pila.x + card_w + gap, cards_area.y, card_w, cards_area.height};
    Rectangle card_lista = {card_cola.x + card_w + gap, cards_area.y, card_w, cards_area.height};
    Rectangle card_cp = {card_lista.x + card_w + gap, cards_area.y, card_w, cards_area.height};
    Rectangle card_lc = {card_cp.x + card_w + gap, cards_area.y, card_w, cards_area.height};
    Rectangle card_sub = {card_lc.x + card_w + gap, cards_area.y, card_w, cards_area.height};
    Rectangle nav_help = {content.x + 20.0f, content.y + 74.0f, content.width - 40.0f, 26.0f};
    int title_w = ui_measure_text("ESTRUCTURAS SECUENCIALES", 24.0f,
                                  0.12f, true);
    int subtitle_w = ui_measure_text("Seleccione la estructura secuencial que desea visualizar",
                                     16.0f, 0.14f, false);

    DrawRectangleRounded(content, 0.02f, 8, Fade(WHITE, 0.45f));
    ui_draw_text("ESTRUCTURAS SECUENCIALES",
                 content.x + (content.width - title_w) * 0.5f, content.y + 14.0f, 24.0f, 0.12f,
                 (Color){20, 58, 112, 255}, true);
    ui_draw_text("Seleccione la estructura secuencial que desea visualizar",
                 content.x + (content.width - subtitle_w) * 0.5f, content.y + 48.0f, 16.0f,
                 0.14f, (Color){53, 66, 83, 255}, false);
    DrawRectangleRounded(nav_help, 0.25f, 8, Fade((Color){220, 232, 247, 255}, 0.55f));
    ui_draw_text("Atajos: 1..6 seleccionar | ESC volver | F1 ayuda | Enter visualizar",
                 nav_help.x + 10.0f, nav_help.y + 5.0f, 13.0f, 0.14f,
                 (Color){36, 56, 84, 255}, false);

    if (draw_home_card(card_pila, "PILAS", estructura_descripcion(ESTRUCTURA_PILA),
                       ESTRUCTURA_PILA, *home_selected == 0) ||
        (activate_selected && *home_selected == 0)) {
        app_state_set_estructura(app, ESTRUCTURA_PILA);
        *mode = SCREEN_VISUALIZER;
    }
    if (draw_home_card(card_cola, "COLAS", estructura_descripcion(ESTRUCTURA_COLA),
                       ESTRUCTURA_COLA, *home_selected == 1) ||
        (activate_selected && *home_selected == 1)) {
        app_state_set_estructura(app, ESTRUCTURA_COLA);
        *mode = SCREEN_VISUALIZER;
    }
    if (draw_home_card(card_lista, "LISTAS ENLAZADAS", estructura_descripcion(ESTRUCTURA_LISTA),
                       ESTRUCTURA_LISTA, *home_selected == 2) ||
        (activate_selected && *home_selected == 2)) {
        app_state_set_estructura(app, ESTRUCTURA_LISTA);
        *mode = SCREEN_VISUALIZER;
    }
    if (draw_home_card(card_cp, "COLAS DE PRIORIDAD",
                       estructura_descripcion(ESTRUCTURA_COLA_PRIORIDAD),
                       ESTRUCTURA_COLA_PRIORIDAD, *home_selected == 3) ||
        (activate_selected && *home_selected == 3)) {
        app_state_set_estructura(app, ESTRUCTURA_COLA_PRIORIDAD);
        *mode = SCREEN_VISUALIZER;
    }
    if (draw_home_card(card_lc, "LISTA CIRCULAR",
                       estructura_descripcion(ESTRUCTURA_LISTA_CIRCULAR),
                       ESTRUCTURA_LISTA_CIRCULAR, *home_selected == 4) ||
        (activate_selected && *home_selected == 4)) {
        app_state_set_estructura(app, ESTRUCTURA_LISTA_CIRCULAR);
        *mode = SCREEN_VISUALIZER;
    }
    if (draw_home_card(card_sub, "SUBLISTAS", estructura_descripcion(ESTRUCTURA_SUBLISTA),
                       ESTRUCTURA_SUBLISTA, *home_selected == 5) ||
        (activate_selected && *home_selected == 5)) {
        app_state_set_estructura(app, ESTRUCTURA_SUBLISTA);
        *mode = SCREEN_VISUALIZER;
    }

    DrawRectangleRounded(info_box, 0.06f, 10, (Color){230, 239, 250, 255});
    DrawRectangleRoundedLinesEx(info_box, 0.06f, 10, 1.4f,
                                Fade((Color){42, 98, 158, 255}, 0.36f));
    ui_draw_text("Informacion", info_box.x + 30.0f, info_box.y + 24.0f, 14.0f, 0.14f,
                 (Color){20, 58, 112, 255}, true);
    ui_draw_text("Este visualizador permite explorar y comprender el comportamiento de",
                 info_box.x + 184.0f, info_box.y + 20.0f, 13.0f, 0.14f,
                 (Color){42, 50, 64, 255}, false);
    ui_draw_text("diferentes estructuras de datos mediante representaciones graficas.",
                 info_box.x + 184.0f, info_box.y + 40.0f, 13.0f, 0.14f,
                 (Color){42, 50, 64, 255}, false);
}

/** @brief Dibuja submenu de grafos y permite entrar al visualizador de grafo. */
static void draw_home_graph_screen(const UILayout *layout, AppState *app, ScreenMode *mode,
                                   int *home_selected, bool activate_selected) {
    Rectangle content = {layout->sidebar.x, layout->sidebar.y,
                         layout->bottom.x + layout->bottom.width - layout->sidebar.x,
                         layout->bottom.y + layout->bottom.height - layout->sidebar.y};
    Rectangle cards_area = {content.x + 44.0f, content.y + 132.0f, content.width - 88.0f, 278.0f};
    float gap = 24.0f;
    float card_w = (cards_area.width - gap) * 0.5f;
    Rectangle create_card = {cards_area.x, cards_area.y, card_w, cards_area.height};
    Rectangle demo_card = {cards_area.x + card_w + gap, cards_area.y, card_w, cards_area.height};
    Rectangle algo_box = {content.x + 44.0f, cards_area.y + cards_area.height + 18.0f,
                          content.width - 88.0f, 152.0f};
    Rectangle selected_box = {content.x + 44.0f, algo_box.y + algo_box.height + 16.0f,
                              content.width - 88.0f, 52.0f};
    static const char *algo_labels[6] = {"BFS", "DFS", "Dijkstra", "Bellman-Ford", "Prim",
                                         "Kruskal"};
    static const int algo_values[6] = {GRAFO_ALGO_BFS, GRAFO_ALGO_DFS, GRAFO_ALGO_DIJKSTRA,
                                       GRAFO_ALGO_BELLMAN_FORD, GRAFO_ALGO_PRIM,
                                       GRAFO_ALGO_KRUSKAL};
    int i;

    DrawRectangleRounded(content, 0.02f, 8, Fade(WHITE, 0.45f));
    ui_draw_text("MODULO DE GRAFOS", content.x + 40.0f, content.y + 24.0f, 30.0f, 0.12f,
                 (Color){20, 58, 112, 255}, true);
    ui_draw_text("Construya vertices/aristas y ejecute BFS, DFS, Dijkstra, Bellman-Ford, Prim y Kruskal",
                 content.x + 40.0f, content.y + 68.0f, 15.0f, 0.12f,
                 (Color){53, 66, 83, 255}, false);
    ui_draw_text("Atajos: 1 crear/editar | 2 demo | 4..9 algoritmos | ESC volver | F1 ayuda",
                 content.x + 40.0f, content.y + 92.0f, 14.0f, 0.12f,
                 (Color){53, 66, 83, 255}, false);

    if (draw_home_card(create_card, "CREAR O EDITAR", "Define vertices, aristas\ny pesos antes de\nejecutar el algoritmo.",
                       ESTRUCTURA_GRAFO, *home_selected == 0) ||
        (activate_selected && *home_selected == 0)) {
        open_graph_visualizer(app, mode);
    }

    if (draw_home_card(demo_card, "CARGAR DEMO", "Inserta un escenario\npreconfigurado para\nprobar recorridos y MST.",
                       ESTRUCTURA_GRAFO, *home_selected == 1) ||
        (activate_selected && *home_selected == 1)) {
        open_graph_visualizer(app, mode);
        app_state_grafo_cargar_demo(app);
    }

    DrawRectangleRounded(algo_box, 0.14f, 8, (Color){230, 239, 250, 255});
    DrawRectangleRoundedLinesEx(algo_box, 0.14f, 8, 1.3f, Fade((Color){42, 98, 158, 255}, 0.36f));
    ui_draw_text("Seleccion rapida de algoritmos", algo_box.x + 16.0f, algo_box.y + 14.0f,
                 16.0f, 0.12f, (Color){20, 58, 112, 255}, true);
    ui_draw_text("Si el grafo ya existe, el algoritmo se ejecuta de inmediato. Si no, queda preseleccionado.",
                 algo_box.x + 16.0f, algo_box.y + 38.0f, 13.0f, 0.12f,
                 (Color){42, 50, 64, 255}, false);

    for (i = 0; i < 6; i++) {
        Rectangle btn = {algo_box.x + 16.0f + (float)(i % 3) * ((algo_box.width - 48.0f) / 3.0f + 8.0f),
                         algo_box.y + 68.0f + (float)(i / 3) * 42.0f,
                         (algo_box.width - 48.0f) / 3.0f,
                         34.0f};
        if (ui_button(btn, algo_labels[i], app->grafo_algoritmo_seleccionado == algo_values[i])) {
            launch_graph_algorithm_from_home(app, mode, algo_values[i]);
        }
    }

    DrawRectangleRounded(selected_box, 0.20f, 8, Fade((Color){220, 232, 247, 255}, 0.72f));
    ui_draw_text("Seleccion actual:", selected_box.x + 16.0f, selected_box.y + 16.0f,
                 14.0f, 0.12f, (Color){36, 56, 84, 255}, true);
    ui_draw_text(grafo_algoritmo_home_nombre(app->grafo_algoritmo_seleccionado),
                 selected_box.x + 168.0f, selected_box.y + 16.0f, 14.0f, 0.12f,
                 (Color){20, 58, 112, 255}, true);
    ui_draw_text(grafo_orden(app->grafo) > 0 ? "Estado: hay un grafo listo para ejecutar." :
                                      "Estado: aun no hay vertices; puedes crearlo o cargar una demo.",
                 selected_box.x + 300.0f, selected_box.y + 16.0f, 13.0f, 0.10f,
                 (Color){42, 50, 64, 255}, false);
}

/** @brief Gestiona navegación de alto nivel entre menu principal y visualizador. */
static void handle_navigation_keyboard(ScreenMode *mode, AppState *app, int *home_selected,
                                       bool *activate_selected, InputFocus focus) {
    *activate_selected = false;
    if (*mode == SCREEN_HELP) {
        return;
    }

    if (*mode == SCREEN_HOME_ROOT) {
        if (IsKeyPressed(KEY_ONE)) {
            *mode = SCREEN_HOME_SECUENCIALES;
            *home_selected = 0;
            return;
        }
        if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_SEVEN)) {
            *mode = SCREEN_HOME_GRAFOS;
            *home_selected = 0;
            return;
        }
        if (IsKeyPressed(KEY_RIGHT)) {
            *home_selected = (*home_selected + 1) % 2;
        }
        if (IsKeyPressed(KEY_LEFT)) {
            *home_selected = (*home_selected + 1) % 2;
        }
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE)) {
            *activate_selected = true;
        }
        return;
    }

    if (*mode == SCREEN_HOME_SECUENCIALES) {
        int estructura_shortcut = estructura_from_shortcut();

        if (estructura_shortcut >= ESTRUCTURA_PILA && estructura_shortcut <= ESTRUCTURA_SUBLISTA) {
            app_state_set_estructura(app, (TipoEstructura)estructura_shortcut);
            *home_selected = estructura_shortcut;
            *mode = SCREEN_VISUALIZER;
            return;
        }
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_H)) {
            *mode = SCREEN_HOME_ROOT;
            *home_selected = 0;
            return;
        }
        if (IsKeyPressed(KEY_RIGHT)) {
            *home_selected = (*home_selected + 1) % 6;
        }
        if (IsKeyPressed(KEY_LEFT)) {
            *home_selected = (*home_selected + 5) % 6;
        }
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE)) {
            *activate_selected = true;
        }
        return;
    }

    if (*mode == SCREEN_HOME_GRAFOS) {
        if (IsKeyPressed(KEY_ONE) || ((IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) ||
            IsKeyPressed(KEY_SPACE)) && *home_selected == 0)) {
            open_graph_visualizer(app, mode);
            return;
        }
        if (IsKeyPressed(KEY_TWO)) {
            open_graph_visualizer(app, mode);
            app_state_grafo_cargar_demo(app);
            return;
        }
        if (IsKeyPressed(KEY_FOUR)) {
            launch_graph_algorithm_from_home(app, mode, GRAFO_ALGO_BFS);
            return;
        }
        if (IsKeyPressed(KEY_FIVE)) {
            launch_graph_algorithm_from_home(app, mode, GRAFO_ALGO_DFS);
            return;
        }
        if (IsKeyPressed(KEY_SIX)) {
            launch_graph_algorithm_from_home(app, mode, GRAFO_ALGO_DIJKSTRA);
            return;
        }
        if (IsKeyPressed(KEY_SEVEN)) {
            launch_graph_algorithm_from_home(app, mode, GRAFO_ALGO_BELLMAN_FORD);
            return;
        }
        if (IsKeyPressed(KEY_EIGHT)) {
            launch_graph_algorithm_from_home(app, mode, GRAFO_ALGO_PRIM);
            return;
        }
        if (IsKeyPressed(KEY_NINE)) {
            launch_graph_algorithm_from_home(app, mode, GRAFO_ALGO_KRUSKAL);
            return;
        }
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_H)) {
            *mode = SCREEN_HOME_ROOT;
            *home_selected = 1;
            return;
        }
        if (IsKeyPressed(KEY_RIGHT)) {
            *home_selected = (*home_selected + 1) % 2;
        }
        if (IsKeyPressed(KEY_LEFT)) {
            *home_selected = (*home_selected + 1) % 2;
        }
    }

    if (*mode == SCREEN_VISUALIZER) {
        if (focus != INPUT_NONE) {
            return;
        }

        int estructura_shortcut = estructura_from_shortcut();

        if (estructura_shortcut >= 0) {
            app_state_set_estructura(app, (TipoEstructura)estructura_shortcut);
            return;
        }

        if (IsKeyPressed(KEY_H) || IsKeyPressed(KEY_ESCAPE)) {
            *home_selected = (app->estructura_activa == ESTRUCTURA_GRAFO) ? 1 : 0;
            *mode = SCREEN_HOME_ROOT;
            return;
        }

        if (IsKeyPressed(KEY_TAB)) {
            TipoEstructura next = (TipoEstructura)((app->estructura_activa + 1) % 7);
            app_state_set_estructura(app, next);
        }
    }
}

/** @brief Cuenta lineas de un bloque de texto multilínea. */
static int count_text_lines(const char *text) {
    int lines = 1;

    if (text == NULL || text[0] == '\0') {
        return 0;
    }

    while (*text != '\0') {
        if (*text == '\n') {
            lines++;
        }
        text++;
    }
    return lines;
}

/** @brief Retorna la complejidad temporal del algoritmo de grafo activo. */
static const char *grafo_algoritmo_tiempo(int algoritmo) {
    switch (algoritmo) {
    case GRAFO_ALGO_BFS:
    case GRAFO_ALGO_DFS:
        return "O(V + E)";
    case GRAFO_ALGO_DIJKSTRA:
        return "O(V^2 + E*V)";
    case GRAFO_ALGO_BELLMAN_FORD:
        return "O(V*E)";
    case GRAFO_ALGO_PRIM:
        return "O(V^2 + E*V)";
    case GRAFO_ALGO_KRUSKAL:
        return "O(E log E)";
    default:
        return "N/A";
    }
}

/** @brief Retorna la complejidad espacial del algoritmo de grafo activo. */
static const char *grafo_algoritmo_espacio(int algoritmo) {
    switch (algoritmo) {
    case GRAFO_ALGO_NINGUNO:
        return "O(V + E)";
    default:
        return "O(V)";
    }
}

/** @brief Obtiene el bloque de pseudocodigo para el algoritmo de grafo activo. */
static GrafoCodigoAlgoritmo grafo_codigo_actual(int algoritmo) {
    switch (algoritmo) {
    case GRAFO_ALGO_BFS:
        return grafo_codigo_bfs();
    case GRAFO_ALGO_DFS:
        return grafo_codigo_dfs();
    case GRAFO_ALGO_DIJKSTRA:
        return grafo_codigo_dijkstra();
    case GRAFO_ALGO_BELLMAN_FORD:
        return grafo_codigo_bellman_ford();
    case GRAFO_ALGO_PRIM:
        return grafo_codigo_prim();
    case GRAFO_ALGO_KRUSKAL:
        return grafo_codigo_kruskal();
    default:
        return grafo_codigo_bfs();
    }
}

/** @brief Limita un valor flotante a un rango cerrado. */
static float clamp_float(float value, float min, float max) {
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

/** @brief Retorna la cantidad de elementos de la estructura actualmente seleccionada. */
static int estructura_cantidad(const AppState *state) {
    int total;
    Nodo *padre;

    if (state == NULL) {
        return 0;
    }

    switch (state->estructura_activa) {
    case ESTRUCTURA_PILA:
        return pila_contar(&state->pila);
    case ESTRUCTURA_COLA:
        return cola_contar(&state->cola);
    case ESTRUCTURA_COLA_PRIORIDAD:
        return cp_contar(&state->cola_prioridad);
    case ESTRUCTURA_LISTA:
        return lista_contar(&state->lista);
    case ESTRUCTURA_LISTA_CIRCULAR:
        return lcir_contar(&state->lista_circular);
    case ESTRUCTURA_SUBLISTA:
        total = 0;
        padre = state->sublista;
        while (padre != NULL) {
            total += 1 + sublista_contar_hijos(padre);
            padre = padre->sgte;
        }
        return total;
    case ESTRUCTURA_GRAFO:
        return (int)grafo_orden(state->grafo);
    default:
        return 0;
    }
}

/** @brief Dibuja la barra de scroll proporcional para paneles textuales. */
static void draw_scrollbar(Rectangle track, float content_height, float viewport_height,
                           float offset) {
    Rectangle thumb;
    float max_scroll;
    float thumb_height;
    float t;

    DrawRectangleRounded(track, 0.30f, 8, (Color){222, 231, 239, 255});
    max_scroll = content_height - viewport_height;
    if (max_scroll <= 0.0f) {
        return;
    }

    thumb_height = track.height * (viewport_height / content_height);
    if (thumb_height < 22.0f) {
        thumb_height = 22.0f;
    }

    t = (max_scroll > 0.0f) ? (offset / max_scroll) : 0.0f;
    thumb = (Rectangle){track.x + 2.0f, track.y + 2.0f + (track.height - thumb_height - 4.0f) * t,
                        track.width - 4.0f, thumb_height};
    DrawRectangleRounded(thumb, 0.35f, 8, (Color){119, 148, 176, 255});
}

/** @brief Dibuja texto multilínea dentro de un viewport con clipping y desplazamiento vertical. */
static void draw_scrollable_multiline_text(const char *text, Rectangle viewport, int font_size,
                                           Color color, float offset_y) {
    const char *start = text;
    const char *line_end;
    char line[256];
    int line_index = 0;
    int render_size = font_size < 12 ? 12 : font_size;
    int line_height = render_size + 7;
    size_t len;

    if (text == NULL) {
        return;
    }

    BeginScissorMode((int)viewport.x, (int)viewport.y, (int)viewport.width, (int)viewport.height);
    while (*start != '\0') {
        int y;

        line_end = strchr(start, '\n');
        len = (line_end == NULL) ? strlen(start) : (size_t)(line_end - start);
        if (len >= sizeof(line)) {
            len = sizeof(line) - 1;
        }
        memcpy(line, start, len);
        line[len] = '\0';

        y = (int)(viewport.y - offset_y) + line_index * line_height;
        if (y + line_height >= (int)viewport.y - line_height && y <= (int)(viewport.y + viewport.height)) {
            ui_draw_text(line, viewport.x, (float)y, (float)render_size, 0.10f, color, false);
        }

        line_index++;
        if (line_end == NULL) {
            break;
        }
        start = line_end + 1;
    }
    EndScissorMode();
}

static const char *APP_HELP_TEXT_PART1 =
    "BIENVENIDO A LA AYUDA COMPLETA DE VISUALSTRUCT\n"
    "\n"
    "Esta seccion explica en detalle como funciona la aplicacion, tanto para estudiantes\n"
    "como para docentes o personas que mantienen el codigo.\n"
    "Puedes recorrerla de arriba hacia abajo como una mini-guia de uso y arquitectura.\n"
    "\n"
    "============================================================\n"
    "A) VISTA GENERAL DE LA APP\n"
    "============================================================\n"
    "VisualStruct combina tres perspectivas al mismo tiempo:\n"
    "1. La perspectiva visual: como cambia la estructura en pantalla.\n"
    "2. La perspectiva de codigo: que instrucciones en C representan la operacion.\n"
    "3. La perspectiva algoritmica: que pasos ocurren y cual es su complejidad.\n"
    "\n"
    "Con esta combinacion, cada accion no solo se ejecuta: tambien se explica.\n"
    "\n"
    "============================================================\n"
    "B) MODULO DE INICIO: MENU PRINCIPAL\n"
    "============================================================\n"
    "En el menu principal eliges la estructura a estudiar:\n"
    "- Pila\n"
    "- Cola\n"
    "- Lista Enlazada\n"
    "- Cola de Prioridad\n"
    "- Lista Circular\n"
    "- Sublistas\n"
    "\n"
    "Puedes elegir con mouse o teclado:\n"
    "- Teclas 1..6 para seleccion rapida.\n"
    "- Flechas izquierda/derecha para mover el foco.\n"
    "- Enter o Espacio para entrar al visualizador.\n"
    "\n"
    "Objetivo pedagogico de este modulo:\n"
    "- Comenzar una sesion desde una estructura concreta.\n"
    "- Evitar confusiones cuando se trabaja con varios tipos de TAD.\n"
    "\n"
    "============================================================\n"
    "C) MODULO DE NAVEGACION GLOBAL\n"
    "============================================================\n"
    "Atajos de navegacion utiles:\n"
    "- F1: abre o cierra esta ayuda desde cualquier pantalla.\n"
    "- H o ESC: vuelve al menu principal desde el visualizador.\n"
    "- TAB: rota entre estructuras en modo visualizador.\n"
    "\n"
    "Consejo de uso:\n"
    "- Si estas explicando una clase, deja F1 como referencia rapida.\n"
    "- Si pierdes contexto, usa H para regresar al menu y reiniciar el flujo.\n"
    "\n"
    "============================================================\n"
    "D) MODULO DE ESTADO (APP STATE)\n"
    "============================================================\n"
    "Este es el nucleo logico de la aplicacion.\n"
    "Responsabilidades principales:\n"
    "- Guardar estructura activa, operacion actual y ultimo resultado.\n"
    "- Validar entradas y reglas de negocio.\n"
    "- Ejecutar operaciones sobre los TAD (push, pop, encolar, etc.).\n"
    "- Actualizar mensajes de estado y activar animaciones de feedback.\n"
    "- Incrementar un serial para registrar cada operacion ejecutada.\n"
    "\n"
    "Por que es importante:\n"
    "- Evita que la interfaz manipule nodos directamente.\n"
    "- Mantiene separadas la logica y la presentacion.\n"
    "\n"
    "============================================================\n"
    "E) MODULO DE CONTROLES DE OPERACION\n"
    "============================================================\n"
    "Los botones del panel central cambian segun la estructura activa.\n"
    "\n"
    "Para Pila, Cola y Cola de Prioridad:\n"
    "- Inicializar\n"
    "- Insertar/Encolar\n"
    "- Eliminar/Desencolar\n"
    "- Vaciar\n"
    "\n"
    "Para Lista Enlazada y Lista Circular:\n"
    "- Inicializar\n"
    "- Insertar al Inicio\n"
    "- Insertar al Final\n"
    "- Buscar\n"
    "- Eliminar\n"
    "- Invertir\n"
    "- Vaciar\n"
    "\n"
    "Para Sublistas:\n"
    "- Inicializar\n"
    "- Insertar Padre\n"
    "- Buscar/Seleccionar Padre\n"
    "- Eliminar Padre\n"
    "- Insertar Hijo (requiere padre activo)\n"
    "- Eliminar Hijo (requiere padre activo)\n"
    "- Vaciar\n"
    "\n"
    "Buenas practicas didacticas:\n"
    "- Inicializa antes de una demostracion nueva.\n"
    "- Ejecuta secuencias cortas de 3 a 6 pasos.\n"
    "- Pide predicciones antes de ejecutar cada boton.\n"
    "\n"
    "============================================================\n"
    "F) MODULO DE ENTRADA DE DATOS\n"
    "============================================================\n"
    "Campo Valor:\n"
    "- Acepta enteros (positivos o negativos).\n"
    "- Se usa para insertar, buscar y eliminar segun el modulo.\n"
    "\n"
    "Campo Prioridad:\n"
    "- Solo aplica a Cola de Prioridad.\n"
    "- Rango permitido: 1..99.\n"
    "- Menor numero = mayor prioridad de salida.\n"
    "\n"
    "Validaciones visuales:\n"
    "- Si escribes un dato invalido, la caja se marca en rojo.\n"
    "- Enter confirma el contenido valido del campo enfocado.\n"
    "\n"
    "============================================================\n";

static const char *APP_HELP_TEXT_PART1B =
    "============================================================\n"
    "G) MODULO DE VISTA GRAFICA\n"
    "============================================================\n"
    "La vista grafica representa el estado real de la estructura activa.\n"
    "No inventa datos: dibuja snapshots obtenidos desde los TAD.\n"
    "\n"
    "Indicadores frecuentes:\n"
    "- NEW: insercion reciente.\n"
    "- POP / OUT / DEL: eliminacion reciente.\n"
    "- FRONT/BACK en cola, HEAD/NULL en lista.\n"
    "\n"
    "Navegacion dentro de la vista:\n"
    "- Pila: scroll vertical con rueda del mouse.\n"
    "- Cola, Lista, Lista Circular y Cola de Prioridad: scroll horizontal.\n"
    "- Sublistas: scroll vertical para recorrer padres e hijos.\n"
    "- Grafo: usa el panel inferior para seguir tipo de paso, tabla de distancias y camino parcial.\n"
    "- En listas largas, usa el scroll para inspeccionar extremos.\n"
    "\n"
    "============================================================\n";

static const char *APP_HELP_TEXT_PART2 =
    "H) MODULO DE CODIGO C ASOCIADO (HISTORIAL)\n"
    "============================================================\n"
    "Este panel registra una entrada por cada operacion ejecutada.\n"
    "Cada entrada incluye:\n"
    "- Numero de paso.\n"
    "- Estructura activa.\n"
    "- Operacion realizada.\n"
    "- Estado (OK o Error).\n"
    "- Mensaje resultante.\n"
    "- Snippet C representativo.\n"
    "\n"
    "Para que sirve:\n"
    "- Reconstruir toda la sesion paso a paso.\n"
    "- Relacionar acciones de UI con instrucciones de C.\n"
    "- Comparar secuencias entre distintos escenarios.\n"
    "\n"
    "Boton Limpiar:\n"
    "- Borra el historial acumulado.\n"
    "- Ideal para iniciar una nueva practica sin ruido previo.\n"
    "\n"
    "============================================================\n"
    "I) MODULO DE TRAZA Y COMPLEJIDAD\n"
    "============================================================\n"
    "Este panel explica que ocurre por dentro de cada operacion.\n"
    "Contenido esperado:\n"
    "- Descripcion corta de la operacion ejecutada.\n"
    "- Pasos del algoritmo en lenguaje comprensible.\n"
    "- Complejidad temporal y espacial.\n"
    "\n"
    "Uso recomendado:\n"
    "- Primero ejecuta la accion.\n"
    "- Luego compara el resultado visual con la traza.\n"
    "- Finalmente conecta eso con la complejidad reportada.\n"
    "- En Grafos, observa tambien el tipo de paso, camino parcial, cerrados y metricas por iteracion.\n"
    "\n"
    "============================================================\n"
    "J) ATAJOS DE TECLADO\n"
    "============================================================\n"
    "Operaciones:\n"
    "- I: inicializar estructura activa.\n"
    "- A: insertar (o final en listas, padre en sublistas).\n"
    "- Z: insertar al inicio (listas) o insertar hijo (sublistas).\n"
    "- D: eliminar.\n"
    "- B: buscar (listas) o seleccionar padre (sublistas).\n"
    "- R: invertir (listas) o eliminar hijo (sublistas).\n"
    "- V: vaciar estructura activa.\n"
    "- P (grafo): autoplay on/off.\n"
    "- O (grafo): velocidad autoplay.\n"
    "- T (grafo): dirigido/no dirigido.\n"
    "- M (grafo): cargar demo.\n"
    "- C (grafo): exportar resumen al portapapeles.\n"
    "- HOME/END (grafo): saltar al inicio o final del algoritmo.\n"
    "\n"
    "Entrada numerica:\n"
    "- UP/DOWN: ajustar valor.\n"
    "- LEFT/RIGHT: ajustar prioridad (cola de prioridad).\n"
    "- ENTER: confirmar campo editado.\n"
    "\n"
    "============================================================\n";

static const char *APP_HELP_TEXT_PART3 =
    "K) ERRORES COMUNES Y COMO INTERPRETARLOS\n"
    "============================================================\n"
    "1. \"Error\" tras eliminar:\n"
    "- Suele ocurrir cuando intentas eliminar en una estructura vacia.\n"
    "- Solucion: insertar primero o inicializar y repetir secuencia.\n"
    "\n"
    "2. Prioridad invalida:\n"
    "- La prioridad debe estar en 1..99.\n"
    "- Revisa el campo y confirma con Enter.\n"
    "\n"
    "3. No encuentro un nodo en pantalla:\n"
    "- Puede estar fuera del area visible.\n"
    "- Usa el scroll del panel grafico para desplazarte.\n"
    "\n"
    "4. Historial muy largo:\n"
    "- Usa Limpiar para iniciar un nuevo bloque de trabajo.\n"
    "\n"
    "============================================================\n";

static const char *APP_HELP_TEXT_PART4 =
    "L) ARQUITECTURA TECNICA (RESUMEN)\n"
    "============================================================\n"
    "Archivos principales:\n"
    "- src/main.c: ciclo principal, eventos, navegacion y render.\n"
    "- src/app_state.c: reglas de negocio y despacho de operaciones.\n"
    "- src/ui.c: widgets, layout y estilo visual compartido.\n"
    "- src/*_view.c: dibujo especifico por estructura.\n"
    "- src/code_viewer.c: snippets C por operacion.\n"
    "- src/algorithm_trace.c: texto de traza y complejidades.\n"
    "- src/pila.c, cola.c, cola_prioridad.c, lista.c, lista_circular.c, sublista.c: TAD puros en C.\n"
    "\n"
    "Principio clave de diseno:\n"
    "- La UI no altera nodos directamente.\n"
    "- Toda modificacion pasa por AppState y por la API publica de los TAD.\n"
    "\n"
    "============================================================\n"
    "M) PROPUESTA DE SESION DE ESTUDIO (10-15 MIN)\n"
    "============================================================\n"
    "1. Elige una estructura y pulsa Inicializar.\n"
    "2. Inserta 3 valores y anticipa el resultado de eliminar.\n"
    "3. Ejecuta eliminar y verifica grafico + historial + traza.\n"
    "4. Repite con valores diferentes.\n"
    "5. Limpia historial y cambia de estructura.\n"
    "6. Compara diferencias de comportamiento y complejidad.\n"
    "\n"
    "CIERRE\n"
    "Si usas esta ayuda como referencia durante la practica,\n"
    "podras entender no solo que hace la app, sino por que lo hace asi.\n";

/** @brief Concatena los bloques de ayuda para evitar literales demasiado largos en compilacion. */
static const char *get_app_help_text(void) {
    static char full_text[12000];
    static bool initialized = false;

    if (!initialized) {
        snprintf(full_text, sizeof(full_text), "%s%s%s%s%s", APP_HELP_TEXT_PART1,
                 APP_HELP_TEXT_PART1B, APP_HELP_TEXT_PART2, APP_HELP_TEXT_PART3,
                 APP_HELP_TEXT_PART4);
        initialized = true;
    }
    return full_text;
}

/** @brief Dibuja la pantalla de ayuda detallada y retorna true si se solicita salir. */
static bool draw_help_screen(const UILayout *layout, float *help_scroll) {
    const char *help_text = get_app_help_text();
    Rectangle content = {layout->sidebar.x, layout->sidebar.y,
                         layout->bottom.x + layout->bottom.width - layout->sidebar.x,
                         layout->bottom.y + layout->bottom.height - layout->sidebar.y};
    Rectangle top_band = {content.x + 20.0f, content.y + 18.0f, content.width - 40.0f, 70.0f};
    Rectangle viewport = {content.x + 26.0f, content.y + 104.0f, content.width - 60.0f,
                          content.height - 128.0f};
    Rectangle back_btn = {top_band.x + top_band.width - 132.0f, top_band.y + 18.0f, 112.0f, 34.0f};
    int lines = count_text_lines(help_text);
    float content_height = lines * 22.0f;
    float max_scroll = content_height - viewport.height;

    if (max_scroll < 0.0f) {
        max_scroll = 0.0f;
    }

    if (CheckCollisionPointRec(GetMousePosition(), viewport) && GetMouseWheelMove() != 0.0f) {
        *help_scroll -= GetMouseWheelMove() * 24.0f;
    }
    *help_scroll = clamp_float(*help_scroll, 0.0f, max_scroll);

    DrawRectangleRounded(content, 0.02f, 10, Fade(WHITE, 0.55f));
    DrawRectangleRounded(top_band, 0.18f, 10, (Color){229, 239, 250, 255});
    DrawRectangleRoundedLinesEx(top_band, 0.18f, 10, 1.6f, Fade((Color){42, 98, 158, 255}, 0.40f));
    ui_draw_text("AYUDA DETALLADA DE VISUALSTRUCT", top_band.x + 16.0f, top_band.y + 12.0f, 23.0f,
                 0.10f, (Color){22, 58, 108, 255}, true);
    ui_draw_text("Explicacion funcional y tecnica de cada modulo de la app",
                 top_band.x + 16.0f, top_band.y + 40.0f, 14.0f, 0.12f, (Color){52, 66, 84, 255},
                 false);

    if (ui_button(back_btn, "Volver", false)) {
        return true;
    }

    DrawRectangleRounded((Rectangle){viewport.x - 8.0f, viewport.y - 8.0f, viewport.width + 16.0f,
                                     viewport.height + 16.0f},
                         0.08f, 8, Fade((Color){236, 244, 252, 255}, 0.72f));
    DrawRectangleRoundedLinesEx((Rectangle){viewport.x - 8.0f, viewport.y - 8.0f,
                                            viewport.width + 16.0f, viewport.height + 16.0f},
                                0.08f, 8, 1.2f, Fade((Color){42, 98, 158, 255}, 0.25f));
    draw_scrollable_multiline_text(help_text, viewport, 16, (Color){39, 51, 66, 255},
                                   *help_scroll);
    draw_scrollbar((Rectangle){viewport.x + viewport.width + 6.0f, viewport.y, 8.0f, viewport.height},
                   content_height, viewport.height, *help_scroll);
    ui_draw_text("Tip: F1 abre/cierra esta ayuda desde cualquier pantalla", content.x + 24.0f,
                 content.y + content.height - 18.0f, 12.0f, 0.10f, (Color){71, 86, 104, 255}, false);
    return false;
}

/** @brief Despacha el render de la vista grafica segun la estructura seleccionada. */
static void draw_active_view(AppState *state, Rectangle panel, float content_top_y) {
    float switch_fx = state->animacion_cambio_estructura;
    float pulse_fx = state->animacion_pulso_panel;
    float switch_smooth = switch_fx * switch_fx * (3.0f - 2.0f * switch_fx);
    float pulse_smooth = pulse_fx * pulse_fx * (3.0f - 2.0f * pulse_fx);
    float max_height = panel.y + panel.height - 12.0f - content_top_y;
    Rectangle area = {panel.x + 12.0f, content_top_y, panel.width - 24.0f, max_height};
    if (area.height < 60.0f) {
        area.height = 60.0f;
    }

    if (switch_smooth > 0.0f) {
        area.x += 20.0f * switch_smooth;
    }

    switch (state->estructura_activa) {
    case ESTRUCTURA_PILA:
        pila_view_draw(state, area);
        break;
    case ESTRUCTURA_COLA:
        cola_view_draw(state, area);
        break;
    case ESTRUCTURA_COLA_PRIORIDAD:
        cola_prioridad_view_draw(state, area);
        break;
    case ESTRUCTURA_LISTA:
        lista_view_draw(state, area);
        break;
    case ESTRUCTURA_LISTA_CIRCULAR:
        lista_circular_view_draw(state, area);
        break;
    case ESTRUCTURA_SUBLISTA:
        sublista_view_draw(state, area);
        break;
    case ESTRUCTURA_GRAFO:
        grafo_controller_actualizar_area(&state->grafo_controller_state, area);
        grafo_controller_dibujar(&state->grafo_controller_state);
        break;
    default:
        break;
    }

    if (switch_smooth > 0.0f) {
        DrawRectangleRounded(area, 0.03f, 8, Fade(WHITE, 0.28f * switch_smooth));
    }
    if (pulse_smooth > 0.0f) {
        DrawRectangleRoundedLinesEx(area, 0.03f, 8, 2.0f + 1.6f * pulse_smooth,
                                    Fade((Color){52, 132, 82, 255}, 0.20f * pulse_smooth));
    }
}

/** @brief Dibuja los controles contextuales de operacion sobre el panel central. */
static float draw_context_controls(AppState *app, Rectangle panel, bool *is_compact_mode) {
    float base_x = panel.x + 16.0f;
    float base_y = panel.y + 36.0f;
    float gap = 10.0f;
    float btn_h = 36.0f;
    float row_step = btn_h + 6.0f;
    Vector2 dpi_scale = GetWindowScaleDPI();
    float dpi_factor = dpi_scale.x > dpi_scale.y ? dpi_scale.x : dpi_scale.y;
    float compact_threshold;
    bool compact;
    int columns = 5;
    float btn_w = (panel.width - 32.0f - gap * (columns - 1)) / columns;
    int count = 1;
    int i;
    float hints_y;
    const char *hint = "Atajos";

    if (btn_w < 112.0f) {
        btn_w = 112.0f;
    }

    if (dpi_factor < 1.0f) {
        dpi_factor = 1.0f;
    }
    compact_threshold = 820.0f * dpi_factor;
    compact = panel.width < compact_threshold;
    columns = compact ? 3 : 5;
    if (is_compact_mode != NULL) {
        *is_compact_mode = compact;
    }

    if (app->estructura_activa == ESTRUCTURA_GRAFO) {
        sync_grafo_algoritmo_por_modo(app);
        if (g_grafo_ui_mode != 2 &&
            (app->grafo_algoritmo_seleccionado == GRAFO_ALGO_DIJKSTRA ||
             app->grafo_algoritmo_seleccionado == GRAFO_ALGO_BELLMAN_FORD)) {
            g_grafo_camino_algo = app->grafo_algoritmo_seleccionado;
        }
        float controls_y = base_y;
        int graph_columns = compact ? 2 : 4;
        float graph_btn_w = (panel.width - 32.0f - gap * (graph_columns - 1)) / graph_columns;
        int action_count = 0;
        int action;
        float graph_hints_y;
        const char *graph_hint = "Atajos: G/X aristas | Pasos: , . / Home End | Auto: P | Demo: M";
        bool show_flow_hint = true;
        bool camino_origen_ok = false;
        bool camino_destino_ok = false;
        bool camino_extremos_validos = false;
        bool camino_arista_actual_existe = false;
        int camino_arista_peso_actual = 0;
        bool mst_inicio_ok = false;

        if (graph_btn_w < 132.0f) {
            graph_btn_w = 132.0f;
        }

        switch (g_grafo_ui_mode) {
        case 0:
            action_count = 8;
            graph_hint = "Flujo: Inicializar -> Vertices -> Aristas";
            break;
        case 1:
            action_count = 6;
            graph_hint = "";
            break;
        case 2:
            action_count = 6;
            graph_hint = "";
            break;
        default:
            action_count = 6;
            graph_hint = "";
            break;
        }
        if (g_grafo_ui_mode == 1) {
            graph_columns = panel.width >= 640.0f ? 3 : 2;
            graph_btn_w = (panel.width - 32.0f - gap * (graph_columns - 1)) / graph_columns;
            if (graph_btn_w < 132.0f) {
                graph_btn_w = 132.0f;
            }
        } else if (g_grafo_ui_mode == 2) {
            graph_columns = panel.width >= 640.0f ? 3 : 2;
            graph_btn_w = (panel.width - 32.0f - gap * (graph_columns - 1)) / graph_columns;
            if (graph_btn_w < 132.0f) {
                graph_btn_w = 132.0f;
            }
            camino_origen_ok = grafo_existe_vertice(app->grafo, app->grafo_vertice_inicio);
            camino_destino_ok = grafo_existe_vertice(app->grafo, app->grafo_vertice_destino);
            camino_extremos_validos = camino_origen_ok && camino_destino_ok;
            if (camino_extremos_validos) {
                camino_arista_actual_existe =
                    grafo_obtener_peso(app->grafo, app->grafo_vertice_inicio,
                                       app->grafo_vertice_destino,
                                       &camino_arista_peso_actual) == GRAFO_OK;
            }
        } else if (g_grafo_ui_mode == 3) {
            graph_columns = panel.width >= 640.0f ? 3 : 2;
            graph_btn_w = (panel.width - 32.0f - gap * (graph_columns - 1)) / graph_columns;
            if (graph_btn_w < 132.0f) {
                graph_btn_w = 132.0f;
            }
            mst_inicio_ok = grafo_existe_vertice(app->grafo, app->grafo_vertice_inicio);
        }

        for (action = 0; action < action_count; action++) {
            Rectangle btn = {base_x + (action % graph_columns) * (graph_btn_w + gap),
                             controls_y + (action / graph_columns) * row_step, graph_btn_w, btn_h};
            if (g_grafo_ui_mode == 0) {
                if (action == 0 && ui_button(btn, "Inicializar (I)", false)) {
                    app_state_operacion_inicializar(app);
                } else if (action == 1 && ui_button(btn, "Vertice + (A)", false)) {
                    app_state_operacion_insertar(app);
                } else if (action == 2 && ui_button(btn, "Vertice - (D)", false)) {
                    app_state_operacion_eliminar(app);
                } else if (action == 3 && ui_button(btn, "Arista + (G)", false)) {
                    app_state_operacion_grafo_insertar_arista(app, app->grafo_vertice_inicio,
                                                              app->grafo_vertice_destino,
                                                              app->input_peso_grafo);
                } else if (action == 4 && ui_button(btn, "Arista - (X)", false)) {
                    app_state_operacion_grafo_eliminar_arista(app, app->grafo_vertice_inicio,
                                                              app->grafo_vertice_destino);
                } else if (action == 5 && ui_button(btn, app->grafo_dirigido ? "Dirigido (T)"
                                                                              : "No dirigido (T)",
                                                    false)) {
                    app_state_grafo_toggle_dirigido(app);
                } else if (action == 6 && ui_button(btn, "Cargar demo (M)", false)) {
                    app_state_grafo_cargar_demo(app);
                } else if (action == 7 && ui_button(btn, "Limpiar", false)) {
                    app_state_operacion_inicializar(app);
                }
            } else if (g_grafo_ui_mode == 1) {
                if (action == 0 &&
                    ui_button(btn,
                              g_grafo_recorrido_algo == GRAFO_ALGO_BFS ? "Algoritmo: BFS"
                                                                       : "Algoritmo: DFS",
                              false)) {
                    g_grafo_recorrido_algo = (g_grafo_recorrido_algo == GRAFO_ALGO_BFS)
                                                 ? GRAFO_ALGO_DFS
                                                 : GRAFO_ALGO_BFS;
                    app->grafo_algoritmo_seleccionado = g_grafo_recorrido_algo;
                    snprintf(app->mensaje_operacion, sizeof(app->mensaje_operacion),
                             "Recorridos: algoritmo seleccionado %s",
                             g_grafo_recorrido_algo == GRAFO_ALGO_BFS ? "BFS" : "DFS");
                    app->ultima_operacion_ok = true;
                } else if (action == 1 &&
                           ui_button(btn, TextFormat("Ejecutar desde V%d", app->grafo_vertice_inicio),
                                     false)) {
                    if (!grafo_existe_vertice(app->grafo, app->grafo_vertice_inicio)) {
                        snprintf(app->mensaje_operacion, sizeof(app->mensaje_operacion),
                                 "Define un vertice de inicio valido para BFS/DFS");
                        app->ultima_operacion_ok = false;
                    } else {
                        app_state_operacion_grafo_ejecutar_algoritmo(app, g_grafo_recorrido_algo,
                                                                     app->grafo_vertice_inicio,
                                                                     app->grafo_vertice_destino);
                    }
                } else if (action == 2 && ui_button(btn, "Paso - (,)", false)) {
                    grafo_controller_paso_anterior(&app->grafo_controller_state);
                } else if (action == 3 && ui_button(btn, "Paso + (.)", false)) {
                    grafo_controller_paso_siguiente(&app->grafo_controller_state);
                } else if (action == 4 && ui_button(btn, "Reiniciar", false)) {
                    grafo_controller_reiniciar(&app->grafo_controller_state);
                } else if (action == 5 && ui_button(btn, app->grafo_controller_state.autoplay_activo
                                                             ? "Auto: ON (P)"
                                                             : "Auto: OFF (P)",
                                                    false)) {
                    grafo_controller_toggle_autoplay(&app->grafo_controller_state);
                }
            } else if (g_grafo_ui_mode == 2) {
                if (action == 0 &&
                    ui_button(btn, g_grafo_camino_algo == GRAFO_ALGO_DIJKSTRA
                                       ? "Algoritmo: Dijkstra"
                                       : "Algoritmo: Bellman-Ford",
                              false)) {
                    g_grafo_camino_algo = (g_grafo_camino_algo == GRAFO_ALGO_DIJKSTRA)
                                              ? GRAFO_ALGO_BELLMAN_FORD
                                              : GRAFO_ALGO_DIJKSTRA;
                    app->grafo_algoritmo_seleccionado = g_grafo_camino_algo;
                    snprintf(app->mensaje_operacion, sizeof(app->mensaje_operacion),
                             "Caminos: algoritmo seleccionado %s",
                             g_grafo_camino_algo == GRAFO_ALGO_DIJKSTRA ? "Dijkstra"
                                                                         : "Bellman-Ford");
                    app->ultima_operacion_ok = true;
                } else if (action == 1 &&
                           ui_button(btn,
                                     TextFormat("Ejecutar V%d -> V%d", app->grafo_vertice_inicio,
                                                app->grafo_vertice_destino),
                                     false)) {
                    if (!camino_extremos_validos) {
                        snprintf(app->mensaje_operacion, sizeof(app->mensaje_operacion),
                                 "Define origen y destino validos para camino minimo");
                        app->ultima_operacion_ok = false;
                    } else {
                        app_state_operacion_grafo_ejecutar_algoritmo(app, g_grafo_camino_algo,
                                                                     app->grafo_vertice_inicio,
                                                                     app->grafo_vertice_destino);
                    }
                } else if (action == 2 && ui_button(btn, "Paso - (,)", false)) {
                    grafo_controller_paso_anterior(&app->grafo_controller_state);
                } else if (action == 3 && ui_button(btn, "Paso + (.)", false)) {
                    grafo_controller_paso_siguiente(&app->grafo_controller_state);
                } else if (action == 4 && ui_button(btn, "Reiniciar (/)", false)) {
                    grafo_controller_reiniciar(&app->grafo_controller_state);
                } else if (action == 5 && ui_button(btn, app->grafo_controller_state.autoplay_activo
                                                             ? "Auto: ON (P)"
                                                             : "Auto: OFF (P)",
                                                    false)) {
                    grafo_controller_toggle_autoplay(&app->grafo_controller_state);
                }
            } else {
                if (action == 0 &&
                    ui_button(btn,
                              app->grafo_algoritmo_seleccionado == GRAFO_ALGO_KRUSKAL
                                  ? "Algoritmo: Kruskal"
                                  : "Algoritmo: Prim",
                              false)) {
                    app->grafo_algoritmo_seleccionado =
                        (app->grafo_algoritmo_seleccionado == GRAFO_ALGO_KRUSKAL)
                            ? GRAFO_ALGO_PRIM
                            : GRAFO_ALGO_KRUSKAL;
                    snprintf(app->mensaje_operacion, sizeof(app->mensaje_operacion),
                             "MST: algoritmo seleccionado %s",
                             app->grafo_algoritmo_seleccionado == GRAFO_ALGO_PRIM ? "Prim"
                                                                                  : "Kruskal");
                    app->ultima_operacion_ok = true;
                } else if (action == 1 && ui_button(btn, "Ejecutar", false)) {
                    if (app->grafo_algoritmo_seleccionado == GRAFO_ALGO_PRIM && !mst_inicio_ok) {
                        snprintf(app->mensaje_operacion, sizeof(app->mensaje_operacion),
                                 "Define un vertice valido en Inicio para Prim");
                        app->ultima_operacion_ok = false;
                    } else {
                        app_state_operacion_grafo_ejecutar_algoritmo(
                            app, app->grafo_algoritmo_seleccionado, app->grafo_vertice_inicio,
                            app->grafo_vertice_destino);
                    }
                } else if (action == 2 && ui_button(btn, "Paso - (,)", false)) {
                    grafo_controller_paso_anterior(&app->grafo_controller_state);
                } else if (action == 3 && ui_button(btn, "Paso + (.)", false)) {
                    grafo_controller_paso_siguiente(&app->grafo_controller_state);
                } else if (action == 4 && ui_button(btn, "Reiniciar", false)) {
                    grafo_controller_reiniciar(&app->grafo_controller_state);
                } else if (action == 5 && ui_button(btn, "Auto ON/OFF", false)) {
                    grafo_controller_toggle_autoplay(&app->grafo_controller_state);
                }
            }
        }

        graph_hints_y = controls_y + ((action_count - 1) / graph_columns + 1) * row_step + 6.0f;
        if (g_grafo_ui_mode == 1) {
            const char *algo_label =
                g_grafo_recorrido_algo == GRAFO_ALGO_BFS ? "BFS" : "DFS";
            char inicio_text[96];

            snprintf(inicio_text, sizeof(inicio_text), "Inicio: V%d", app->grafo_vertice_inicio);

            ui_draw_text(TextFormat("Recorridos: %s | V%d", algo_label, app->grafo_vertice_inicio),
                         panel.x + 16.0f,
                         graph_hints_y, 15.0f, 0.08f, (Color){38, 60, 86, 255}, true);
            (void)inicio_text;
            graph_hints_y += 16.0f;
            show_flow_hint = false;
        } else if (g_grafo_ui_mode == 2) {
            char od_text[96];
            snprintf(od_text, sizeof(od_text), "Origen: V%d  Destino: V%d",
                     app->grafo_vertice_inicio, app->grafo_vertice_destino);
            ui_draw_text(
                TextFormat("Camino minimo: %s | V%d -> V%d",
                           g_grafo_camino_algo == GRAFO_ALGO_DIJKSTRA ? "Dijkstra"
                                                                       : "Bellman-Ford",
                           app->grafo_vertice_inicio, app->grafo_vertice_destino),
                panel.x + 16.0f, graph_hints_y, 15.0f, 0.08f, (Color){38, 60, 86, 255}, true);
            (void)od_text;
            ui_draw_text(camino_arista_actual_existe
                             ? TextFormat("Peso directo V%d->V%d: %d", app->grafo_vertice_inicio,
                                          app->grafo_vertice_destino, camino_arista_peso_actual)
                             : TextFormat("Sin arista directa V%d->V%d",
                                          app->grafo_vertice_inicio,
                                          app->grafo_vertice_destino),
                         panel.x + 16.0f, graph_hints_y + 16.0f, 13.0f, 0.08f,
                         (Color){54, 66, 82, 255}, false);
            graph_hints_y += 31.0f;
            show_flow_hint = false;
        } else if (g_grafo_ui_mode == 3) {
            char inicio_mst[96];
            const char *algo_mst =
                app->grafo_algoritmo_seleccionado == GRAFO_ALGO_KRUSKAL ? "Kruskal" : "Prim";

            snprintf(inicio_mst, sizeof(inicio_mst), "Inicio Prim: V%d (%s)",
                     app->grafo_vertice_inicio, mst_inicio_ok ? "OK" : "invalido");

            ui_draw_text(TextFormat("MST: %s", algo_mst), panel.x + 16.0f,
                         graph_hints_y, 15.0f, 0.08f, (Color){38, 60, 86, 255}, true);
            ui_draw_text(inicio_mst, panel.x + 16.0f, graph_hints_y + 16.0f, 14.0f, 0.08f,
                         (Color){54, 66, 82, 255}, false);
            graph_hints_y += 31.0f;
            show_flow_hint = false;
        }
        if (show_flow_hint) {
            ui_draw_text(graph_hint, panel.x + 16.0f, graph_hints_y, 15.0f, 0.10f,
                         (Color){54, 66, 82, 255}, false);
            graph_hints_y += 18.0f;
        }
        return graph_hints_y + 2.0f;
    }

    if (app->estructura_activa == ESTRUCTURA_LISTA ||
        app->estructura_activa == ESTRUCTURA_LISTA_CIRCULAR) {
        count = 7;
        hint = "Atajos: UP/DOWN valor, Z inicio, A final, B buscar, D eliminar, R invertir";
    } else if (app->estructura_activa == ESTRUCTURA_SUBLISTA) {
        count = 7;
        hint = "Atajos: A padre+, B seleccionar padre, D padre-, Z hijo+, R hijo-";
    } else if (app->estructura_activa == ESTRUCTURA_COLA_PRIORIDAD) {
        count = 4;
        hint = "Atajos: UP/DOWN valor, LEFT/RIGHT prioridad";
    } else {
        count = 4;
        hint = "Atajos: UP/DOWN valor";
    }

    for (i = 0; i < count; i++) {
        Rectangle btn = {base_x + (i % columns) * (btn_w + gap),
                         base_y + (i / columns) * row_step,
                         btn_w,
                         btn_h};

        switch (app->estructura_activa) {
        case ESTRUCTURA_PILA:
            if (i == 0 && ui_button(btn, "Inicializar (I)", false)) {
                app_state_operacion_inicializar(app);
            } else if (i == 1 && ui_button(btn, "Push (A)", false)) {
                app_state_operacion_insertar(app);
            } else if (i == 2 && ui_button(btn, "Pop (D)", false)) {
                app_state_operacion_eliminar(app);
            } else if (i == 3 && ui_button(btn, "Vaciar (V)", false)) {
                app_state_operacion_vaciar(app);
            }
            break;
        case ESTRUCTURA_COLA:
            if (i == 0 && ui_button(btn, "Inicializar (I)", false)) {
                app_state_operacion_inicializar(app);
            } else if (i == 1 && ui_button(btn, "Encolar (A)", false)) {
                app_state_operacion_insertar(app);
            } else if (i == 2 && ui_button(btn, "Desencolar (D)", false)) {
                app_state_operacion_eliminar(app);
            } else if (i == 3 && ui_button(btn, "Vaciar (V)", false)) {
                app_state_operacion_vaciar(app);
            }
            break;
        case ESTRUCTURA_COLA_PRIORIDAD:
            if (i == 0 && ui_button(btn, "Inicializar (I)", false)) {
                app_state_operacion_inicializar(app);
            } else if (i == 1 && ui_button(btn, "Encolar (A)", false)) {
                app_state_operacion_insertar(app);
            } else if (i == 2 && ui_button(btn, "Desencolar (D)", false)) {
                app_state_operacion_eliminar(app);
            } else if (i == 3 && ui_button(btn, "Vaciar (V)", false)) {
                app_state_operacion_vaciar(app);
            }
            break;
        case ESTRUCTURA_LISTA:
        case ESTRUCTURA_LISTA_CIRCULAR:
            if (i == 0 && ui_button(btn, "Inicializar (I)", false)) {
                app_state_operacion_inicializar(app);
            } else if (i == 1 && ui_button(btn, "Inicio (Z)", false)) {
                app_state_operacion_lista_insertar_inicio(app);
            } else if (i == 2 && ui_button(btn, "Final (A)", false)) {
                app_state_operacion_lista_insertar_final(app);
            } else if (i == 3 && ui_button(btn, "Buscar (B)", false)) {
                app_state_operacion_buscar(app);
            } else if (i == 4 && ui_button(btn, "Eliminar (D)", false)) {
                app_state_operacion_eliminar(app);
            } else if (i == 5 && ui_button(btn, "Invertir (R)", false)) {
                app_state_operacion_invertir(app);
            } else if (i == 6 && ui_button(btn, "Vaciar (V)", false)) {
                app_state_operacion_vaciar(app);
            }
            break;
        case ESTRUCTURA_SUBLISTA:
            if (i == 0 && ui_button(btn, "Inicializar (I)", false)) {
                app_state_operacion_inicializar(app);
            } else if (i == 1 && ui_button(btn, "Padre + (A)", false)) {
                app_state_operacion_insertar(app);
            } else if (i == 2 && ui_button(btn, "Sel Padre (B)", false)) {
                app_state_operacion_buscar(app);
            } else if (i == 3 && ui_button(btn, "Padre - (D)", false)) {
                app_state_operacion_eliminar(app);
            } else if (i == 4 && ui_button(btn, "Hijo + (Z)", false)) {
                app_state_operacion_sublista_insertar_hijo(app);
            } else if (i == 5 && ui_button(btn, "Hijo - (R)", false)) {
                app_state_operacion_sublista_eliminar_hijo(app);
            } else if (i == 6 && ui_button(btn, "Vaciar (V)", false)) {
                app_state_operacion_vaciar(app);
            }
            break;
        default:
            break;
        }
    }

    hints_y = base_y + ((count - 1) / columns + 1) * row_step + 6.0f;
    ui_draw_text(hint, panel.x + 16.0f, hints_y, 14.0f, 0.16f, (Color){66, 76, 86, 255}, false);
    return hints_y + 24.0f;
}

/** @brief Atiende atajos globales de teclado para entradas y operaciones. */
static void handle_keyboard(AppState *app, InputFocus focus) {
    if (focus != INPUT_NONE) {
        return;
    }

    if (IsKeyPressed(KEY_UP)) {
        app_state_ajustar_valor(app, 1);
    }
    if (IsKeyPressed(KEY_DOWN)) {
        app_state_ajustar_valor(app, -1);
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        if (app->estructura_activa == ESTRUCTURA_GRAFO &&
            (g_grafo_ui_mode == 0 || g_grafo_ui_mode == 2)) {
            app_state_ajustar_peso_grafo(app, 1);
        } else {
            app_state_ajustar_prioridad(app, 1);
        }
    }
    if (IsKeyPressed(KEY_LEFT)) {
        if (app->estructura_activa == ESTRUCTURA_GRAFO &&
            (g_grafo_ui_mode == 0 || g_grafo_ui_mode == 2)) {
            app_state_ajustar_peso_grafo(app, -1);
        } else {
            app_state_ajustar_prioridad(app, -1);
        }
    }
    if (IsKeyPressed(KEY_I)) {
        app_state_operacion_inicializar(app);
    }
    if (IsKeyPressed(KEY_A)) {
        if (app->estructura_activa == ESTRUCTURA_GRAFO) {
            app_state_operacion_insertar(app);
        } else if (app->estructura_activa == ESTRUCTURA_LISTA ||
            app->estructura_activa == ESTRUCTURA_LISTA_CIRCULAR) {
            app_state_operacion_lista_insertar_final(app);
        } else {
            app_state_operacion_insertar(app);
        }
    }
    if (IsKeyPressed(KEY_Z)) {
        if (app->estructura_activa == ESTRUCTURA_SUBLISTA) {
            app_state_operacion_sublista_insertar_hijo(app);
        } else {
            app_state_operacion_lista_insertar_inicio(app);
        }
    }
    if (IsKeyPressed(KEY_D)) {
        app_state_operacion_eliminar(app);
    }
    if (app->estructura_activa == ESTRUCTURA_GRAFO) {
        if (IsKeyPressed(KEY_G)) {
            if (g_grafo_ui_mode == 2) {
                app_state_operacion_grafo_actualizar_peso_arista(
                    app, app->grafo_vertice_inicio, app->grafo_vertice_destino,
                    app->input_peso_grafo);
            } else {
                app_state_operacion_grafo_insertar_arista(app, app->grafo_vertice_inicio,
                                                          app->grafo_vertice_destino,
                                                          app->input_peso_grafo);
            }
        }
        if (IsKeyPressed(KEY_X)) {
            app_state_operacion_grafo_eliminar_arista(app, app->grafo_vertice_inicio,
                                                      app->grafo_vertice_destino);
        }
        if (IsKeyPressed(KEY_FOUR)) {
            app_state_operacion_grafo_ejecutar_algoritmo(app, GRAFO_ALGO_BFS,
                                                         app->grafo_vertice_inicio,
                                                         app->grafo_vertice_destino);
        }
        if (IsKeyPressed(KEY_FIVE)) {
            app_state_operacion_grafo_ejecutar_algoritmo(app, GRAFO_ALGO_DFS,
                                                         app->grafo_vertice_inicio,
                                                         app->grafo_vertice_destino);
        }
        if (IsKeyPressed(KEY_SIX)) {
            app_state_operacion_grafo_ejecutar_algoritmo(app, GRAFO_ALGO_DIJKSTRA,
                                                         app->grafo_vertice_inicio,
                                                         app->grafo_vertice_destino);
        }
        if (IsKeyPressed(KEY_COMMA)) {
            grafo_controller_paso_anterior(&app->grafo_controller_state);
        }
        if (IsKeyPressed(KEY_PERIOD)) {
            grafo_controller_paso_siguiente(&app->grafo_controller_state);
        }
        if (IsKeyPressed(KEY_SLASH)) {
            grafo_controller_reiniciar(&app->grafo_controller_state);
        }
        if (IsKeyPressed(KEY_HOME)) {
            grafo_controller_ir_inicio(&app->grafo_controller_state);
        }
        if (IsKeyPressed(KEY_END)) {
            grafo_controller_ir_final(&app->grafo_controller_state);
        }
        if (IsKeyPressed(KEY_P)) {
            grafo_controller_toggle_autoplay(&app->grafo_controller_state);
        }
        if (IsKeyPressed(KEY_O)) {
            grafo_controller_cambiar_velocidad(&app->grafo_controller_state);
        }
        if (IsKeyPressed(KEY_T)) {
            app_state_grafo_toggle_dirigido(app);
        }
        if (IsKeyPressed(KEY_M)) {
            app_state_grafo_cargar_demo(app);
        }
    }
    if (IsKeyPressed(KEY_V)) {
        app_state_operacion_vaciar(app);
    }
    if (IsKeyPressed(KEY_B)) {
        app_state_operacion_buscar(app);
    }
    if (IsKeyPressed(KEY_R)) {
        if (app->estructura_activa == ESTRUCTURA_SUBLISTA) {
            app_state_operacion_sublista_eliminar_hijo(app);
        } else if (app->estructura_activa == ESTRUCTURA_GRAFO) {
            app_state_operacion_grafo_ejecutar_algoritmo(app, GRAFO_ALGO_KRUSKAL,
                                                         app->grafo_vertice_inicio,
                                                         app->grafo_vertice_destino);
        } else {
            app_state_operacion_invertir(app);
        }
    }
}

/** @brief Sincroniza los buffers de texto editables con el estado real de entrada. */
static void sync_input_buffers(const AppState *app, char *value_text, size_t value_size,
                               char *priority_text, size_t priority_size,
                               char *graph_origin_text, size_t graph_origin_size,
                               char *graph_dest_text, size_t graph_dest_size,
                               char *graph_weight_text, size_t graph_weight_size,
                               InputFocus focus) {
    if (focus != INPUT_VALOR) {
        snprintf(value_text, value_size, "%d", app->input_valor);
    }
    if (focus != INPUT_PRIORIDAD) {
        snprintf(priority_text, priority_size, "%d", app->input_prioridad);
    }
    if (focus != INPUT_GRAFO_ORIGEN) {
        snprintf(graph_origin_text, graph_origin_size, "%d", app->grafo_vertice_inicio);
    }
    if (focus != INPUT_GRAFO_DESTINO) {
        snprintf(graph_dest_text, graph_dest_size, "%d", app->grafo_vertice_destino);
    }
    if (focus != INPUT_GRAFO_PESO) {
        snprintf(graph_weight_text, graph_weight_size, "%d", app->input_peso_grafo);
    }
}

/** @brief Aplica el contenido textual de una caja de entrada al estado global. */
static void apply_input_focus(AppState *app, InputFocus focus, const char *text) {
    int parsed = 0;

    if (text == NULL || sscanf(text, "%d", &parsed) != 1) {
        return;
    }

    if (focus == INPUT_VALOR) {
        app_state_set_valor(app, parsed);
    } else if (focus == INPUT_PRIORIDAD) {
        app_state_set_prioridad(app, parsed);
    } else if (focus == INPUT_GRAFO_ORIGEN) {
        app->grafo_vertice_inicio = parsed;
    } else if (focus == INPUT_GRAFO_DESTINO) {
        app->grafo_vertice_destino = parsed;
    } else if (focus == INPUT_GRAFO_PESO) {
        app_state_set_peso_grafo(app, parsed);
    }
}

/** @brief Valida si un texto representa un entero sintacticamente valido. */
static bool parse_int_text(const char *text, int *value) {
    int parsed;
    char extra;

    if (text == NULL || text[0] == '\0') {
        return false;
    }

    if (sscanf(text, "%d%c", &parsed, &extra) != 1) {
        return false;
    }

    if (value != NULL) {
        *value = parsed;
    }
    return true;
}

/** @brief Indica si un ID de vertice existe en el grafo activo. */
static bool graph_vertex_exists(const AppState *app, int vertex_id) {
    if (app == NULL || app->grafo == NULL || vertex_id < 0) {
        return false;
    }
    return grafo_existe_vertice(app->grafo, vertex_id);
}

/** @brief Actualiza el buffer de la caja de entrada activa usando teclado. */
static void edit_active_input(char *buffer, size_t size, bool *replace_on_type) {
    int key = GetCharPressed();
    size_t len = strlen(buffer);
    bool replace_now = (replace_on_type != NULL) ? *replace_on_type : false;
    int numeric_value = 0;
    bool has_numeric_value = parse_int_text(buffer, &numeric_value);

    while (key > 0) {
        if (key >= '0' && key <= '9') {
            if (replace_now) {
                buffer[0] = '\0';
                len = 0;
                replace_now = false;
            }
            if (len + 1 < size) {
                if (key >= '0' && key <= '9' && len == 1 && buffer[0] == '0') {
                    buffer[0] = (char)key;
                } else if (key >= '0' && key <= '9' && len == 2 && buffer[0] == '-' &&
                           buffer[1] == '0') {
                    buffer[1] = (char)key;
                } else {
                    buffer[len] = (char)key;
                    buffer[len + 1] = '\0';
                    len++;
                }
            }
        } else if (key == '-') {
            if (replace_now) {
                buffer[0] = '-';
                buffer[1] = '\0';
                len = 1;
                replace_now = false;
            } else if (len == 0 && size > 1) {
                buffer[0] = '-';
                buffer[1] = '\0';
                len = 1;
            }
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE) && len > 0) {
        buffer[len - 1] = '\0';
        replace_now = false;
    }

    /* Entradas se comportan como numericas con nudge por teclado. */
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_KP_ADD)) {
        if (!has_numeric_value) {
            numeric_value = 0;
        }
        numeric_value += 1;
        snprintf(buffer, size, "%d", numeric_value);
        replace_now = false;
    } else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_KP_SUBTRACT)) {
        if (!has_numeric_value) {
            numeric_value = 0;
        }
        numeric_value -= 1;
        snprintf(buffer, size, "%d", numeric_value);
        replace_now = false;
    }

    if (replace_on_type != NULL) {
        *replace_on_type = replace_now;
    }
}

int main(int argc, char **argv) {
    const int screen_width = 1280;
    const int screen_height = 760;
    UIContext ui;
    UILayout layout;
    AppState app;
    AlgorithmInfo info;
    const char *snippet;
    const char *tiempo_texto;
    const char *espacio_texto;
    Rectangle btn;
    Rectangle value_box;
    Rectangle priority_box;
    Rectangle graph_origin_box;
    Rectangle graph_dest_box;
    Rectangle graph_weight_box;
    InputFocus input_focus = INPUT_NONE;
    char value_text[16];
    char priority_text[16];
    char graph_origin_text[16];
    char graph_dest_text[16];
    char graph_weight_text[16];
    float code_scroll = 0.0f;
    float graph_result_scroll = 0.0f;
    Color status_color;
    const char *status_label;
    int cantidad_activa;
    bool value_invalid;
    bool priority_invalid;
    bool graph_origin_invalid;
    bool graph_dest_invalid;
    bool graph_weight_invalid;
    bool graph_origin_exists;
    bool graph_dest_exists;
    ScreenMode screen_mode = SCREEN_HOME_ROOT;
    int parsed_value;
    int parsed_priority;
    int parsed_graph_origin;
    int parsed_graph_dest;
    int parsed_graph_weight;
    int home_selected = 0;
    bool home_activate = false;
    ScreenMode screen_before_help = SCREEN_HOME_ROOT;
    float help_scroll = 0.0f;
    char code_history[CODE_HISTORY_CAPACITY];
    unsigned int code_history_last_serial = 0;
    int code_history_entries = 0;
    const char *code_display_text;
    bool code_panel_compact = true;
    InputFocus last_input_focus = INPUT_NONE;
    bool replace_on_type = false;
    E2EVisualMode e2e_visual;

    e2e_visual_parse_args(&e2e_visual, argc, argv);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
    InitWindow(screen_width, screen_height, "VisualStruct UTP");
    SetWindowMinSize(1100, 680);
    SetExitKey(KEY_NULL);
    ui_init(&ui, screen_width, screen_height);
    app_state_init(&app);
    code_history[0] = '\0';
    snprintf(value_text, sizeof(value_text), "%d", app.input_valor);
    snprintf(priority_text, sizeof(priority_text), "%d", app.input_prioridad);
    snprintf(graph_origin_text, sizeof(graph_origin_text), "%d", app.grafo_vertice_inicio);
    snprintf(graph_dest_text, sizeof(graph_dest_text), "%d", app.grafo_vertice_destino);
    snprintf(graph_weight_text, sizeof(graph_weight_text), "%d", app.input_peso_grafo);

    SetTargetFPS(60);

    while (true) {

        if (WindowShouldClose()) {
            if (screen_mode == SCREEN_VISUALIZER) {
                screen_mode = SCREEN_HOME_ROOT;
                continue;
            }
            break;
        }

        ui_set_size(&ui, GetScreenWidth(), GetScreenHeight());
        layout = ui_get_layout(&ui);
        if (screen_mode == SCREEN_VISUALIZER && app.estructura_activa == ESTRUCTURA_GRAFO) {
            const float target_bottom_graph = 96.0f;
            float reduce_h = layout.bottom.height - target_bottom_graph;
            if (reduce_h < 0.0f) {
                reduce_h = 0.0f;
            }
            if (reduce_h > 0.0f) {
                layout.center.height += reduce_h;
                layout.right.height += reduce_h;
                layout.bottom.y += reduce_h;
                layout.bottom.height -= reduce_h;
            }
        }
        app_state_update_visuals(&app, GetFrameTime());
        grafo_controller_actualizar(&app.grafo_controller_state, GetFrameTime());
        handle_navigation_keyboard(&screen_mode, &app, &home_selected, &home_activate, input_focus);
        if (IsKeyPressed(KEY_F1)) {
            if (screen_mode == SCREEN_HELP) {
                screen_mode = screen_before_help;
            } else {
                screen_before_help = screen_mode;
                help_scroll = 0.0f;
                screen_mode = SCREEN_HELP;
            }
        }

        if (screen_mode == SCREEN_HELP) {
            bool close_help;

            BeginDrawing();
            ClearBackground((Color){250, 252, 254, 255});
            ui_draw_header(&ui);
            ui_draw_footer(&ui);
            close_help = draw_help_screen(&layout, &help_scroll);
            EndDrawing();
            if (e2e_visual_after_frame(&e2e_visual, &app, &screen_mode, &home_selected,
                                       &home_activate)) {
                break;
            }

            if (close_help || IsKeyPressed(KEY_ESCAPE)) {
                screen_mode = screen_before_help;
            }
            continue;
        }

        if (screen_mode == SCREEN_HOME_ROOT || screen_mode == SCREEN_HOME_SECUENCIALES ||
            screen_mode == SCREEN_HOME_GRAFOS) {
            BeginDrawing();
            ClearBackground((Color){250, 252, 254, 255});
            ui_draw_header(&ui);
            ui_draw_footer(&ui);
            if (screen_mode == SCREEN_HOME_ROOT) {
                draw_home_root_screen(&layout, &screen_mode, &home_selected, home_activate);
            } else if (screen_mode == SCREEN_HOME_SECUENCIALES) {
                draw_home_screen(&layout, &app, &screen_mode, &home_selected, home_activate);
            } else {
                draw_home_graph_screen(&layout, &app, &screen_mode, &home_selected,
                                       home_activate);
            }
            EndDrawing();
            if (e2e_visual_after_frame(&e2e_visual, &app, &screen_mode, &home_selected,
                                       &home_activate)) {
                break;
            }
            continue;
        }

        {
            float wheel_move = GetMouseWheelMove();
            Rectangle graph_area = app.grafo_controller_state.vista.area_renderizado;
            Rectangle graph_result_viewport = {layout.right.x + 24.0f, layout.right.y + 198.0f,
                                               layout.right.width - 48.0f,
                                               layout.right.height - 232.0f};
            bool shift_horizontal = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
            if (wheel_move != 0.0f && app.estructura_activa == ESTRUCTURA_GRAFO &&
                graph_area.width > 1.0f && graph_area.height > 1.0f &&
                CheckCollisionPointRec(GetMousePosition(), graph_area)) {
                grafo_controller_scroll_vista(&app.grafo_controller_state, wheel_move,
                                              shift_horizontal);
            } else if (wheel_move != 0.0f && app.estructura_activa == ESTRUCTURA_GRAFO &&
                       CheckCollisionPointRec(GetMousePosition(), graph_result_viewport)) {
                graph_result_scroll -= wheel_move * 22.0f;
            } else if (CheckCollisionPointRec(GetMousePosition(), layout.right) &&
                       wheel_move != 0.0f) {
                code_scroll -= wheel_move * 22.0f;
            }
        }
        handle_keyboard(&app, input_focus);
        sync_input_buffers(&app, value_text, sizeof(value_text), priority_text,
                           sizeof(priority_text), graph_origin_text,
                           sizeof(graph_origin_text), graph_dest_text,
                           sizeof(graph_dest_text), graph_weight_text,
                           sizeof(graph_weight_text), input_focus);
        value_invalid = !parse_int_text(value_text, &parsed_value);
        priority_invalid = !parse_int_text(priority_text, &parsed_priority) ||
                           parsed_priority < 1 || parsed_priority > 99;
        graph_origin_invalid = !parse_int_text(graph_origin_text, &parsed_graph_origin);
        graph_dest_invalid = !parse_int_text(graph_dest_text, &parsed_graph_dest);
        graph_weight_invalid = !parse_int_text(graph_weight_text, &parsed_graph_weight) ||
                               parsed_graph_weight < -999 || parsed_graph_weight > 999;
        graph_origin_exists = false;
        graph_dest_exists = false;

        if (app.estructura_activa == ESTRUCTURA_GRAFO) {
            if (!graph_origin_invalid) {
                graph_origin_exists = graph_vertex_exists(&app, parsed_graph_origin);
                graph_origin_invalid = !graph_origin_exists;
            }
            if (!graph_dest_invalid) {
                graph_dest_exists = graph_vertex_exists(&app, parsed_graph_dest);
                graph_dest_invalid = !graph_dest_exists;
            }
        }

        if (app.estructura_activa == ESTRUCTURA_GRAFO) {
            if (!graph_origin_invalid) {
                app.grafo_vertice_inicio = parsed_graph_origin;
            }
            if (!graph_dest_invalid) {
                app.grafo_vertice_destino = parsed_graph_dest;
            }
            if (!graph_weight_invalid) {
                app_state_set_peso_grafo(&app, parsed_graph_weight);
            }
        }

        if (input_focus != last_input_focus) {
            replace_on_type = (input_focus != INPUT_NONE);
            last_input_focus = input_focus;
        }

        if (input_focus == INPUT_VALOR) {
            edit_active_input(value_text, sizeof(value_text), &replace_on_type);
        } else if (input_focus == INPUT_PRIORIDAD) {
            edit_active_input(priority_text, sizeof(priority_text), &replace_on_type);
        } else if (input_focus == INPUT_GRAFO_ORIGEN) {
            edit_active_input(graph_origin_text, sizeof(graph_origin_text), &replace_on_type);
        } else if (input_focus == INPUT_GRAFO_DESTINO) {
            edit_active_input(graph_dest_text, sizeof(graph_dest_text), &replace_on_type);
        } else if (input_focus == INPUT_GRAFO_PESO) {
            edit_active_input(graph_weight_text, sizeof(graph_weight_text), &replace_on_type);
        }

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
            if (input_focus != INPUT_NONE) {
                if ((input_focus == INPUT_VALOR && !value_invalid) ||
                    (input_focus == INPUT_PRIORIDAD && !priority_invalid) ||
                    (input_focus == INPUT_GRAFO_ORIGEN && !graph_origin_invalid) ||
                    (input_focus == INPUT_GRAFO_DESTINO && !graph_dest_invalid) ||
                    (input_focus == INPUT_GRAFO_PESO && !graph_weight_invalid)) {
                    apply_input_focus(&app, input_focus,
                                      input_focus == INPUT_VALOR
                                          ? value_text
                                          : input_focus == INPUT_PRIORIDAD
                                                ? priority_text
                                                : input_focus == INPUT_GRAFO_ORIGEN
                                                      ? graph_origin_text
                                                      : input_focus == INPUT_GRAFO_DESTINO
                                                            ? graph_dest_text
                                                            : graph_weight_text);
                    input_focus = INPUT_NONE;
                }
            }
        }

        snippet = code_viewer_get_snippet(app.estructura_activa, app.operacion_actual);
        if (app.operacion_serial != code_history_last_serial &&
            app.operacion_actual != OPERACION_NINGUNA) {
            append_code_history(code_history, sizeof(code_history), &code_history_entries, &app,
                                snippet);
            code_history_last_serial = app.operacion_serial;
            code_scroll = 1000000.0f;
            graph_result_scroll = 0.0f;
        }
        code_display_text = (code_history_entries > 0) ? code_history : snippet;
        info = algorithm_trace_get_info(app.estructura_activa, app.operacion_actual);
        cantidad_activa = estructura_cantidad(&app);
        tiempo_texto = info.tiempo;
        espacio_texto = info.espacio;
        if (app.estructura_activa == ESTRUCTURA_GRAFO) {
            sync_grafo_algoritmo_por_modo(&app);
            tiempo_texto = grafo_algoritmo_tiempo(app.grafo_algoritmo_seleccionado);
            espacio_texto = grafo_algoritmo_espacio(app.grafo_algoritmo_seleccionado);
        }

        if (app.operacion_actual == OPERACION_NINGUNA) {
            status_color = (Color){110, 126, 144, 255};
            status_label = "Estado";
        } else if (app.ultima_operacion_ok) {
            status_color = (Color){34, 122, 72, 255};
            status_label = "OK";
        } else {
            status_color = (Color){176, 54, 44, 255};
            status_label = "Error";
        }

        {
            char trace_text[1024];
            int code_lines = count_text_lines(code_display_text);
            float code_content_height;
            float code_viewport_height = layout.right.height - 118.0f;

            snprintf(trace_text, sizeof(trace_text),
                     "Estructura activa: %s\n"
                     "%s\n"
                     "Pasos: %s\n"
                     "Complejidad Tiempo: %s\n"
                     "Complejidad Espacio: %s",
                     estructura_nombre(app.estructura_activa), app.mensaje_operacion, info.pasos,
                     info.tiempo, info.espacio);
            code_content_height = code_lines * 22.0f;
            code_scroll = clamp_float(code_scroll, 0.0f,
                                      code_content_height > code_viewport_height
                                          ? code_content_height - code_viewport_height
                                          : 0.0f);
        }

        BeginDrawing();
        ClearBackground((Color){250, 252, 254, 255});

        ui_draw_header(&ui);
        ui_draw_footer(&ui);

        {
            float sidebar_section_bottom;

            if (app.estructura_activa == ESTRUCTURA_GRAFO) {
                Rectangle graph_mode_box;
                Rectangle graph_status_box;
                float graph_mode_y;
                float graph_btn_h = 32.0f;
                float graph_btn_gap = 6.0f;
                float graph_inputs_height;
                float graph_inputs_top;
                float graph_input_box_h = 38.0f;
                float graph_input_step = 62.0f;
                int graph_input_fields = 0;
                float status_y;
                float status_h;
                bool graph_mode_recorridos = g_grafo_ui_mode == 1;
                bool graph_mode_caminos = g_grafo_ui_mode == 2;
                Rectangle graph_mode_btn;
                const char *graph_mode_labels[4] = {"Construccion", "Recorridos", "Caminos", "MST"};

                ui_draw_panel(layout.sidebar, "Menu Grafo");
                btn = (Rectangle){layout.sidebar.x + 12.0f, layout.sidebar.y + 52.0f,
                                  layout.sidebar.width - 24.0f, 40.0f};
                if (ui_sidebar_button(btn, "Menu principal", false)) {
                    screen_mode = SCREEN_HOME_ROOT;
                }

                btn = (Rectangle){layout.sidebar.x + 12.0f, layout.sidebar.y + 102.0f,
                                  layout.sidebar.width - 24.0f, 40.0f};
                if (ui_sidebar_button(btn, "Ayuda (F1)", false)) {
                    screen_before_help = SCREEN_VISUALIZER;
                    help_scroll = 0.0f;
                    screen_mode = SCREEN_HELP;
                }

                graph_mode_y = layout.sidebar.y + 152.0f;
                if (graph_mode_recorridos || g_grafo_ui_mode == 3) {
                    graph_input_fields = 1;
                } else if (graph_mode_caminos) {
                    graph_input_fields = 3;
                } else {
                    graph_input_fields = 4;
                }
                if (graph_input_fields <= 1) {
                    graph_inputs_height = 64.0f;
                } else {
                    graph_inputs_height =
                        graph_input_box_h + (float)(graph_input_fields - 1) * graph_input_step + 12.0f;
                }
                graph_inputs_top = layout.sidebar.y + layout.sidebar.height - graph_inputs_height;
                graph_mode_box =
                    (Rectangle){layout.sidebar.x + 12.0f, graph_mode_y, layout.sidebar.width - 24.0f,
                                32.0f + 4.0f * (graph_btn_h + graph_btn_gap) + 4.0f};
                DrawRectangleRounded(graph_mode_box, 0.18f, 8, Fade((Color){220, 232, 247, 255}, 0.80f));
                DrawRectangleRoundedLinesEx(graph_mode_box, 0.18f, 8, 1.0f,
                                            Fade((Color){78, 110, 146, 255}, 0.25f));
                ui_draw_text("Vista de grafo", graph_mode_box.x + 10.0f, graph_mode_box.y + 10.0f,
                             13.0f, 0.08f, (Color){22, 46, 72, 255}, true);

                graph_mode_btn = (Rectangle){graph_mode_box.x + 8.0f, graph_mode_box.y + 30.0f,
                                             graph_mode_box.width - 16.0f, graph_btn_h};
                for (int mode_idx = 0; mode_idx < 4; mode_idx++) {
                    if (draw_graph_sidebar_button(graph_mode_btn, graph_mode_labels[mode_idx],
                                                  g_grafo_ui_mode == mode_idx)) {
                        g_grafo_ui_mode = mode_idx;
                        grafo_set_modo_contexto(&app);
                    }
                    graph_mode_btn.y += graph_btn_h + graph_btn_gap;
                }

                status_y = graph_mode_box.y + graph_mode_box.height + 10.0f;
                status_h = graph_inputs_top - status_y - 12.0f;
                if (status_h > 108.0f) {
                    status_h = 108.0f;
                }
                if (status_h < 64.0f) {
                    status_h = 64.0f;
                }
                graph_status_box = (Rectangle){layout.sidebar.x + 12.0f, status_y,
                                               layout.sidebar.width - 24.0f, status_h};
                DrawRectangleRounded(graph_status_box, 0.18f, 8, Fade((Color){220, 232, 247, 255}, 0.80f));
                DrawRectangleRoundedLinesEx(graph_status_box, 0.18f, 8, 1.0f,
                                            Fade((Color){78, 110, 146, 255}, 0.25f));
                ui_draw_text("Estado de grafo", graph_status_box.x + 10.0f, graph_status_box.y + 10.0f,
                             14.0f, 0.10f, (Color){22, 46, 72, 255}, true);
                ui_draw_text(TextFormat("Vertices: %d", cantidad_activa),
                             graph_status_box.x + 10.0f, graph_status_box.y + 34.0f,
                             13.0f, 0.08f, (Color){56, 68, 84, 255}, false);
                ui_draw_text(app.grafo_dirigido ? "Tipo: dirigido" : "Tipo: no dirigido",
                             graph_status_box.x + 10.0f, graph_status_box.y + 54.0f,
                             13.0f, 0.08f, (Color){56, 68, 84, 255}, false);
                sidebar_section_bottom = graph_status_box.y + graph_status_box.height + 8.0f;
            } else {
                ui_draw_panel(layout.sidebar, "Estructuras");
                btn = (Rectangle){layout.sidebar.x + 12.0f, layout.sidebar.y + 52.0f,
                                  layout.sidebar.width - 24.0f, 40.0f};
                if (ui_sidebar_button(btn, "Menu principal", false)) {
                    screen_mode = SCREEN_HOME_ROOT;
                }

                btn = (Rectangle){layout.sidebar.x + 12.0f, layout.sidebar.y + 102.0f,
                                  layout.sidebar.width - 24.0f, 40.0f};
                if (ui_sidebar_button(btn, "Ayuda (F1)", false)) {
                    screen_before_help = SCREEN_VISUALIZER;
                    help_scroll = 0.0f;
                    screen_mode = SCREEN_HELP;
                }

                btn = (Rectangle){layout.sidebar.x + 12.0f, layout.sidebar.y + 152.0f,
                                  layout.sidebar.width - 24.0f, 42.0f};
                if (ui_sidebar_button(btn, "Pila", app.estructura_activa == ESTRUCTURA_PILA)) {
                    app_state_set_estructura(&app, ESTRUCTURA_PILA);
                }
                btn.y += 50.0f;
                if (ui_sidebar_button(btn, "Cola", app.estructura_activa == ESTRUCTURA_COLA)) {
                    app_state_set_estructura(&app, ESTRUCTURA_COLA);
                }
                btn.y += 50.0f;
                if (ui_sidebar_button(btn, "Cola Prioridad",
                                      app.estructura_activa == ESTRUCTURA_COLA_PRIORIDAD)) {
                    app_state_set_estructura(&app, ESTRUCTURA_COLA_PRIORIDAD);
                }
                btn.y += 50.0f;
                if (ui_sidebar_button(btn, "Lista", app.estructura_activa == ESTRUCTURA_LISTA)) {
                    app_state_set_estructura(&app, ESTRUCTURA_LISTA);
                }
                btn.y += 50.0f;
                if (ui_sidebar_button(btn, "Lista Circular",
                                      app.estructura_activa == ESTRUCTURA_LISTA_CIRCULAR)) {
                    app_state_set_estructura(&app, ESTRUCTURA_LISTA_CIRCULAR);
                }
                btn.y += 50.0f;
                if (ui_sidebar_button(btn, "Sublistas",
                                      app.estructura_activa == ESTRUCTURA_SUBLISTA)) {
                    app_state_set_estructura(&app, ESTRUCTURA_SUBLISTA);
                }
                btn.y += 50.0f;
                if (ui_sidebar_button(btn, "Grafo", app.estructura_activa == ESTRUCTURA_GRAFO)) {
                    app_state_set_estructura(&app, ESTRUCTURA_GRAFO);
                    grafo_set_modo_contexto(&app);
                }

                sidebar_section_bottom = btn.y + 56.0f;
            }

            DrawLine((int)(layout.sidebar.x + 16.0f), (int)sidebar_section_bottom,
                     (int)(layout.sidebar.x + layout.sidebar.width - 16.0f),
                     (int)sidebar_section_bottom, Fade((Color){78, 110, 146, 255}, 0.28f));

        {
            bool show_priority = app.estructura_activa == ESTRUCTURA_COLA_PRIORIDAD;
            bool show_graph_inputs = app.estructura_activa == ESTRUCTURA_GRAFO;
            bool show_graph_recorridos = show_graph_inputs && g_grafo_ui_mode == 1;
            bool show_graph_caminos = show_graph_inputs && g_grafo_ui_mode == 2;
            bool show_graph_mst = show_graph_inputs && g_grafo_ui_mode == 3;
            bool show_value_input = !show_graph_inputs || g_grafo_ui_mode == 0;
            bool show_graph_origin_input = show_graph_inputs;
            bool show_graph_dest_input =
                show_graph_inputs && (g_grafo_ui_mode == 0 || g_grafo_ui_mode == 2);
            bool show_graph_weight_input =
                show_graph_inputs && (g_grafo_ui_mode == 0 || g_grafo_ui_mode == 2);
            float graph_input_box_h = 38.0f;
            float graph_input_step = 62.0f;
            int graph_visible_fields = 0;
            float info_y = sidebar_section_bottom + 8.0f;
            float inputs_height = 0.0f;
            float inputs_top = layout.sidebar.y + layout.sidebar.height - inputs_height;
            float available_info_h = inputs_top - info_y;
            float help_y = info_y + 70.0f;
            float nav_y = inputs_top - 18.0f;
            bool compact_sidebar = available_info_h < 120.0f;
            float title_size = compact_sidebar ? 20.0f : 22.0f;
            float meta_size = compact_sidebar ? 14.0f : 16.0f;
            float help_size = compact_sidebar ? 11.0f : 12.0f;
            float nav_size = compact_sidebar ? 10.0f : 11.0f;

            if (show_graph_inputs) {
                if (show_value_input) {
                    graph_visible_fields++;
                }
                if (show_graph_origin_input) {
                    graph_visible_fields++;
                }
                if (show_graph_dest_input) {
                    graph_visible_fields++;
                }
                if (show_graph_weight_input) {
                    graph_visible_fields++;
                }
                if (graph_visible_fields <= 1) {
                    inputs_height = 64.0f;
                } else {
                    inputs_height =
                        graph_input_box_h + (float)(graph_visible_fields - 1) * graph_input_step + 12.0f;
                }
            } else {
                inputs_height = show_priority ? 92.0f : 42.0f;
            }

            inputs_top = layout.sidebar.y + layout.sidebar.height - inputs_height;
            available_info_h = inputs_top - info_y;
            help_y = info_y + 70.0f;
            nav_y = inputs_top - 18.0f;
            compact_sidebar = available_info_h < 120.0f;
            title_size = compact_sidebar ? 20.0f : 22.0f;
            meta_size = compact_sidebar ? 14.0f : 16.0f;
            help_size = compact_sidebar ? 11.0f : 12.0f;
            nav_size = compact_sidebar ? 10.0f : 11.0f;

            value_box = (Rectangle){layout.sidebar.x + 12.0f,
                                    layout.sidebar.y + layout.sidebar.height - inputs_height,
                                    layout.sidebar.width - 24.0f, graph_input_box_h};
            priority_box = (Rectangle){layout.sidebar.x + 12.0f,
                                       layout.sidebar.y + layout.sidebar.height - 42.0f,
                                       layout.sidebar.width - 24.0f, 38.0f};
            graph_origin_box = (Rectangle){layout.sidebar.x + 12.0f,
                                           show_value_input ? (value_box.y + graph_input_step)
                                                            : value_box.y,
                                           layout.sidebar.width - 24.0f, graph_input_box_h};
            graph_dest_box = (Rectangle){layout.sidebar.x + 12.0f,
                                         graph_origin_box.y + graph_input_step,
                                         layout.sidebar.width - 24.0f, graph_input_box_h};
            graph_weight_box = (Rectangle){layout.sidebar.x + 12.0f,
                                           graph_dest_box.y + graph_input_step,
                                           layout.sidebar.width - 24.0f, graph_input_box_h};

            if (show_graph_inputs) {
                if (available_info_h >= 22.0f) {
                    ui_draw_text("Entradas de grafo", layout.sidebar.x + 12.0f, info_y,
                                 14.0f, 0.10f, (Color){50, 64, 80, 255}, true);
                }
                if (available_info_h >= 40.0f) {
                    if (show_graph_recorridos) {
                        ui_draw_text("Recorridos: usar Inicio.", layout.sidebar.x + 12.0f,
                                     info_y + 18.0f, 11.0f, 0.08f, (Color){76, 91, 110, 255},
                                     false);
                    } else if (show_graph_caminos) {
                        ui_draw_text("Caminos: Origen, Destino y Peso.", layout.sidebar.x + 12.0f,
                                     info_y + 18.0f, 11.0f, 0.08f, (Color){76, 91, 110, 255},
                                     false);
                    } else if (show_graph_mst) {
                        ui_draw_text("MST: Inicio solo para Prim.", layout.sidebar.x + 12.0f,
                                     info_y + 18.0f, 11.0f, 0.08f, (Color){76, 91, 110, 255},
                                     false);
                    }
                }
            } else {
                ui_draw_text("Seleccion actual:", layout.sidebar.x + 12.0f, info_y,
                             14.0f, 0.16f, (Color){66, 76, 86, 255}, false);
                ui_draw_text(estructura_nombre(app.estructura_activa), layout.sidebar.x + 12.0f,
                             info_y + 20.0f, title_size, 0.12f, (Color){28, 52, 76, 255}, true);
                ui_draw_text(TextFormat("Elementos: %d", cantidad_activa), layout.sidebar.x + 12.0f,
                             info_y + 50.0f, meta_size, 0.2f, (Color){66, 76, 86, 255}, false);

                if (available_info_h > 96.0f) {
                    ui_draw_text("Puedes editar los campos o usar atajos.", layout.sidebar.x + 12.0f,
                                 help_y, help_size, 0.10f, (Color){66, 76, 86, 255}, false);
                    ui_draw_text("Navegacion: H menu | TAB sig. | 1..7 estructura", layout.sidebar.x + 12.0f,
                                 nav_y, nav_size, 0.10f, (Color){76, 91, 110, 255}, false);
                } else if (available_info_h > 80.0f) {
                    ui_draw_text("H menu | TAB | 1..7", layout.sidebar.x + 12.0f, nav_y,
                                 compact_sidebar ? 10.0f : 11.0f, 0.10f,
                                 (Color){76, 91, 110, 255}, false);
                }
            }

            if (show_value_input &&
                ui_input_box(value_box, "Valor", value_text, input_focus == INPUT_VALOR,
                             value_invalid)) {
                if (input_focus == INPUT_PRIORIDAD && !priority_invalid) {
                    apply_input_focus(&app, INPUT_PRIORIDAD, priority_text);
                } else if (input_focus == INPUT_GRAFO_ORIGEN && !graph_origin_invalid) {
                    apply_input_focus(&app, INPUT_GRAFO_ORIGEN, graph_origin_text);
                } else if (input_focus == INPUT_GRAFO_DESTINO && !graph_dest_invalid) {
                    apply_input_focus(&app, INPUT_GRAFO_DESTINO, graph_dest_text);
                } else if (input_focus == INPUT_GRAFO_PESO && !graph_weight_invalid) {
                    apply_input_focus(&app, INPUT_GRAFO_PESO, graph_weight_text);
                }
                input_focus = INPUT_VALOR;
            }
            if (show_priority && ui_input_box(priority_box, "Prioridad", priority_text,
                                              input_focus == INPUT_PRIORIDAD, priority_invalid)) {
                if (input_focus == INPUT_VALOR && !value_invalid) {
                    apply_input_focus(&app, INPUT_VALOR, value_text);
                }
                input_focus = INPUT_PRIORIDAD;
            }
            if (show_graph_origin_input &&
                ui_input_box(graph_origin_box,
                             show_graph_recorridos ? "Inicio BFS/DFS"
                                                   : (show_graph_mst ? "Inicio Prim" : "Origen"),
                             graph_origin_text,
                             input_focus == INPUT_GRAFO_ORIGEN, graph_origin_invalid)) {
                if (input_focus == INPUT_VALOR && !value_invalid) {
                    apply_input_focus(&app, INPUT_VALOR, value_text);
                }
                input_focus = INPUT_GRAFO_ORIGEN;
                if (strcmp(graph_origin_text, "-1") == 0) {
                    graph_origin_text[0] = '\0';
                }
            }
            if (show_graph_dest_input &&
                ui_input_box(graph_dest_box, "Destino", graph_dest_text,
                             input_focus == INPUT_GRAFO_DESTINO, graph_dest_invalid)) {
                if (input_focus == INPUT_VALOR && !value_invalid) {
                    apply_input_focus(&app, INPUT_VALOR, value_text);
                }
                input_focus = INPUT_GRAFO_DESTINO;
                if (strcmp(graph_dest_text, "-1") == 0) {
                    graph_dest_text[0] = '\0';
                }
            }
            if (show_graph_weight_input &&
                ui_input_box(graph_weight_box, "Peso", graph_weight_text,
                             input_focus == INPUT_GRAFO_PESO, graph_weight_invalid)) {
                if (input_focus == INPUT_VALOR && !value_invalid) {
                    apply_input_focus(&app, INPUT_VALOR, value_text);
                }
                input_focus = INPUT_GRAFO_PESO;
            }
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                (!show_value_input || !CheckCollisionPointRec(GetMousePosition(), value_box)) &&
                (!show_priority || !CheckCollisionPointRec(GetMousePosition(), priority_box)) &&
                (!show_graph_origin_input ||
                 !CheckCollisionPointRec(GetMousePosition(), graph_origin_box)) &&
                (!show_graph_dest_input ||
                 !CheckCollisionPointRec(GetMousePosition(), graph_dest_box)) &&
                (!show_graph_weight_input ||
                 !CheckCollisionPointRec(GetMousePosition(), graph_weight_box))) {
                if (input_focus == INPUT_VALOR && !value_invalid) {
                    apply_input_focus(&app, INPUT_VALOR, value_text);
                } else if (show_priority && input_focus == INPUT_PRIORIDAD && !priority_invalid) {
                    apply_input_focus(&app, INPUT_PRIORIDAD, priority_text);
                } else if (show_graph_origin_input && input_focus == INPUT_GRAFO_ORIGEN &&
                           !graph_origin_invalid) {
                    apply_input_focus(&app, INPUT_GRAFO_ORIGEN, graph_origin_text);
                } else if (show_graph_dest_input && input_focus == INPUT_GRAFO_DESTINO &&
                           !graph_dest_invalid) {
                    apply_input_focus(&app, INPUT_GRAFO_DESTINO, graph_dest_text);
                } else if (show_graph_weight_input && input_focus == INPUT_GRAFO_PESO &&
                           !graph_weight_invalid) {
                    apply_input_focus(&app, INPUT_GRAFO_PESO, graph_weight_text);
                }
                input_focus = INPUT_NONE;
            }
            if (!show_value_input && input_focus == INPUT_VALOR) {
                input_focus = INPUT_NONE;
            }
            if (show_priority == false && input_focus == INPUT_PRIORIDAD) {
                input_focus = INPUT_NONE;
            }
            if (!show_graph_origin_input && input_focus == INPUT_GRAFO_ORIGEN) {
                input_focus = INPUT_NONE;
            }
            if (!show_graph_dest_input && input_focus == INPUT_GRAFO_DESTINO) {
                input_focus = INPUT_NONE;
            }
            if (!show_graph_weight_input && input_focus == INPUT_GRAFO_PESO) {
                input_focus = INPUT_NONE;
            }
            if (show_graph_inputs == false &&
                (input_focus == INPUT_GRAFO_ORIGEN || input_focus == INPUT_GRAFO_DESTINO ||
                 input_focus == INPUT_GRAFO_PESO)) {
                input_focus = INPUT_NONE;
            }
        }
        }
        if (value_invalid && !(app.estructura_activa == ESTRUCTURA_GRAFO && g_grafo_ui_mode != 0)) {
            ui_draw_text("Valor invalido", value_box.x, value_box.y + 42.0f, 12.0f, 0.2f,
                         (Color){176, 54, 44, 255}, false);
        }
        if (app.estructura_activa == ESTRUCTURA_COLA_PRIORIDAD && priority_invalid) {
            ui_draw_text("Prioridad: 1..99", priority_box.x, priority_box.y + 42.0f, 12.0f,
                         0.2f, (Color){176, 54, 44, 255}, false);
        }
        if (app.estructura_activa == ESTRUCTURA_GRAFO && graph_origin_invalid) {
            ui_draw_text("Origen invalido", graph_origin_box.x, graph_origin_box.y + 42.0f,
                         12.0f, 0.2f, (Color){176, 54, 44, 255}, false);
        }
        if (app.estructura_activa == ESTRUCTURA_GRAFO && graph_dest_invalid) {
            if (g_grafo_ui_mode == 0 || g_grafo_ui_mode == 2) {
                ui_draw_text("Destino invalido", graph_dest_box.x, graph_dest_box.y + 42.0f, 12.0f,
                             0.2f, (Color){176, 54, 44, 255}, false);
            }
        }
        if (app.estructura_activa == ESTRUCTURA_GRAFO && graph_weight_invalid) {
            if (g_grafo_ui_mode == 0 || g_grafo_ui_mode == 2) {
                ui_draw_text("Peso: -999..999", graph_weight_box.x, graph_weight_box.y + 42.0f, 12.0f,
                             0.2f, (Color){176, 54, 44, 255}, false);
            }
        }

        ui_draw_panel(layout.center, "Representacion Grafica");
        {
            bool compact_mode = false;
            float view_top;

            view_top = draw_context_controls(&app, layout.center, &compact_mode);

            draw_active_view(&app, layout.center, view_top);
        }

        {
        ui_draw_panel(layout.right,
                      app.estructura_activa == ESTRUCTURA_GRAFO ? "Resumen pedagogico"
                                                                 : "Codigo C Asociado");
        DrawRectangleRounded((Rectangle){layout.right.x + 14.0f, layout.right.y + 40.0f,
                                         layout.right.width - 28.0f, 52.0f},
                            0.20f, 8, Fade((Color){224, 235, 248, 255}, 0.88f));
        DrawRectangleRoundedLinesEx((Rectangle){layout.right.x + 14.0f, layout.right.y + 40.0f,
                                                layout.right.width - 28.0f, 52.0f},
                                    0.20f, 8, 1.2f, Fade((Color){42, 98, 158, 255}, 0.30f));
        ui_draw_text(TextFormat("Estructura: %s", estructura_nombre(app.estructura_activa)),
                    layout.right.x + 20.0f, layout.right.y + 48.0f, 13.0f, 0.10f,
                    (Color){34, 52, 76, 255}, false);
        ui_draw_text(TextFormat("T: %s | E: %s", tiempo_texto, espacio_texto),
                    layout.right.x + 20.0f, layout.right.y + 66.0f, 12.0f, 0.08f,
                    (Color){64, 76, 95, 255}, false);

        if (app.estructura_activa != ESTRUCTURA_GRAFO &&
            ui_button((Rectangle){layout.right.x + layout.right.width - 136.0f, layout.right.y + 45.0f,
                                  116.0f, 26.0f},
                      code_panel_compact ? "Expandir" : "Compacto", false)) {
            code_panel_compact = !code_panel_compact;
            code_scroll = 0.0f;
        }
        if (app.estructura_activa != ESTRUCTURA_GRAFO && !code_panel_compact &&
            ui_button((Rectangle){layout.right.x + layout.right.width - 136.0f, layout.right.y + 73.0f,
                                  116.0f, 26.0f},
                      "Limpiar", false)) {
            code_history[0] = '\0';
            code_history_entries = 0;
            code_history_last_serial = app.operacion_serial;
            code_scroll = 0.0f;
            code_display_text = snippet;
        }

        if (app.estructura_activa == ESTRUCTURA_GRAFO) {
            Rectangle result_box = {layout.right.x + 14.0f, layout.right.y + 102.0f,
                                    layout.right.width - 28.0f, layout.right.height - 118.0f};
            char detalle_text[768];
            char objetivo_text[160];
            char entrada_text[160];
            char estado_text[128];
            char resultado_text[1024];
            char recorrido_lista[640];
            char mst_aristas[768];
            int costo_total = 0;
            int costo_visible = 0;
            int i;
            int peso_directo = 0;
            bool hay_peso_directo = false;
            Rectangle result_viewport;
            float result_content_height;
            int paso_visible = app.grafo_controller_state.total_pasos > 0
                                   ? app.grafo_controller_state.paso_actual + 1
                                   : 0;
            int vertices_visibles = app.grafo_controller_state.script_vertices_count;
            int aristas_visibles = app.grafo_controller_state.script_aristas_count;

            for (i = 0; i < app.grafo_controller_state.script_aristas_count; i++) {
                costo_total += app.grafo_controller_state.script_aristas[i].peso;
            }
            if (paso_visible > 0) {
                if (vertices_visibles > paso_visible) {
                    vertices_visibles = paso_visible;
                }
                if (aristas_visibles > paso_visible) {
                    aristas_visibles = paso_visible;
                }
            }
            if (vertices_visibles < 0) {
                vertices_visibles = 0;
            }
            if (aristas_visibles < 0) {
                aristas_visibles = 0;
            }
            for (i = 0; i < aristas_visibles; i++) {
                costo_visible += app.grafo_controller_state.script_aristas[i].peso;
            }
            grafo_formatear_orden_vertices(&app.grafo_controller_state, detalle_text,
                                           sizeof(detalle_text));
            if (detalle_text[0] == '\0') {
                snprintf(detalle_text, sizeof(detalle_text), "-");
            }

            if (strstr(app.mensaje_operacion, "sin camino") != NULL) {
                snprintf(estado_text, sizeof(estado_text), "Estado: Sin camino");
            } else if (strstr(app.mensaje_operacion, "ciclo negativo") != NULL) {
                snprintf(estado_text, sizeof(estado_text), "Estado: Ciclo negativo");
            } else if (app.ultima_operacion_ok) {
                snprintf(estado_text, sizeof(estado_text), "Estado: OK");
            } else {
                snprintf(estado_text, sizeof(estado_text), "Estado: Pendiente");
            }

            if (g_grafo_ui_mode == 1) {
                size_t used = 0;
                int limite = vertices_visibles;
                if (limite > 24) {
                    limite = 24;
                }
                recorrido_lista[0] = '\0';
                if (limite <= 0) {
                    snprintf(recorrido_lista, sizeof(recorrido_lista), "Sin recorrido");
                } else {
                    for (i = 0; i < limite; i++) {
                        int w = snprintf(recorrido_lista + used, sizeof(recorrido_lista) - used,
                                         "%d) V%d%s",
                                         i + 1,
                                         app.grafo_controller_state.script_vertices[i],
                                         (i + 1 < limite) ? "\n" : "");
                        if (w <= 0 || (size_t)w >= sizeof(recorrido_lista) - used) {
                            break;
                        }
                        used += (size_t)w;
                    }
                    if (vertices_visibles > limite && used + 5 < sizeof(recorrido_lista)) {
                        snprintf(recorrido_lista + used, sizeof(recorrido_lista) - used, "\n...");
                    }
                }
                snprintf(objetivo_text, sizeof(objetivo_text),
                         "Objetivo: recorrer vertices desde un inicio");
                snprintf(entrada_text, sizeof(entrada_text), "Inicio: V%d | Paso: %d/%d",
                         app.grafo_vertice_inicio, paso_visible,
                         app.grafo_controller_state.total_pasos);
                snprintf(resultado_text, sizeof(resultado_text), "Orden:\n%s", recorrido_lista);
            } else if (g_grafo_ui_mode == 2) {
                size_t used = 0;
                int limite = vertices_visibles;
                if (limite > 24) {
                    limite = 24;
                }
                recorrido_lista[0] = '\0';
                if (limite <= 0) {
                    snprintf(recorrido_lista, sizeof(recorrido_lista), "Sin ruta");
                } else {
                    for (i = 0; i < limite; i++) {
                        int w = snprintf(recorrido_lista + used, sizeof(recorrido_lista) - used,
                                         "%d) V%d%s",
                                         i + 1,
                                         app.grafo_controller_state.script_vertices[i],
                                         (i + 1 < limite) ? "\n" : "");
                        if (w <= 0 || (size_t)w >= sizeof(recorrido_lista) - used) {
                            break;
                        }
                        used += (size_t)w;
                    }
                    if (vertices_visibles > limite && used + 5 < sizeof(recorrido_lista)) {
                        snprintf(recorrido_lista + used, sizeof(recorrido_lista) - used, "\n...");
                    }
                }
                hay_peso_directo =
                    grafo_obtener_peso(app.grafo, app.grafo_vertice_inicio,
                                       app.grafo_vertice_destino, &peso_directo) == GRAFO_OK;
                snprintf(objetivo_text, sizeof(objetivo_text),
                         "Objetivo: encontrar camino minimo");
                snprintf(entrada_text, sizeof(entrada_text), "Origen: V%d  Destino: V%d | Paso: %d/%d",
                         app.grafo_vertice_inicio, app.grafo_vertice_destino, paso_visible,
                         app.grafo_controller_state.total_pasos);
                if (hay_peso_directo) {
                    snprintf(resultado_text, sizeof(resultado_text),
                             "Ruta:\n%s\nCosto parcial: %d\nPeso directo: %d", recorrido_lista,
                             costo_visible, peso_directo);
                } else {
                    snprintf(resultado_text, sizeof(resultado_text),
                             "Ruta:\n%s\nCosto parcial: %d\nSin arista directa V%d->V%d",
                              recorrido_lista,
                              costo_visible,
                              app.grafo_vertice_inicio,
                              app.grafo_vertice_destino);
                }
            } else if (g_grafo_ui_mode == 3) {
                size_t used = 0;
                int limite = aristas_visibles;
                if (limite > 24) {
                    limite = 24;
                }
                mst_aristas[0] = '\0';
                if (limite <= 0) {
                    snprintf(mst_aristas, sizeof(mst_aristas), "Sin aristas seleccionadas");
                } else {
                    for (i = 0; i < limite; i++) {
                        int w = snprintf(mst_aristas + used, sizeof(mst_aristas) - used,
                                         "%d) V%d - V%d (peso=%d)%s",
                                         i + 1,
                                         app.grafo_controller_state.script_aristas[i].origen,
                                         app.grafo_controller_state.script_aristas[i].destino,
                                         app.grafo_controller_state.script_aristas[i].peso,
                                         (i + 1 < limite) ? "\n" : "");
                        if (w <= 0 || (size_t)w >= sizeof(mst_aristas) - used) {
                            break;
                        }
                        used += (size_t)w;
                    }
                    if (app.grafo_controller_state.script_aristas_count > limite &&
                        used + 5 < sizeof(mst_aristas)) {
                        snprintf(mst_aristas + used, sizeof(mst_aristas) - used, "\n...");
                    }
                }
                snprintf(objetivo_text, sizeof(objetivo_text),
                         "Objetivo: construir arbol de expansion minima");
                snprintf(entrada_text, sizeof(entrada_text), "Algoritmo: %s | Paso: %d/%d",
                         grafo_algoritmo_home_nombre(app.grafo_algoritmo_seleccionado),
                         paso_visible, app.grafo_controller_state.total_pasos);
                snprintf(resultado_text, sizeof(resultado_text),
                         "Aristas seleccionadas: %d\nCosto parcial: %d\nLista de aristas:\n%s",
                         aristas_visibles, costo_visible, mst_aristas);
            } else {
                snprintf(objetivo_text, sizeof(objetivo_text), "Modo construccion");
                snprintf(entrada_text, sizeof(entrada_text), "Vertices: %d  Aristas: %d  Tipo: %s",
                         app.grafo_controller_state.estado_visual.cantidad_vertices,
                         app.grafo_controller_state.estado_visual.cantidad_aristas,
                         app.grafo_dirigido ? "dirigido" : "no dirigido");
                snprintf(resultado_text, sizeof(resultado_text),
                         "Siguiente paso:\n1) Agregar vertices\n2) Conectar aristas");
            }

            bool compact_construccion = app.estructura_activa == ESTRUCTURA_GRAFO && g_grafo_ui_mode == 0;

            DrawRectangleRounded(result_box, 0.16f, 8, Fade(WHITE, 0.74f));
            DrawRectangleRoundedLinesEx(result_box, 0.16f, 8, 1.0f,
                                        Fade((Color){42, 98, 158, 255}, 0.20f));
            ui_draw_text(compact_construccion ? "Resumen de construccion" : "Resumen de ejecucion",
                         result_box.x + 10.0f, result_box.y + 8.0f,
                         15.0f, 0.10f, (Color){24, 46, 76, 255}, true);

            if (compact_construccion) {
                ui_draw_text(estado_text, result_box.x + 10.0f, result_box.y + 32.0f, 14.0f, 0.08f,
                             (Color){56, 72, 92, 255}, false);
                ui_draw_text(entrada_text, result_box.x + 10.0f, result_box.y + 50.0f, 14.0f, 0.08f,
                             (Color){56, 72, 92, 255}, false);
                result_viewport = (Rectangle){result_box.x + 10.0f, result_box.y + 72.0f,
                                              result_box.width - 20.0f, result_box.height - 88.0f};
            } else {
                ui_draw_text(objetivo_text,
                             result_box.x + 10.0f, result_box.y + 28.0f, 14.0f, 0.08f,
                             (Color){56, 72, 92, 255}, false);
                ui_draw_text(estado_text, result_box.x + 10.0f, result_box.y + 44.0f, 14.0f, 0.08f,
                             (Color){56, 72, 92, 255}, false);
                ui_draw_text(entrada_text, result_box.x + 10.0f, result_box.y + 60.0f, 14.0f, 0.08f,
                             (Color){56, 72, 92, 255}, false);
                ui_draw_text("Resultado:", result_box.x + 10.0f, result_box.y + 78.0f, 14.0f, 0.08f,
                             (Color){36, 58, 86, 255}, true);
                result_viewport = (Rectangle){result_box.x + 10.0f, result_box.y + 96.0f,
                                              result_box.width - 20.0f, result_box.height - 114.0f};
            }
            result_content_height = count_text_lines(resultado_text) * 20.0f;
            graph_result_scroll = clamp_float(
                graph_result_scroll, 0.0f,
                result_content_height > result_viewport.height
                    ? result_content_height - result_viewport.height
                    : 0.0f);
            draw_scrollable_multiline_text(
                resultado_text, result_viewport, 15, (Color){44, 58, 74, 255},
                graph_result_scroll);
            draw_scrollbar((Rectangle){result_box.x + result_box.width - 8.0f, result_viewport.y,
                                       6.0f, result_viewport.height},
                           result_content_height, result_viewport.height, graph_result_scroll);
        } else if (code_panel_compact) {
            char preview_text[640];
            Rectangle compact_box = {layout.right.x + 14.0f, layout.right.y + 102.0f,
                                     layout.right.width - 28.0f, layout.right.height - 118.0f};
            float preview_h = compact_box.height - 94.0f;
            build_compact_preview(snippet, preview_text, sizeof(preview_text), 7);
            DrawRectangleRounded(compact_box, 0.16f, 8, Fade(WHITE, 0.74f));
            DrawRectangleRoundedLinesEx(compact_box, 0.16f, 8, 1.0f,
                                        Fade((Color){42, 98, 158, 255}, 0.20f));
            ui_draw_text("Resumen rapido", compact_box.x + 10.0f, compact_box.y + 8.0f,
                         13.0f, 0.10f, (Color){24, 46, 76, 255}, true);
            if (app.estructura_activa == ESTRUCTURA_GRAFO) {
                ui_draw_text(TextFormat("Algoritmo: %s",
                                        grafo_algoritmo_home_nombre(app.grafo_algoritmo_seleccionado)),
                             compact_box.x + 10.0f, compact_box.y + 26.0f, 12.0f, 0.08f,
                             (Color){56, 72, 92, 255}, false);
            }
            ui_draw_text(TextFormat("Operacion: %s", app.mensaje_operacion),
                         compact_box.x + 10.0f, compact_box.y + 42.0f, 12.0f, 0.08f,
                         (Color){56, 72, 92, 255}, false);
            if (preview_h < 60.0f) {
                preview_h = 60.0f;
            }
            draw_scrollable_multiline_text(preview_text,
                                           (Rectangle){compact_box.x + 10.0f, compact_box.y + 62.0f,
                                                        compact_box.width - 20.0f,
                                                        preview_h},
                                           15, (Color){44, 58, 74, 255}, 0.0f);
            if (compact_box.height > 118.0f) {
                ui_draw_text("Tip: pulsa Expandir para ver historial y desplazamiento.",
                             compact_box.x + 10.0f, compact_box.y + compact_box.height - 18.0f,
                             11.0f, 0.08f, (Color){88, 102, 120, 255}, false);
            }
        } else if (app.estructura_activa == ESTRUCTURA_GRAFO) {
            GrafoCodigoAlgoritmo codigo_grafo =
                grafo_codigo_actual(app.grafo_algoritmo_seleccionado);
            int linea_activa = grafo_linea_desde_paso(app.grafo_algoritmo_seleccionado,
                                                      app.grafo_controller_state.paso_actual,
                                                      app.grafo_controller_state.total_pasos);
            float graph_code_content_height;
            float graph_code_viewport_height = layout.right.height - 118.0f;
            grafo_codigo_establecer_linea_actual(&codigo_grafo, linea_activa);
            graph_code_content_height = 30.0f + codigo_grafo.cantidad_lineas * 20.0f;
            code_scroll = clamp_float(code_scroll, 0.0f,
                                      graph_code_content_height > graph_code_viewport_height
                                          ? graph_code_content_height - graph_code_viewport_height
                                          : 0.0f);
            grafo_codigo_dibujar_con_scroll(
                &codigo_grafo,
                (Rectangle){layout.right.x + 14.0f, layout.right.y + 102.0f,
                            layout.right.width - 28.0f - 10.0f, layout.right.height - 118.0f},
                code_scroll);
            draw_scrollbar((Rectangle){layout.right.x + layout.right.width - 12.0f,
                                       layout.right.y + 102.0f, 8.0f, layout.right.height - 118.0f},
                           graph_code_content_height, layout.right.height - 118.0f, code_scroll);
        } else {
            draw_scrollable_multiline_text(code_display_text,
                                           (Rectangle){layout.right.x + 14.0f, layout.right.y + 102.0f,
                                                       layout.right.width - 28.0f - 10.0f,
                                                       layout.right.height - 118.0f},
                                           16, (Color){40, 52, 64, 255}, code_scroll);
            draw_scrollbar((Rectangle){layout.right.x + layout.right.width - 12.0f,
                                       layout.right.y + 102.0f, 8.0f, layout.right.height - 118.0f},
                           count_text_lines(code_display_text) * 22.0f, layout.right.height - 118.0f,
                           code_scroll);
        }

        ui_draw_panel(layout.bottom, app.estructura_activa == ESTRUCTURA_GRAFO
                                       ? "Traza y progreso"
                                       : "Operacion, Traza y Complejidad");
        {
            bool graph_active = app.estructura_activa == ESTRUCTURA_GRAFO;
            float summary_w = (layout.bottom.width < 980.0f ? 260.0f : 302.0f);
            float summary_line_step;
            float summary_text_y;
            float bottom_inner_top = graph_active ? 30.0f : 40.0f;
            float bottom_inner_margin = graph_active ? 36.0f : 52.0f;
            Rectangle summary_box = {layout.bottom.x + 14.0f, layout.bottom.y + bottom_inner_top, summary_w,
                                     layout.bottom.height - bottom_inner_margin};
            Rectangle trace_box = graph_active
                                      ? (Rectangle){layout.bottom.x + 14.0f, layout.bottom.y + bottom_inner_top,
                                                    layout.bottom.width - 24.0f,
                                                    layout.bottom.height - bottom_inner_margin}
                                      : (Rectangle){summary_box.x + summary_box.width + 10.0f,
                                                    layout.bottom.y + bottom_inner_top,
                                                    layout.bottom.width - summary_box.width - 24.0f - 10.0f,
                                                    layout.bottom.height - bottom_inner_margin};

            if (!graph_active) {
                DrawRectangleRounded(summary_box, 0.16f, 8, Fade((Color){232, 240, 250, 255}, 0.88f));
                DrawRectangleRoundedLinesEx(summary_box, 0.16f, 8, 1.2f,
                                            Fade((Color){42, 98, 158, 255}, 0.28f));
                DrawRectangleRounded((Rectangle){summary_box.x + 12.0f, summary_box.y + 8.0f,
                                                 92.0f, 22.0f},
                                     0.35f, 8, Fade(status_color, 0.18f));
                DrawRectangleRoundedLinesEx((Rectangle){summary_box.x + 12.0f, summary_box.y + 8.0f,
                                                        92.0f, 22.0f},
                                            0.35f, 8, 1.5f, Fade(status_color, 0.65f));
                ui_draw_text(status_label, summary_box.x + 32.0f, summary_box.y + 11.0f, 12.0f, 0.12f,
                             status_color, false);
                summary_line_step = summary_box.height < 112.0f ? 13.0f : 15.0f;
                summary_text_y = summary_box.y + 34.0f;

                ui_draw_text("Resumen", summary_box.x + 12.0f, summary_text_y, 12.0f, 0.10f,
                             (Color){24, 46, 76, 255}, true);
                ui_draw_text(TextFormat("Estructura: %s", estructura_nombre(app.estructura_activa)),
                             summary_box.x + 12.0f, summary_text_y + summary_line_step, 12.0f, 0.10f,
                             (Color){42, 54, 70, 255}, false);
                ui_draw_text(TextFormat("Elementos: %d", cantidad_activa), summary_box.x + 12.0f,
                             summary_text_y + summary_line_step * 2.0f, 12.0f, 0.10f,
                             (Color){42, 54, 70, 255}, false);
                ui_draw_text(TextFormat("Complejidad: T=%s | E=%s", tiempo_texto, espacio_texto),
                             summary_box.x + 12.0f, summary_text_y + summary_line_step * 3.0f,
                             11.0f, 0.08f, (Color){52, 66, 84, 255}, false);
                ui_draw_text("Tip: usa pasos para inspeccionar cada transicion.",
                             summary_box.x + 12.0f, summary_text_y + summary_line_step * 4.0f,
                             11.0f, 0.08f, (Color){84, 96, 112, 255}, false);
            }

            DrawRectangleRounded(trace_box, 0.16f, 8, Fade(WHITE, 0.50f));
            DrawRectangleRoundedLinesEx(trace_box, 0.16f, 8, 1.2f,
                                        Fade((Color){42, 98, 158, 255}, 0.20f));
            if (!graph_active || trace_box.height >= 86.0f) {
                ui_draw_text(graph_active && g_grafo_ui_mode == 0 ? "Estado de construccion"
                                                                  : "Traza esencial",
                             trace_box.x + 10.0f, trace_box.y + 8.0f, 13.0f, 0.12f,
                             (Color){24, 46, 76, 255}, true);
            }

            if (graph_active) {
                char paso_label[64];
                char estado_label[32];
                char resumen_compacto[192];
                char lista_vertices[640];
                bool mostrar_lista_recorrido =
                    app.grafo_algoritmo_seleccionado == GRAFO_ALGO_BFS ||
                    app.grafo_algoritmo_seleccionado == GRAFO_ALGO_DFS;
                bool graph_mode_construccion = g_grafo_ui_mode == 0;
                bool compact_trace = trace_box.height < 112.0f;
                bool tiny_trace = trace_box.height < 86.0f;
                float progress_ratio = 0.0f;
                char progress_text[24];
                Rectangle bar_track;
                Rectangle bar_fill;
                Rectangle lista_box;
                float progress_bar_top = trace_box.y + trace_box.height - 20.0f;
                float y_base = tiny_trace ? (trace_box.y + 8.0f) : (trace_box.y + 30.0f);
                snprintf(paso_label, sizeof(paso_label), "Paso: %d/%d",
                         app.grafo_controller_state.total_pasos > 0
                             ? app.grafo_controller_state.paso_actual + 1
                             : 0,
                         app.grafo_controller_state.total_pasos);
                snprintf(estado_label, sizeof(estado_label), "Auto: %s",
                         app.grafo_controller_state.autoplay_activo ? "ON" : "OFF");
                if (graph_mode_construccion) {
                    float line1_y = trace_box.y + 18.0f;
                    float line2_y = line1_y + 18.0f;
                    float line_min_y = trace_box.y + 10.0f;
                    float max_line2_y = progress_bar_top - 16.0f;
                    bool draw_two_lines = (line2_y <= max_line2_y);

                    if (line1_y < line_min_y) {
                        line1_y = line_min_y;
                    }
                    if (!draw_two_lines) {
                        line1_y = max_line2_y - 10.0f;
                        if (line1_y < line_min_y) {
                            line1_y = line_min_y;
                        }
                    }

                    snprintf(resumen_compacto, sizeof(resumen_compacto),
                             "Construccion | Vertices: %d | Aristas: %d | %s",
                             app.grafo_controller_state.estado_visual.cantidad_vertices,
                             app.grafo_controller_state.estado_visual.cantidad_aristas,
                             estado_label);
                    ui_draw_text(resumen_compacto, trace_box.x + 10.0f, line1_y,
                                 12.0f, 0.08f, (Color){56, 72, 92, 255}, false);
                    if (draw_two_lines) {
                        ui_draw_text("Flujo sugerido: Inicializar -> Vertices -> Aristas",
                                     trace_box.x + 10.0f, line2_y,
                                     11.0f, 0.08f, (Color){76, 91, 110, 255}, false);
                    }
                } else if (compact_trace) {
                    const char *msg = app.mensaje_operacion;
                    char operacion_corta[64];
                    size_t msg_len = strlen(msg);
                    if (msg_len > 40) {
                        snprintf(operacion_corta, sizeof(operacion_corta), "%.37s...", msg);
                    } else {
                        snprintf(operacion_corta, sizeof(operacion_corta), "%.63s", msg);
                    }
                    snprintf(resumen_compacto, sizeof(resumen_compacto), "%s | %s | %s",
                             operacion_corta, paso_label, estado_label);
                    ui_draw_text(resumen_compacto, trace_box.x + 10.0f, trace_box.y + 24.0f,
                                 15.0f, 0.08f, (Color){56, 72, 92, 255}, false);
                } else {
                    ui_draw_text(TextFormat("Operacion: %s", app.mensaje_operacion),
                                 trace_box.x + 10.0f, y_base, 12.0f, 0.08f,
                                 (Color){56, 72, 92, 255}, false);
                    ui_draw_text(paso_label, trace_box.x + 10.0f, y_base + 18.0f, 12.0f, 0.08f,
                                 (Color){56, 72, 92, 255}, false);
                    ui_draw_text(estado_label, trace_box.x + 10.0f, y_base + 34.0f, 12.0f, 0.08f,
                                 (Color){56, 72, 92, 255}, false);
                }

                if (!graph_mode_construccion && !compact_trace && mostrar_lista_recorrido &&
                    trace_box.height >= 132.0f) {
                    grafo_formatear_orden_vertices_lista(&app.grafo_controller_state, lista_vertices,
                                                         sizeof(lista_vertices), 12);
                    lista_box = (Rectangle){trace_box.x + 10.0f, y_base + 52.0f,
                                            trace_box.width - 20.0f, trace_box.height - 130.0f};
                    if (lista_box.height > 44.0f) {
                        DrawRectangleRounded(lista_box, 0.12f, 8,
                                             Fade((Color){232, 240, 250, 255}, 0.65f));
                        DrawRectangleRoundedLinesEx(lista_box, 0.12f, 8, 1.0f,
                                                    Fade((Color){42, 98, 158, 255}, 0.28f));
                        ui_draw_text("Vertices del recorrido", lista_box.x + 8.0f,
                                     lista_box.y + 6.0f, 11.0f, 0.08f,
                                     (Color){24, 46, 76, 255}, true);
                        draw_scrollable_multiline_text(
                            lista_vertices,
                            (Rectangle){lista_box.x + 8.0f, lista_box.y + 22.0f,
                                        lista_box.width - 16.0f, lista_box.height - 26.0f},
                            12, (Color){56, 72, 92, 255}, 0.0f);
                    }
                }

                if (app.grafo_controller_state.total_pasos > 0) {
                    progress_ratio = (float)(app.grafo_controller_state.paso_actual + 1) /
                                     (float)app.grafo_controller_state.total_pasos;
                }
                if (graph_mode_construccion) {
                    int v = app.grafo_controller_state.estado_visual.cantidad_vertices;
                    int a = app.grafo_controller_state.estado_visual.cantidad_aristas;
                    if (v <= 0) {
                        progress_ratio = 0.0f;
                    } else if (a <= 0) {
                        progress_ratio = 0.5f;
                    } else {
                        progress_ratio = 1.0f;
                    }
                }
                if (progress_ratio < 0.0f) {
                    progress_ratio = 0.0f;
                }
                if (progress_ratio > 1.0f) {
                    progress_ratio = 1.0f;
                }

                snprintf(progress_text, sizeof(progress_text), "%d%%",
                         (int)(progress_ratio * 100.0f + 0.5f));
                bar_track = (Rectangle){trace_box.x + 10.0f, progress_bar_top,
                                        trace_box.width - 20.0f, 10.0f};
                bar_fill = (Rectangle){bar_track.x, bar_track.y, bar_track.width * progress_ratio,
                                       bar_track.height};
                DrawRectangleRounded(bar_track, 0.35f, 8, (Color){214, 226, 240, 255});
                DrawRectangleRounded(bar_fill, 0.35f, 8, (Color){39, 128, 224, 255});
                ui_draw_text(progress_text, bar_track.x + bar_track.width - 24.0f,
                             bar_track.y - 12.0f, 11.0f, 0.08f, (Color){56, 72, 92, 255}, false);
            } else {
                char pasos_preview[512];
                build_compact_preview(info.pasos, pasos_preview, sizeof(pasos_preview), 6);
                ui_draw_text(TextFormat("Operacion: %s", app.mensaje_operacion), trace_box.x + 10.0f,
                             trace_box.y + 30.0f, 12.0f, 0.08f, (Color){56, 72, 92, 255}, false);
                draw_scrollable_multiline_text(
                    pasos_preview,
                    (Rectangle){trace_box.x + 10.0f, trace_box.y + 52.0f, trace_box.width - 20.0f,
                                trace_box.height - 76.0f},
                    16, (Color){48, 60, 76, 255}, 0.0f);
            }
        }
        }

        EndDrawing();
        if (e2e_visual_after_frame(&e2e_visual, &app, &screen_mode, &home_selected,
                                   &home_activate)) {
            break;
        }
    }

    app_state_shutdown(&app);
    ui_unload(&ui);
    CloseWindow();

    return 0;
}

