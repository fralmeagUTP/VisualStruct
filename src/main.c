#include "raylib.h"

#include <stdio.h>
#include <string.h>

#include "algorithm_trace.h"
#include "app_state.h"
#include "code_viewer.h"
#include "cola_prioridad_view.h"
#include "cola_view.h"
#include "lista_view.h"
#include "lista_circular_view.h"
#include "pila_view.h"
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
    return -1;
}

typedef enum {
    INPUT_NONE = 0,
    INPUT_VALOR,
    INPUT_PRIORIDAD
} InputFocus;

typedef enum {
    SCREEN_HOME = 0,
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

/** @brief Dibuja la portada de estructuras y permite seleccionar una para visualizar. */
static void draw_home_screen(const UILayout *layout, AppState *app, ScreenMode *mode,
                             int *home_selected, bool activate_selected) {
    Rectangle content = {layout->sidebar.x, layout->sidebar.y,
                         layout->bottom.x + layout->bottom.width - layout->sidebar.x,
                         layout->bottom.y + layout->bottom.height - layout->sidebar.y};
    Rectangle cards_area = {content.x + 20.0f, content.y + 98.0f, content.width - 40.0f, 360.0f};
    Rectangle info_box = {content.x + 20.0f, cards_area.y + cards_area.height + 20.0f,
                          content.width - 40.0f, 78.0f};
    float gap = 14.0f;
    float card_w = (cards_area.width - gap * 4.0f) / 5.0f;
    Rectangle card_pila = {cards_area.x, cards_area.y, card_w, cards_area.height};
    Rectangle card_cola = {card_pila.x + card_w + gap, cards_area.y, card_w, cards_area.height};
    Rectangle card_lista = {card_cola.x + card_w + gap, cards_area.y, card_w, cards_area.height};
    Rectangle card_cp = {card_lista.x + card_w + gap, cards_area.y, card_w, cards_area.height};
    Rectangle card_lc = {card_cp.x + card_w + gap, cards_area.y, card_w, cards_area.height};
    Rectangle nav_help = {content.x + 20.0f, content.y + 74.0f, content.width - 40.0f, 26.0f};
    int title_w = ui_measure_text("VISUALIZADOR DE ESTRUCTURAS DE DATOS SECUENCIALES", 24.0f,
                                  0.12f, true);
    int subtitle_w = ui_measure_text("Seleccione la estructura de datos que desea visualizar",
                                     16.0f, 0.14f, false);

    DrawRectangleRounded(content, 0.02f, 8, Fade(WHITE, 0.45f));
    ui_draw_text("VISUALIZADOR DE ESTRUCTURAS DE DATOS SECUENCIALES",
                 content.x + (content.width - title_w) * 0.5f, content.y + 14.0f, 24.0f, 0.12f,
                 (Color){20, 58, 112, 255}, true);
    ui_draw_text("Seleccione la estructura de datos que desea visualizar",
                 content.x + (content.width - subtitle_w) * 0.5f, content.y + 48.0f, 16.0f,
                 0.14f, (Color){53, 66, 83, 255}, false);
    DrawRectangleRounded(nav_help, 0.25f, 8, Fade((Color){220, 232, 247, 255}, 0.55f));
    ui_draw_text("Atajos: 1 Pilas  2 Colas  3 Listas  4 Prioridad  5 Circular  |  F1 Ayuda  |  Enter: Visualizar",
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

    DrawRectangleRounded(info_box, 0.06f, 10, (Color){230, 239, 250, 255});
    DrawRectangleRoundedLinesEx(info_box, 0.06f, 10, 1.4f,
                                Fade((Color){42, 98, 158, 255}, 0.36f));
    ui_draw_text("Informacion", info_box.x + 30.0f, info_box.y + 24.0f, 14.0f, 0.14f,
                 (Color){20, 58, 112, 255}, true);
    ui_draw_text("Este visualizador permite explorar y comprender el comportamiento de",
                 info_box.x + 184.0f, info_box.y + 20.0f, 13.0f, 0.14f,
                 (Color){42, 50, 64, 255}, false);
    ui_draw_text("diferentes estructuras de datos secuenciales mediante representaciones graficas.",
                 info_box.x + 184.0f, info_box.y + 40.0f, 13.0f, 0.14f,
                 (Color){42, 50, 64, 255}, false);
}

/** @brief Gestiona navegación de alto nivel entre menu principal y visualizador. */
static void handle_navigation_keyboard(ScreenMode *mode, AppState *app, int *home_selected,
                                       bool *activate_selected) {
    int estructura_shortcut = estructura_from_shortcut();

    *activate_selected = false;
    if (*mode == SCREEN_HELP) {
        return;
    }

    if (estructura_shortcut >= 0) {
        app_state_set_estructura(app, (TipoEstructura)estructura_shortcut);
        *home_selected = estructura_shortcut;
        if (*mode == SCREEN_HOME) {
            *mode = SCREEN_VISUALIZER;
        }
        return;
    }

    if (*mode == SCREEN_HOME) {
        if (IsKeyPressed(KEY_RIGHT)) {
            *home_selected = (*home_selected + 1) % 5;
        }
        if (IsKeyPressed(KEY_LEFT)) {
            *home_selected = (*home_selected + 4) % 5;
        }
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE)) {
            *activate_selected = true;
        }
        return;
    }

    if (*mode == SCREEN_VISUALIZER) {
        if (IsKeyPressed(KEY_H) || IsKeyPressed(KEY_ESCAPE)) {
            *home_selected = app->estructura_activa;
            *mode = SCREEN_HOME;
            return;
        }

        if (IsKeyPressed(KEY_TAB)) {
            TipoEstructura next = (TipoEstructura)((app->estructura_activa + 1) % 5);
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
    "\n"
    "Puedes elegir con mouse o teclado:\n"
    "- Teclas 1..5 para seleccion rapida.\n"
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
    "\n"
    "============================================================\n"
    "J) ATAJOS DE TECLADO\n"
    "============================================================\n"
    "Operaciones:\n"
    "- I: inicializar estructura activa.\n"
    "- A: insertar (o final en listas).\n"
    "- Z: insertar al inicio (listas).\n"
    "- D: eliminar.\n"
    "- B: buscar (listas).\n"
    "- R: invertir (listas).\n"
    "- V: vaciar estructura activa.\n"
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
    "- src/pila.c, cola.c, cola_prioridad.c, lista.c, lista_circular.c: TAD puros en C.\n"
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
static void draw_active_view(const AppState *state, Rectangle panel, float content_top_y) {
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
        if (app->estructura_activa == ESTRUCTURA_LISTA ||
            app->estructura_activa == ESTRUCTURA_LISTA_CIRCULAR) {
            app_state_operacion_lista_insertar_final(app);
        } else {
            app_state_operacion_insertar(app);
        }
    }
    if (IsKeyPressed(KEY_Z)) {
        app_state_operacion_lista_insertar_inicio(app);
    }
    if (IsKeyPressed(KEY_D)) {
        app_state_operacion_eliminar(app);
    }
    if (IsKeyPressed(KEY_V)) {
        app_state_operacion_vaciar(app);
    }
    if (IsKeyPressed(KEY_B)) {
        app_state_operacion_buscar(app);
    }
    if (IsKeyPressed(KEY_R)) {
        app_state_operacion_invertir(app);
    }
}

/** @brief Sincroniza los buffers de texto editables con el estado real de entrada. */
static void sync_input_buffers(const AppState *app, char *value_text, size_t value_size,
                               char *priority_text, size_t priority_size, InputFocus focus) {
    if (focus != INPUT_VALOR) {
        snprintf(value_text, value_size, "%d", app->input_valor);
    }
    if (focus != INPUT_PRIORIDAD) {
        snprintf(priority_text, priority_size, "%d", app->input_prioridad);
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
    Rectangle btn;
    Rectangle value_box;
    Rectangle priority_box;
    InputFocus input_focus = INPUT_NONE;
    char value_text[16];
    char priority_text[16];
    float code_scroll = 0.0f;
    float trace_scroll = 0.0f;
    Color status_color;
    const char *status_label;
    int cantidad_activa;
    bool value_invalid;
    bool priority_invalid;
    ScreenMode screen_mode = SCREEN_HOME;
    int parsed_value;
    int parsed_priority;
    int home_selected = 0;
    bool home_activate = false;
    ScreenMode screen_before_help = SCREEN_HOME;
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

    SetTargetFPS(60);

    while (true) {
        if (WindowShouldClose()) {
            if (screen_mode == SCREEN_VISUALIZER) {
                screen_mode = SCREEN_HOME;
                continue;
            }
            break;
        }

        ui_set_size(&ui, GetScreenWidth(), GetScreenHeight());
        layout = ui_get_layout(&ui);
        app_state_update_visuals(&app, GetFrameTime());
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

        if (screen_mode == SCREEN_HOME) {
            BeginDrawing();
            ClearBackground((Color){250, 252, 254, 255});
            ui_draw_header(&ui);
            ui_draw_footer(&ui);
            draw_home_screen(&layout, &app, &screen_mode, &home_selected, home_activate);
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
                           sizeof(priority_text), input_focus);
        value_invalid = !parse_int_text(value_text, &parsed_value);
        priority_invalid = !parse_int_text(priority_text, &parsed_priority) ||
                           parsed_priority < 1 || parsed_priority > 99;

        if (input_focus == INPUT_VALOR) {
            edit_active_input(value_text, sizeof(value_text));
        } else if (input_focus == INPUT_PRIORIDAD) {
            edit_active_input(priority_text, sizeof(priority_text));
        }

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
            if (input_focus != INPUT_NONE) {
                if ((input_focus == INPUT_VALOR && !value_invalid) ||
                    (input_focus == INPUT_PRIORIDAD && !priority_invalid)) {
                    apply_input_focus(&app, input_focus,
                                      input_focus == INPUT_VALOR ? value_text : priority_text);
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

        ui_draw_panel(layout.sidebar, "Estructuras");
        btn = (Rectangle){layout.sidebar.x + 12.0f, layout.sidebar.y + 52.0f,
                          layout.sidebar.width - 24.0f, 40.0f};
        if (ui_sidebar_button(btn, "Menu principal", false)) {
            screen_mode = SCREEN_HOME;
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

        DrawLine((int)(layout.sidebar.x + 16.0f), (int)(btn.y + 56.0f),
                 (int)(layout.sidebar.x + layout.sidebar.width - 16.0f), (int)(btn.y + 56.0f),
                 Fade((Color){78, 110, 146, 255}, 0.28f));
        {
            bool show_priority = app.estructura_activa == ESTRUCTURA_COLA_PRIORIDAD;
            float info_y = btn.y + 64.0f;
            float inputs_top = layout.sidebar.y + layout.sidebar.height - (show_priority ? 92.0f : 42.0f);
            float available_info_h = inputs_top - info_y;
            float help_y = info_y + 70.0f;
            float nav_y = inputs_top - 18.0f;
            bool compact_sidebar = available_info_h < 120.0f;
            float title_size = compact_sidebar ? 20.0f : 22.0f;
            float meta_size = compact_sidebar ? 14.0f : 16.0f;
            float help_size = compact_sidebar ? 11.0f : 12.0f;
            float nav_size = compact_sidebar ? 10.0f : 11.0f;

            value_box = (Rectangle){layout.sidebar.x + 12.0f,
                                    layout.sidebar.y + layout.sidebar.height - (show_priority ? 92.0f : 42.0f),
                                    layout.sidebar.width - 24.0f, 38.0f};
            priority_box = (Rectangle){layout.sidebar.x + 12.0f,
                                       layout.sidebar.y + layout.sidebar.height - 42.0f,
                                       layout.sidebar.width - 24.0f, 38.0f};

            ui_draw_text("Seleccion actual:", layout.sidebar.x + 12.0f, info_y,
                         14.0f, 0.16f, (Color){66, 76, 86, 255}, false);
            ui_draw_text(estructura_nombre(app.estructura_activa), layout.sidebar.x + 12.0f,
                         info_y + 20.0f, title_size, 0.12f, (Color){28, 52, 76, 255}, true);
            ui_draw_text(TextFormat("Elementos: %d", cantidad_activa), layout.sidebar.x + 12.0f,
                         info_y + 50.0f, meta_size, 0.2f, (Color){66, 76, 86, 255}, false);

            if (available_info_h > 96.0f) {
                ui_draw_text("Puedes editar los campos o usar atajos.", layout.sidebar.x + 12.0f,
                             help_y, help_size, 0.10f, (Color){66, 76, 86, 255}, false);
                ui_draw_text("Navegacion: H menu | TAB sig. | 1..5 estructura", layout.sidebar.x + 12.0f,
                             nav_y, nav_size, 0.10f, (Color){76, 91, 110, 255}, false);
            } else if (available_info_h > 80.0f) {
                ui_draw_text("H menu | TAB | 1..5", layout.sidebar.x + 12.0f, nav_y,
                             compact_sidebar ? 10.0f : 11.0f, 0.10f, (Color){76, 91, 110, 255}, false);
            }

            if (ui_input_box(value_box, "Valor", value_text, input_focus == INPUT_VALOR,
                             value_invalid)) {
                if (input_focus == INPUT_PRIORIDAD && !priority_invalid) {
                    apply_input_focus(&app, INPUT_PRIORIDAD, priority_text);
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
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                !CheckCollisionPointRec(GetMousePosition(), value_box) &&
                (!show_priority || !CheckCollisionPointRec(GetMousePosition(), priority_box))) {
                if (input_focus == INPUT_VALOR && !value_invalid) {
                    apply_input_focus(&app, INPUT_VALOR, value_text);
                } else if (show_priority && input_focus == INPUT_PRIORIDAD && !priority_invalid) {
                    apply_input_focus(&app, INPUT_PRIORIDAD, priority_text);
                }
                input_focus = INPUT_NONE;
            }
            if (show_priority == false && input_focus == INPUT_PRIORIDAD) {
                input_focus = INPUT_NONE;
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
        ui_draw_text(TextFormat("Tiempo: %s | Espacio: %s", info.tiempo, info.espacio),
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
        draw_scrollable_multiline_text(code_display_text,
                                       (Rectangle){layout.right.x + 14.0f, layout.right.y + 102.0f,
                                                   layout.right.width - 28.0f - 10.0f,
                                                   layout.right.height - 118.0f},
                                       16, (Color){40, 52, 64, 255}, code_scroll);
        draw_scrollbar((Rectangle){layout.right.x + layout.right.width - 12.0f,
                                   layout.right.y + 102.0f, 8.0f, layout.right.height - 118.0f},
                       count_text_lines(code_display_text) * 22.0f, layout.right.height - 118.0f,
                       code_scroll);

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
            ui_draw_text(TextFormat("T: %s", info.tiempo), summary_box.x + 12.0f,
                         summary_text_y + summary_line_step * 3.0f, 12.0f, 0.10f,
                         (Color){42, 54, 70, 255}, false);
            ui_draw_text(TextFormat("E: %s", info.espacio), summary_box.x + 12.0f,
                         summary_text_y + summary_line_step * 4.0f, 12.0f, 0.10f,
                         (Color){42, 54, 70, 255}, false);

            DrawRectangleRounded(trace_box, 0.16f, 8, Fade(WHITE, 0.50f));
            DrawRectangleRoundedLinesEx(trace_box, 0.16f, 8, 1.2f,
                                        Fade((Color){42, 98, 158, 255}, 0.20f));
            ui_draw_text("Traza", trace_box.x + 10.0f, trace_box.y + 8.0f, 13.0f, 0.12f,
                         (Color){24, 46, 76, 255}, true);

        {
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
