#include "raylib.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
    Rectangle cta = {card.x + 28.0f, card.y + card.height - 62.0f, card.width - 56.0f, 38.0f};
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
    DrawLine((int)card.x + 20, (int)card.y + 168, (int)(card.x + card.width - 20),
             (int)card.y + 168, Fade((Color){44, 92, 153, 255}, 0.18f));

    draw_ui_multiline(description, card.x + 24.0f, card.y + 182.0f, 14.0f, 0.16f,
                      (Color){36, 44, 58, 255});

    return ui_button(cta, "Visualizar", false);
}

/** @brief Dibuja un boton compacto para el panel lateral de grafos. */
static bool draw_graph_sidebar_button(Rectangle bounds, const char *label, bool active) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, bounds);
    int label_width = ui_measure_text(label, 14.0f, 0.08f, false);
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
    ui_draw_text(label, bounds.x + (bounds.width - label_width) * 0.5f, bounds.y + 8.0f,
                 14.0f, 0.08f, active ? (Color){10, 43, 92, 255} : (Color){52, 61, 74, 255}, false);

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
                 18.0f, 0.14f, (Color){53, 66, 83, 255}, false);
    ui_draw_text("Atajos: 1 Secuenciales | 2 Grafos | Enter seleccionar | F1 Ayuda",
                 content.x + 42.0f, content.y + 96.0f, 14.0f, 0.12f,
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
                                       bool *activate_selected) {
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
    int line_height = font_size + 6;
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
            ui_draw_text(line, viewport.x, (float)y, (float)font_size, 0.25f, color, false);
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
    Rectangle area = {panel.x + 12.0f, content_top_y, panel.width - 24.0f,
                      panel.height - (content_top_y - panel.y) - 12.0f};

    if (area.height < 120.0f) {
        area.height = 120.0f;
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
    float base_y = panel.y + 42.0f;
    float gap = 10.0f;
    float btn_h = 36.0f;
    float row_step = btn_h + 8.0f;
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
    } else if (app->estructura_activa == ESTRUCTURA_GRAFO) {
        count = 21;
        hint = "Algoritmo: 4..6/R | Pasos: , . / Home End | Auto: P | Demo: M | Dirigido: T";
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
        case ESTRUCTURA_GRAFO:
            if (i == 0 && ui_button(btn, "Inicializar (I)", false)) {
                app_state_operacion_inicializar(app);
            } else if (i == 1 && ui_button(btn, "Vertice + (A)", false)) {
                app_state_operacion_insertar(app);
            } else if (i == 2 && ui_button(btn, "Vertice - (D)", false)) {
                app_state_operacion_eliminar(app);
            } else if (i == 3 && ui_button(btn, "Arista +", false)) {
                app_state_operacion_grafo_insertar_arista(app, app->grafo_vertice_inicio,
                                                          app->grafo_vertice_destino,
                                                          app->input_prioridad);
            } else if (i == 4 && ui_button(btn, "Arista -", false)) {
                app_state_operacion_grafo_eliminar_arista(app, app->grafo_vertice_inicio,
                                                          app->grafo_vertice_destino);
            } else if (i == 5 && ui_button(btn, "BFS", false)) {
                app_state_operacion_grafo_ejecutar_algoritmo(app, GRAFO_ALGO_BFS,
                                                             app->grafo_vertice_inicio,
                                                             app->grafo_vertice_destino);
            } else if (i == 6 && ui_button(btn, "DFS", false)) {
                app_state_operacion_grafo_ejecutar_algoritmo(app, GRAFO_ALGO_DFS,
                                                             app->grafo_vertice_inicio,
                                                             app->grafo_vertice_destino);
            } else if (i == 7 && ui_button(btn, "Dijkstra", false)) {
                app_state_operacion_grafo_ejecutar_algoritmo(app, GRAFO_ALGO_DIJKSTRA,
                                                             app->grafo_vertice_inicio,
                                                             app->grafo_vertice_destino);
            } else if (i == 8 && ui_button(btn, "Bellman-Ford", false)) {
                app_state_operacion_grafo_ejecutar_algoritmo(app, GRAFO_ALGO_BELLMAN_FORD,
                                                             app->grafo_vertice_inicio,
                                                             app->grafo_vertice_destino);
            } else if (i == 9 && ui_button(btn, "Prim", false)) {
                app_state_operacion_grafo_ejecutar_algoritmo(app, GRAFO_ALGO_PRIM,
                                                             app->grafo_vertice_inicio,
                                                             app->grafo_vertice_destino);
            } else if (i == 10 && ui_button(btn, "Kruskal", false)) {
                app_state_operacion_grafo_ejecutar_algoritmo(app, GRAFO_ALGO_KRUSKAL,
                                                             app->grafo_vertice_inicio,
                                                             app->grafo_vertice_destino);
            } else if (i == 11 && ui_button(btn, "Paso - (,)", false)) {
                grafo_controller_paso_anterior(&app->grafo_controller_state);
            } else if (i == 12 && ui_button(btn, "Paso + (.)", false)) {
                grafo_controller_paso_siguiente(&app->grafo_controller_state);
            } else if (i == 13 && ui_button(btn, "Reiniciar (/)", false)) {
                grafo_controller_reiniciar(&app->grafo_controller_state);
            } else if (i == 14 && ui_button(btn, "Inicio", false)) {
                grafo_controller_ir_inicio(&app->grafo_controller_state);
            } else if (i == 15 && ui_button(btn, "Final", false)) {
                grafo_controller_ir_final(&app->grafo_controller_state);
            } else if (i == 16 && ui_button(btn,
                                            app->grafo_controller_state.autoplay_activo
                                                ? "Auto: ON"
                                                : "Auto: OFF",
                                            false)) {
                grafo_controller_toggle_autoplay(&app->grafo_controller_state);
            } else if (i == 17 && ui_button(btn,
                                            app->grafo_controller_state.autoplay_velocidad_idx == 0
                                                ? "Vel: Lenta"
                                                : app->grafo_controller_state.autoplay_velocidad_idx == 1
                                                      ? "Vel: Media"
                                                      : "Vel: Rapida",
                                            false)) {
                grafo_controller_cambiar_velocidad(&app->grafo_controller_state);
            } else if (i == 18 && ui_button(btn,
                                            app->grafo_dirigido ? "Dirigido" : "No dirigido",
                                            false)) {
                app_state_grafo_toggle_dirigido(app);
            } else if (i == 19 && ui_button(btn, "Cargar demo", false)) {
                app_state_grafo_cargar_demo(app);
            } else if (i == 20 && ui_button(btn, "Exportar", false)) {
                grafo_exportar_resumen_clipboard(app);
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
static void handle_keyboard(AppState *app) {
    if (IsKeyPressed(KEY_UP)) {
        app_state_ajustar_valor(app, 1);
    }
    if (IsKeyPressed(KEY_DOWN)) {
        app_state_ajustar_valor(app, -1);
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        app_state_ajustar_prioridad(app, 1);
    }
    if (IsKeyPressed(KEY_LEFT)) {
        app_state_ajustar_prioridad(app, -1);
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
            app_state_operacion_grafo_insertar_arista(app, app->grafo_vertice_inicio,
                                                      app->grafo_vertice_destino,
                                                      app->input_prioridad);
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
        if (IsKeyPressed(KEY_C)) {
            grafo_exportar_resumen_clipboard(app);
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
        snprintf(graph_weight_text, graph_weight_size, "%d", app->input_prioridad);
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
        app_state_set_prioridad(app, parsed);
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

/** @brief Actualiza el buffer de la caja de entrada activa usando teclado. */
static void edit_active_input(char *buffer, size_t size) {
    int key = GetCharPressed();
    size_t len = strlen(buffer);

    while (key > 0) {
        if ((key >= '0' && key <= '9') || (key == '-' && len == 0)) {
            if (len + 1 < size) {
                buffer[len] = (char)key;
                buffer[len + 1] = '\0';
                len++;
            }
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE) && len > 0) {
        buffer[len - 1] = '\0';
    }
}

int main(void) {
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
    float trace_scroll = 0.0f;
    Color status_color;
    const char *status_label;
    int cantidad_activa;
    bool value_invalid;
    bool priority_invalid;
    bool graph_origin_invalid;
    bool graph_dest_invalid;
    bool graph_weight_invalid;
    ScreenMode screen_mode = SCREEN_HOME_ROOT;
    int parsed_value;
    int parsed_priority;
    int home_selected = 0;
    bool home_activate = false;
    ScreenMode screen_before_help = SCREEN_HOME_ROOT;
    float help_scroll = 0.0f;
    char code_history[CODE_HISTORY_CAPACITY];
    unsigned int code_history_last_serial = 0;
    int code_history_entries = 0;
    const char *code_display_text;

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
    snprintf(graph_weight_text, sizeof(graph_weight_text), "%d", app.input_prioridad);

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
        app_state_update_visuals(&app, GetFrameTime());
        grafo_controller_actualizar(&app.grafo_controller_state, GetFrameTime());
        handle_navigation_keyboard(&screen_mode, &app, &home_selected, &home_activate);
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
            continue;
        }

        if (CheckCollisionPointRec(GetMousePosition(), layout.right) && GetMouseWheelMove() != 0.0f) {
            code_scroll -= GetMouseWheelMove() * 22.0f;
        } else if (CheckCollisionPointRec(GetMousePosition(), layout.bottom) &&
                   GetMouseWheelMove() != 0.0f) {
            trace_scroll -= GetMouseWheelMove() * 22.0f;
        }
        handle_keyboard(&app);
        sync_input_buffers(&app, value_text, sizeof(value_text), priority_text,
                           sizeof(priority_text), graph_origin_text,
                           sizeof(graph_origin_text), graph_dest_text,
                           sizeof(graph_dest_text), graph_weight_text,
                           sizeof(graph_weight_text), input_focus);
        value_invalid = !parse_int_text(value_text, &parsed_value);
        priority_invalid = !parse_int_text(priority_text, &parsed_priority) ||
                           parsed_priority < 1 || parsed_priority > 99;
        graph_origin_invalid = !parse_int_text(graph_origin_text, NULL);
        graph_dest_invalid = !parse_int_text(graph_dest_text, NULL);
        graph_weight_invalid = !parse_int_text(graph_weight_text, &parsed_priority) ||
                       parsed_priority < -999 || parsed_priority > 999;

        if (input_focus == INPUT_VALOR) {
            edit_active_input(value_text, sizeof(value_text));
        } else if (input_focus == INPUT_PRIORIDAD) {
            edit_active_input(priority_text, sizeof(priority_text));
        } else if (input_focus == INPUT_GRAFO_ORIGEN) {
            edit_active_input(graph_origin_text, sizeof(graph_origin_text));
        } else if (input_focus == INPUT_GRAFO_DESTINO) {
            edit_active_input(graph_dest_text, sizeof(graph_dest_text));
        } else if (input_focus == INPUT_GRAFO_PESO) {
            edit_active_input(graph_weight_text, sizeof(graph_weight_text));
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
        }
        code_display_text = (code_history_entries > 0) ? code_history : snippet;
        info = algorithm_trace_get_info(app.estructura_activa, app.operacion_actual);
        cantidad_activa = estructura_cantidad(&app);
        tiempo_texto = info.tiempo;
        espacio_texto = info.espacio;
        if (app.estructura_activa == ESTRUCTURA_GRAFO) {
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
            int trace_lines;
            float code_content_height;
            float trace_content_height;
            float code_viewport_height = layout.right.height - 118.0f;
            float trace_viewport_height = layout.bottom.height - 82.0f;

            snprintf(trace_text, sizeof(trace_text),
                     "Estructura activa: %s\n"
                     "%s\n"
                     "Pasos: %s\n"
                     "Complejidad Tiempo: %s\n"
                     "Complejidad Espacio: %s",
                     estructura_nombre(app.estructura_activa), app.mensaje_operacion, info.pasos,
                     info.tiempo, info.espacio);
            trace_lines = count_text_lines(trace_text);
            code_content_height = code_lines * 22.0f;
            trace_content_height = trace_lines * 24.0f;
            code_scroll = clamp_float(code_scroll, 0.0f,
                                      code_content_height > code_viewport_height
                                          ? code_content_height - code_viewport_height
                                          : 0.0f);
            trace_scroll = clamp_float(trace_scroll, 0.0f,
                                       trace_content_height > trace_viewport_height
                                           ? trace_content_height - trace_viewport_height
                                           : 0.0f);
        }

        BeginDrawing();
        ClearBackground((Color){250, 252, 254, 255});

        ui_draw_header(&ui);
        ui_draw_footer(&ui);

        {
            float sidebar_section_bottom;

            if (app.estructura_activa == ESTRUCTURA_GRAFO) {
                float gap_y = 8.0f;
                float grid_gap_x = 6.0f;
                float cell_w;
                float row_y;
                int action_index;

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

                cell_w = (layout.sidebar.width - 24.0f - grid_gap_x) * 0.5f;
                row_y = layout.sidebar.y + 152.0f;
                for (action_index = 0; action_index < 14; action_index++) {
                    Rectangle graph_btn = {layout.sidebar.x + 12.0f + (float)(action_index % 2) *
                                           (cell_w + grid_gap_x),
                                           row_y + (float)(action_index / 2) * (30.0f + gap_y),
                                           cell_w, 30.0f};

                    if (action_index == 0 &&
                        draw_graph_sidebar_button(graph_btn, "Crear", false)) {
                        app_state_operacion_inicializar(&app);
                    } else if (action_index == 1 &&
                               draw_graph_sidebar_button(graph_btn, "Vertice +", false)) {
                        app_state_operacion_insertar(&app);
                    } else if (action_index == 2 &&
                               draw_graph_sidebar_button(graph_btn, "Vertice -", false)) {
                        app_state_operacion_eliminar(&app);
                    } else if (action_index == 3 &&
                               draw_graph_sidebar_button(graph_btn, "Arista +", false)) {
                        app_state_operacion_grafo_insertar_arista(&app, app.grafo_vertice_inicio,
                                                                  app.grafo_vertice_destino,
                                                                  app.input_prioridad);
                    } else if (action_index == 4 &&
                               draw_graph_sidebar_button(graph_btn, "Arista -", false)) {
                        app_state_operacion_grafo_eliminar_arista(&app, app.grafo_vertice_inicio,
                                                                  app.grafo_vertice_destino);
                    } else if (action_index == 5 &&
                               draw_graph_sidebar_button(graph_btn, "BFS",
                                                         app.grafo_algoritmo_seleccionado ==
                                                             GRAFO_ALGO_BFS)) {
                        app_state_operacion_grafo_ejecutar_algoritmo(&app, GRAFO_ALGO_BFS,
                                                                     app.grafo_vertice_inicio,
                                                                     app.grafo_vertice_destino);
                    } else if (action_index == 6 &&
                               draw_graph_sidebar_button(graph_btn, "DFS",
                                                         app.grafo_algoritmo_seleccionado ==
                                                             GRAFO_ALGO_DFS)) {
                        app_state_operacion_grafo_ejecutar_algoritmo(&app, GRAFO_ALGO_DFS,
                                                                     app.grafo_vertice_inicio,
                                                                     app.grafo_vertice_destino);
                    } else if (action_index == 7 &&
                               draw_graph_sidebar_button(graph_btn, "Dijkstra",
                                                         app.grafo_algoritmo_seleccionado ==
                                                             GRAFO_ALGO_DIJKSTRA)) {
                        app_state_operacion_grafo_ejecutar_algoritmo(&app,
                                                                     GRAFO_ALGO_DIJKSTRA,
                                                                     app.grafo_vertice_inicio,
                                                                     app.grafo_vertice_destino);
                    } else if (action_index == 8 &&
                               draw_graph_sidebar_button(graph_btn, "Bellman",
                                                         app.grafo_algoritmo_seleccionado ==
                                                             GRAFO_ALGO_BELLMAN_FORD)) {
                        app_state_operacion_grafo_ejecutar_algoritmo(&app,
                                                                     GRAFO_ALGO_BELLMAN_FORD,
                                                                     app.grafo_vertice_inicio,
                                                                     app.grafo_vertice_destino);
                    } else if (action_index == 9 &&
                               draw_graph_sidebar_button(graph_btn, "Prim",
                                                         app.grafo_algoritmo_seleccionado ==
                                                             GRAFO_ALGO_PRIM)) {
                        app_state_operacion_grafo_ejecutar_algoritmo(&app, GRAFO_ALGO_PRIM,
                                                                     app.grafo_vertice_inicio,
                                                                     app.grafo_vertice_destino);
                    } else if (action_index == 10 &&
                               draw_graph_sidebar_button(graph_btn, "Kruskal",
                                                         app.grafo_algoritmo_seleccionado ==
                                                             GRAFO_ALGO_KRUSKAL)) {
                        app_state_operacion_grafo_ejecutar_algoritmo(&app,
                                                                     GRAFO_ALGO_KRUSKAL,
                                                                     app.grafo_vertice_inicio,
                                                                     app.grafo_vertice_destino);
                    } else if (action_index == 11 &&
                               draw_graph_sidebar_button(graph_btn, "Demo", false)) {
                        app_state_grafo_cargar_demo(&app);
                    } else if (action_index == 12 &&
                               draw_graph_sidebar_button(graph_btn,
                                                         app.grafo_dirigido ? "Dirigido"
                                                                            : "No dirigido",
                                                         false)) {
                        app_state_grafo_toggle_dirigido(&app);
                    } else if (action_index == 13 &&
                               draw_graph_sidebar_button(graph_btn, "Exportar", false)) {
                        grafo_exportar_resumen_clipboard(&app);
                    }
                }

                sidebar_section_bottom = row_y + 7.0f * (30.0f + gap_y) - gap_y + 10.0f;
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
                }

                sidebar_section_bottom = btn.y + 56.0f;
            }

            DrawLine((int)(layout.sidebar.x + 16.0f), (int)sidebar_section_bottom,
                     (int)(layout.sidebar.x + layout.sidebar.width - 16.0f),
                     (int)sidebar_section_bottom, Fade((Color){78, 110, 146, 255}, 0.28f));

        {
            bool show_priority = app.estructura_activa == ESTRUCTURA_COLA_PRIORIDAD;
            bool show_graph_inputs = app.estructura_activa == ESTRUCTURA_GRAFO;
            float info_y = sidebar_section_bottom + 8.0f;
            float inputs_height = show_graph_inputs ? 192.0f : (show_priority ? 92.0f : 42.0f);
            float inputs_top = layout.sidebar.y + layout.sidebar.height - inputs_height;
            float available_info_h = inputs_top - info_y;
            float help_y = info_y + 70.0f;
            float nav_y = inputs_top - 18.0f;
            bool compact_sidebar = available_info_h < 120.0f;
            float title_size = compact_sidebar ? 20.0f : 22.0f;
            float meta_size = compact_sidebar ? 14.0f : 16.0f;
            float help_size = compact_sidebar ? 11.0f : 12.0f;
            float nav_size = compact_sidebar ? 10.0f : 11.0f;

            value_box = (Rectangle){layout.sidebar.x + 12.0f,
                                    layout.sidebar.y + layout.sidebar.height - inputs_height,
                                    layout.sidebar.width - 24.0f, 38.0f};
            priority_box = (Rectangle){layout.sidebar.x + 12.0f,
                                       layout.sidebar.y + layout.sidebar.height - 42.0f,
                                       layout.sidebar.width - 24.0f, 38.0f};
            graph_origin_box = (Rectangle){layout.sidebar.x + 12.0f,
                                           value_box.y + 50.0f,
                                           layout.sidebar.width - 24.0f, 38.0f};
            graph_dest_box = (Rectangle){layout.sidebar.x + 12.0f,
                                         graph_origin_box.y + 50.0f,
                                         layout.sidebar.width - 24.0f, 38.0f};
            graph_weight_box = (Rectangle){layout.sidebar.x + 12.0f,
                                           graph_dest_box.y + 50.0f,
                                           layout.sidebar.width - 24.0f, 38.0f};

            if (show_graph_inputs) {
                ui_draw_text("Opciones de grafo:", layout.sidebar.x + 12.0f, info_y,
                             14.0f, 0.12f, (Color){66, 76, 86, 255}, false);
                ui_draw_text(TextFormat("Vertices: %d", cantidad_activa), layout.sidebar.x + 12.0f,
                             info_y + 18.0f, meta_size, 0.16f, (Color){66, 76, 86, 255}, false);
                ui_draw_text(TextFormat("Algoritmo: %s",
                                         grafo_algoritmo_home_nombre(app.grafo_algoritmo_seleccionado)),
                             layout.sidebar.x + 12.0f, info_y + 38.0f, meta_size, 0.14f,
                             (Color){28, 52, 76, 255}, true);
                if (available_info_h > 78.0f) {
                    ui_draw_text("Campos: valor, origen, destino y peso.", layout.sidebar.x + 12.0f,
                                 nav_y, help_size, 0.08f, (Color){76, 91, 110, 255}, false);
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

            if (ui_input_box(value_box, "Valor", value_text, input_focus == INPUT_VALOR,
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
            if (show_graph_inputs &&
                ui_input_box(graph_origin_box, "Origen", graph_origin_text,
                             input_focus == INPUT_GRAFO_ORIGEN, graph_origin_invalid)) {
                if (input_focus == INPUT_VALOR && !value_invalid) {
                    apply_input_focus(&app, INPUT_VALOR, value_text);
                }
                input_focus = INPUT_GRAFO_ORIGEN;
            }
            if (show_graph_inputs &&
                ui_input_box(graph_dest_box, "Destino", graph_dest_text,
                             input_focus == INPUT_GRAFO_DESTINO, graph_dest_invalid)) {
                if (input_focus == INPUT_VALOR && !value_invalid) {
                    apply_input_focus(&app, INPUT_VALOR, value_text);
                }
                input_focus = INPUT_GRAFO_DESTINO;
            }
            if (show_graph_inputs &&
                ui_input_box(graph_weight_box, "Peso", graph_weight_text,
                             input_focus == INPUT_GRAFO_PESO, graph_weight_invalid)) {
                if (input_focus == INPUT_VALOR && !value_invalid) {
                    apply_input_focus(&app, INPUT_VALOR, value_text);
                }
                input_focus = INPUT_GRAFO_PESO;
            }
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                !CheckCollisionPointRec(GetMousePosition(), value_box) &&
                (!show_priority || !CheckCollisionPointRec(GetMousePosition(), priority_box)) &&
                (!show_graph_inputs || !CheckCollisionPointRec(GetMousePosition(), graph_origin_box)) &&
                (!show_graph_inputs || !CheckCollisionPointRec(GetMousePosition(), graph_dest_box)) &&
                (!show_graph_inputs || !CheckCollisionPointRec(GetMousePosition(), graph_weight_box))) {
                if (input_focus == INPUT_VALOR && !value_invalid) {
                    apply_input_focus(&app, INPUT_VALOR, value_text);
                } else if (show_priority && input_focus == INPUT_PRIORIDAD && !priority_invalid) {
                    apply_input_focus(&app, INPUT_PRIORIDAD, priority_text);
                } else if (show_graph_inputs && input_focus == INPUT_GRAFO_ORIGEN &&
                           !graph_origin_invalid) {
                    apply_input_focus(&app, INPUT_GRAFO_ORIGEN, graph_origin_text);
                } else if (show_graph_inputs && input_focus == INPUT_GRAFO_DESTINO &&
                           !graph_dest_invalid) {
                    apply_input_focus(&app, INPUT_GRAFO_DESTINO, graph_dest_text);
                } else if (show_graph_inputs && input_focus == INPUT_GRAFO_PESO &&
                           !graph_weight_invalid) {
                    apply_input_focus(&app, INPUT_GRAFO_PESO, graph_weight_text);
                }
                input_focus = INPUT_NONE;
            }
            if (show_priority == false && input_focus == INPUT_PRIORIDAD) {
                input_focus = INPUT_NONE;
            }
            if (show_graph_inputs == false &&
                (input_focus == INPUT_GRAFO_ORIGEN || input_focus == INPUT_GRAFO_DESTINO ||
                 input_focus == INPUT_GRAFO_PESO)) {
                input_focus = INPUT_NONE;
            }
        }
        }
        if (value_invalid) {
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
            ui_draw_text("Destino invalido", graph_dest_box.x, graph_dest_box.y + 42.0f, 12.0f,
                         0.2f, (Color){176, 54, 44, 255}, false);
        }
        if (app.estructura_activa == ESTRUCTURA_GRAFO && graph_weight_invalid) {
            ui_draw_text("Peso: -999..999", graph_weight_box.x, graph_weight_box.y + 42.0f, 12.0f,
                         0.2f, (Color){176, 54, 44, 255}, false);
        }

        ui_draw_panel(layout.center, "Representacion Grafica");
        {
            bool compact_mode = false;
            float view_top = draw_context_controls(&app, layout.center, &compact_mode);

            if (compact_mode) {
                Rectangle compact_badge = {layout.center.x + layout.center.width - 142.0f,
                                           layout.center.y + 10.0f, 118.0f, 22.0f};
                DrawRectangleRounded(compact_badge, 0.30f, 8, Fade((Color){42, 98, 158, 255}, 0.14f));
                DrawRectangleRoundedLinesEx(compact_badge, 0.30f, 8, 1.2f,
                                            Fade((Color){42, 98, 158, 255}, 0.46f));
                ui_draw_text("Modo compacto", compact_badge.x + 10.0f, compact_badge.y + 4.0f,
                             12.0f, 0.10f, (Color){33, 74, 126, 255}, false);
            }

            draw_active_view(&app, layout.center, view_top);
        }

        ui_draw_panel(layout.right, "Codigo C Asociado (Historial)");
        DrawRectangleRounded((Rectangle){layout.right.x + 14.0f, layout.right.y + 40.0f,
                                         layout.right.width - 28.0f, 52.0f},
                             0.20f, 8, Fade((Color){224, 235, 248, 255}, 0.88f));
        DrawRectangleRoundedLinesEx((Rectangle){layout.right.x + 14.0f, layout.right.y + 40.0f,
                                                layout.right.width - 28.0f, 52.0f},
                                    0.20f, 8, 1.2f, Fade((Color){42, 98, 158, 255}, 0.30f));
        ui_draw_text(TextFormat("Estructura: %s", estructura_nombre(app.estructura_activa)),
                     layout.right.x + 20.0f, layout.right.y + 48.0f, 13.0f, 0.12f,
                     (Color){34, 52, 76, 255}, false);
        ui_draw_text(TextFormat("Tiempo: %s | Espacio: %s", tiempo_texto, espacio_texto),
                     layout.right.x + 20.0f, layout.right.y + 66.0f, 12.0f, 0.10f,
                     (Color){64, 76, 95, 255}, false);
        if (ui_button((Rectangle){layout.right.x + layout.right.width - 136.0f, layout.right.y + 51.0f,
                                  116.0f, 30.0f},
                      "Limpiar", false)) {
            code_history[0] = '\0';
            code_history_entries = 0;
            code_history_last_serial = app.operacion_serial;
            code_scroll = 0.0f;
            code_display_text = snippet;
        }
        if (app.estructura_activa == ESTRUCTURA_GRAFO) {
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

        ui_draw_panel(layout.bottom, "Operacion, Traza y Complejidad");
        {
            float summary_w = layout.bottom.width < 980.0f ? 228.0f : 258.0f;
            float summary_line_step;
            float summary_text_y;
            Rectangle summary_box = {layout.bottom.x + 14.0f, layout.bottom.y + 40.0f, summary_w,
                                     layout.bottom.height - 52.0f};
            Rectangle trace_box = {summary_box.x + summary_box.width + 10.0f, layout.bottom.y + 40.0f,
                                   layout.bottom.width - summary_box.width - 24.0f - 10.0f,
                                   layout.bottom.height - 52.0f};

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
            ui_draw_text(TextFormat("T: %s", tiempo_texto), summary_box.x + 12.0f,
                         summary_text_y + summary_line_step * 3.0f, 12.0f, 0.10f,
                         (Color){42, 54, 70, 255}, false);
            ui_draw_text(TextFormat("E: %s", espacio_texto), summary_box.x + 12.0f,
                         summary_text_y + summary_line_step * 4.0f, 12.0f, 0.10f,
                         (Color){42, 54, 70, 255}, false);

            if (app.estructura_activa == ESTRUCTURA_GRAFO &&
                (app.grafo_algoritmo_seleccionado == GRAFO_ALGO_DIJKSTRA ||
                 app.grafo_algoritmo_seleccionado == GRAFO_ALGO_BELLMAN_FORD)) {
                char tabla_multi[512];
                Rectangle dist_box;

                grafo_tabla_distancias_multiline(&app, app.grafo_controller_state.paso_actual,
                                                 tabla_multi, sizeof(tabla_multi));
                dist_box = (Rectangle){summary_box.x + 10.0f,
                                       summary_text_y + summary_line_step * 5.0f + 6.0f,
                                       summary_box.width - 20.0f,
                                       summary_box.height - (summary_text_y - summary_box.y) -
                                           summary_line_step * 5.0f - 16.0f};

                if (dist_box.height > 28.0f) {
                    char camino_label[256];
                    char cerrados_label[192];
                    char progreso_label[32];
                    char tipo_label[64];
                    char reproduccion_label[40];
                    DrawRectangleRounded(dist_box, 0.12f, 8, Fade(WHITE, 0.78f));
                    DrawRectangleRoundedLinesEx(dist_box, 0.12f, 8, 1.0f,
                                                Fade((Color){42, 98, 158, 255}, 0.25f));
                    ui_draw_text("Distancias", dist_box.x + 8.0f, dist_box.y + 6.0f, 11.0f,
                                 0.10f, (Color){24, 46, 76, 255}, true);
                    if (tabla_multi[0] != '\0') {
                        int vertice_activo = grafo_vertice_activo_paso(&app, app.grafo_controller_state.paso_actual);
                        int paso_actual = app.grafo_controller_state.paso_actual;
                        int mejoras_paso = grafo_contar_mejoras_paso(&app, paso_actual);
                        int sin_cambio_paso = grafo_contar_sin_cambio_paso(&app, paso_actual);
                        int empeora_paso = grafo_contar_empeora_paso(&app, paso_actual);
                        bool hay_mejora = grafo_hay_mejora_paso(&app, paso_actual);
                        char activo_label[32];
                        char arista_label[64];
                        char mejoras_label[24];
                        char sin_cambio_label[28];
                        char empeora_label[24];
                        Color mejora_chip = hay_mejora ? Fade((Color){34, 122, 72, 255}, 0.38f)
                                                       : Fade((Color){150, 158, 166, 255}, 0.22f);
                        Color mejora_texto = hay_mejora ? (Color){56, 72, 92, 255}
                                                        : (Color){122, 132, 144, 255};
                        grafo_camino_parcial_paso(&app, paso_actual, camino_label,
                                                  sizeof(camino_label));
                        grafo_cerrados_paso(&app, paso_actual, cerrados_label,
                                            sizeof(cerrados_label));
                        snprintf(progreso_label, sizeof(progreso_label), "Paso %d/%d",
                                 paso_actual + 1, app.grafo_controller_state.total_pasos);
                        snprintf(tipo_label, sizeof(tipo_label), "Tipo: %s", grafo_tipo_paso_label(&app));
                        snprintf(reproduccion_label, sizeof(reproduccion_label), "Auto: %s",
                                 app.grafo_controller_state.autoplay_activo ? "ON" : "OFF");
                        if (vertice_activo >= 0) {
                            snprintf(activo_label, sizeof(activo_label), "Activo: v%d", vertice_activo);
                        } else {
                            snprintf(activo_label, sizeof(activo_label), "Activo: -");
                        }
                        if (paso_actual >= 0 && paso_actual < app.grafo_controller_state.script_aristas_count) {
                            const GrafoArista *a = &app.grafo_controller_state.script_aristas[paso_actual];
                            snprintf(arista_label, sizeof(arista_label), "Relajando: %d->%d (w=%d)",
                                     a->origen, a->destino, a->peso);
                        } else {
                            snprintf(arista_label, sizeof(arista_label), "Relajando: -");
                        }
                        snprintf(mejoras_label, sizeof(mejoras_label), "Mejoras: %d", mejoras_paso);
                        snprintf(sin_cambio_label, sizeof(sin_cambio_label), "Sin cambio: %d", sin_cambio_paso);
                        snprintf(empeora_label, sizeof(empeora_label), "Empeora: %d", empeora_paso);
                        ui_draw_text(progreso_label,
                                     dist_box.x + dist_box.width - 86.0f, dist_box.y + 6.0f,
                                     10.0f, 0.08f, (Color){56, 72, 92, 255}, false);
                        ui_draw_text(activo_label,
                                     dist_box.x + dist_box.width - 86.0f, dist_box.y + 18.0f,
                                     10.0f, 0.08f, (Color){56, 72, 92, 255}, false);
                        ui_draw_text(mejoras_label,
                                     dist_box.x + dist_box.width - 86.0f, dist_box.y + 30.0f,
                                     10.0f, 0.08f, mejora_texto, false);
                        ui_draw_text(sin_cambio_label,
                                     dist_box.x + dist_box.width - 86.0f, dist_box.y + 42.0f,
                                     10.0f, 0.08f, (Color){106, 118, 132, 255}, false);
                        ui_draw_text(empeora_label,
                                     dist_box.x + dist_box.width - 86.0f, dist_box.y + 54.0f,
                                     10.0f, 0.08f,
                                     empeora_paso > 0 ? (Color){146, 74, 34, 255}
                                                      : (Color){122, 132, 144, 255},
                                     false);
                        ui_draw_text(tipo_label,
                                     dist_box.x + 8.0f, dist_box.y + 30.0f,
                                     10.0f, 0.08f, (Color){56, 72, 92, 255}, false);
                        ui_draw_text(reproduccion_label,
                                     dist_box.x + 8.0f, dist_box.y + 42.0f,
                                     10.0f, 0.08f, (Color){56, 72, 92, 255}, false);
                        ui_draw_text(camino_label,
                                     dist_box.x + 8.0f, dist_box.y + 54.0f,
                                     10.0f, 0.08f, (Color){56, 72, 92, 255}, false);
                        ui_draw_text(cerrados_label,
                                     dist_box.x + 8.0f, dist_box.y + 66.0f,
                                     10.0f, 0.08f, (Color){56, 72, 92, 255}, false);
                        ui_draw_text(arista_label,
                                     dist_box.x + 8.0f, dist_box.y + 18.0f,
                                     10.0f, 0.08f, (Color){56, 72, 92, 255}, false);
                        DrawRectangleRounded((Rectangle){dist_box.x + 8.0f, dist_box.y + dist_box.height - 12.0f,
                                                         9.0f, 9.0f},
                                             0.25f, 4, Fade((Color){46, 112, 185, 255}, 0.42f));
                        ui_draw_text("Activo", dist_box.x + 21.0f,
                                     dist_box.y + dist_box.height - 13.0f,
                                     9.0f, 0.08f, (Color){56, 72, 92, 255}, false);
                        DrawRectangleRounded((Rectangle){dist_box.x + 64.0f, dist_box.y + dist_box.height - 12.0f,
                                                         9.0f, 9.0f},
                                     0.25f, 4, mejora_chip);
                        ui_draw_text("Mejora", dist_box.x + 77.0f,
                                     dist_box.y + dist_box.height - 13.0f,
                                 9.0f, 0.08f, mejora_texto, false);
                        grafo_dibujar_tabla_distancias(
                            &app,
                            (Rectangle){dist_box.x + 8.0f, dist_box.y + 80.0f,
                                        dist_box.width - 16.0f, dist_box.height - 98.0f},
                            app.grafo_controller_state.paso_actual);
                    }
                }
            }

            DrawRectangleRounded(trace_box, 0.16f, 8, Fade(WHITE, 0.50f));
            DrawRectangleRoundedLinesEx(trace_box, 0.16f, 8, 1.2f,
                                        Fade((Color){42, 98, 158, 255}, 0.20f));
            ui_draw_text("Traza", trace_box.x + 10.0f, trace_box.y + 8.0f, 13.0f, 0.12f,
                         (Color){24, 46, 76, 255}, true);

            if (app.estructura_activa == ESTRUCTURA_GRAFO) {
                GrafoTrace traza_grafo = grafo_trace_desde_estado(&app);
                grafo_trace_dibujar(&traza_grafo,
                                    (Rectangle){trace_box.x + 10.0f,
                                                trace_box.y + 28.0f,
                                                trace_box.width - 20.0f,
                                                trace_box.height - 56.0f});
                grafo_trace_dibujar_barra_progreso(
                    &traza_grafo,
                    (Rectangle){trace_box.x + 10.0f, trace_box.y + trace_box.height - 22.0f,
                                trace_box.width - 20.0f, 12.0f});
            } else {
                char trace_text[1024];

                snprintf(trace_text, sizeof(trace_text),
                         "Operacion: %s\n\n"
                         "Pasos:\n%s",
                         app.mensaje_operacion, info.pasos);
                draw_scrollable_multiline_text(trace_text,
                                               (Rectangle){trace_box.x + 10.0f,
                                                           trace_box.y + 28.0f,
                                                           trace_box.width - 20.0f - 10.0f,
                                                           trace_box.height - 34.0f},
                                               17, (Color){48, 60, 76, 255}, trace_scroll);
                draw_scrollbar((Rectangle){trace_box.x + trace_box.width - 10.0f,
                                           trace_box.y + 28.0f, 6.0f,
                                           trace_box.height - 34.0f},
                               count_text_lines(trace_text) * 22.0f, trace_box.height - 34.0f,
                               trace_scroll);
            }
        }

        EndDrawing();
    }

    app_state_shutdown(&app);
    ui_unload(&ui);
    CloseWindow();

    return 0;
}
