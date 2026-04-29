#include "lista_view.h"

#include "ui.h"

static float LIST_SCROLL_OFFSET = 0.0f;
static bool LIST_SCROLL_DRAGGING = false;
static float LIST_SCROLL_DRAG_GRAB = 0.0f;

/** @brief Limita un valor flotante dentro de un rango cerrado. */
static float clamp_float_local(float value, float min, float max) {
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

/**
 * @file lista_view.c
 * @brief Renderizado de la lista enlazada simple en el panel central.
 */

/**
 * @brief Dibuja la lista enlazada mostrando HEAD, nodos y NULL.
 * @param state Estado global de la aplicación.
 * @param panel Región de dibujo asignada a la vista activa.
 */
void lista_view_draw(const AppState *state, Rectangle panel) {
    int valores[128];
    int total;
    int i;
    float insert_fx;
    float delete_fx;
    float x;
    const float y = 56.0f;
    const float w = 90.0f;
    const float h = 40.0f;
    const float gap = 18.0f;
    float viewport_width;
    float content_width;
    float max_scroll;
    float wheel;
    Vector2 mouse;
    Rectangle track;
    Rectangle thumb;
    float thumb_width;
    float thumb_x;
    float thumb_travel;
    float t;

    if (state == NULL) {
        return;
    }

    total = lista_copiar_valores(&state->lista, valores, 128);
    insert_fx = ((state->operacion_animada == OPERACION_INSERTAR_INICIO) ||
                 (state->operacion_animada == OPERACION_INSERTAR_FINAL))
                    ? state->animacion_feedback
                    : 0.0f;
    delete_fx = (state->operacion_animada == OPERACION_ELIMINAR) ? state->animacion_feedback : 0.0f;

    DrawRectangleRounded(panel, 0.03f, 8, (Color){234, 244, 252, 255});
    DrawRectangleRoundedLinesEx(panel, 0.03f, 8, 2.0f, (Color){26, 88, 140, 255});

    if (total == 0) {
        LIST_SCROLL_OFFSET = 0.0f;
        LIST_SCROLL_DRAGGING = false;
        ui_draw_text("Lista vacia", panel.x + 16.0f, panel.y + 16.0f, 21.0f, 0.12f,
                 (Color){26, 88, 140, 255}, false);
        ui_draw_text("HEAD -> NULL", panel.x + 16.0f, panel.y + 48.0f, 18.0f, 0.3f,
                 (Color){26, 88, 140, 255}, false);
        return;
    }

    viewport_width = panel.width - 24.0f;
    content_width = 16.0f + total * w + (total - 1) * gap + 50.0f;
    max_scroll = content_width - viewport_width;
    if (max_scroll < 0.0f) {
        max_scroll = 0.0f;
    }

    wheel = GetMouseWheelMove();
    mouse = GetMousePosition();
    if (CheckCollisionPointRec(mouse, panel) && wheel != 0.0f) {
        LIST_SCROLL_OFFSET -= wheel * 24.0f;
    }
    LIST_SCROLL_OFFSET = clamp_float_local(LIST_SCROLL_OFFSET, 0.0f, max_scroll);

    x = panel.x + 16.0f - LIST_SCROLL_OFFSET;
    BeginScissorMode((int)(panel.x + 2.0f), (int)(panel.y + 2.0f), (int)(panel.width - 4.0f),
                     (int)(panel.height - 16.0f));
    ui_draw_text("HEAD", x, panel.y + 24.0f, 15.0f, 0.15f, (Color){174, 52, 18, 255}, false);

    for (i = 0; i < total; i++) {
        Rectangle node = {x, panel.y + y, w, h};
        bool found = state->operacion_actual == OPERACION_BUSCAR && valores[i] == state->input_valor;
        bool inserted =
            state->ultima_operacion_ok &&
            ((state->operacion_actual == OPERACION_INSERTAR_INICIO && i == 0) ||
             (state->operacion_actual == OPERACION_INSERTAR_FINAL && i == total - 1));
        Color fill = found ? (Color){255, 230, 196, 255} : (Color){197, 220, 240, 255};

        if (node.x + node.width < panel.x - 4.0f || node.x > panel.x + panel.width + 4.0f) {
            x += w + gap;
            continue;
        }

        if (inserted) {
            fill = (Color){215, 244, 221, 255};
            node.x -= 7.0f * insert_fx;
            node.y -= 5.0f * insert_fx;
            node.width += 14.0f * insert_fx;
            node.height += 10.0f * insert_fx;
        }

        DrawRectangleRounded(node, 0.18f, 8, fill);
        DrawRectangleRoundedLinesEx(node, 0.18f, 8, 2.0f, (Color){26, 88, 140, 255});
        ui_draw_text(TextFormat("%d", valores[i]), node.x + 30.0f, node.y + 10.0f, 18.0f, 0.08f,
                     (Color){24, 56, 82, 255}, false);
        if (found) {
            ui_draw_text("F", node.x + 8.0f, node.y + 12.0f, 12.0f, 0.15f,
                         (Color){174, 52, 18, 255}, false);
        }
        if (inserted) {
            ui_draw_text("N", node.x + 72.0f, node.y + 12.0f, 12.0f, 0.15f,
                         (Color){22, 112, 57, 255}, false);
        }

        if (i < total - 1) {
            DrawLine((int)(node.x + w + 4), (int)(node.y + h * 0.5f), (int)(node.x + w + 14),
                     (int)(node.y + h * 0.5f), (Color){26, 88, 140, 255});
            DrawTriangle((Vector2){node.x + w + 14, node.y + h * 0.5f},
                         (Vector2){node.x + w + 9, node.y + h * 0.5f - 4},
                         (Vector2){node.x + w + 9, node.y + h * 0.5f + 4},
                         (Color){26, 88, 140, 255});
        }
        x += w + gap;
    }

    ui_draw_text("NULL", x, panel.y + y + 12.0f, 15.0f, 0.15f, (Color){174, 52, 18, 255}, false);
    EndScissorMode();

    if (max_scroll > 0.0f) {
        track = (Rectangle){panel.x + 8.0f, panel.y + panel.height - 8.0f, panel.width - 16.0f,
                            4.0f};
        thumb_width = track.width * (viewport_width / content_width);
        if (thumb_width < 30.0f) {
            thumb_width = 30.0f;
        }
        thumb_travel = track.width - thumb_width;
        t = (max_scroll > 0.0f) ? (LIST_SCROLL_OFFSET / max_scroll) : 0.0f;
        thumb_x = track.x + thumb_travel * t;
        thumb = (Rectangle){thumb_x, track.y, thumb_width, track.height};

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mouse, thumb)) {
                LIST_SCROLL_DRAGGING = true;
                LIST_SCROLL_DRAG_GRAB = mouse.x - thumb.x;
            } else if (CheckCollisionPointRec(mouse, track)) {
                float target_t;

                target_t = (mouse.x - track.x - thumb_width * 0.5f) /
                           (thumb_travel > 0.0f ? thumb_travel : 1.0f);
                target_t = clamp_float_local(target_t, 0.0f, 1.0f);
                LIST_SCROLL_OFFSET = target_t * max_scroll;
            }
        }
        if (LIST_SCROLL_DRAGGING) {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                float new_thumb_x = mouse.x - LIST_SCROLL_DRAG_GRAB;
                float new_t;

                new_thumb_x = clamp_float_local(new_thumb_x, track.x, track.x + thumb_travel);
                new_t = (thumb_travel > 0.0f) ? ((new_thumb_x - track.x) / thumb_travel) : 0.0f;
                LIST_SCROLL_OFFSET = new_t * max_scroll;
                thumb.x = new_thumb_x;
            } else {
                LIST_SCROLL_DRAGGING = false;
            }
        }

        DrawRectangleRounded(track, 0.70f, 6, Fade((Color){183, 200, 219, 255}, 0.70f));
        DrawRectangleRounded(thumb, 0.70f, 6, (Color){105, 138, 176, 255});
    } else {
        LIST_SCROLL_DRAGGING = false;
    }

    if (state->operacion_actual == OPERACION_ELIMINAR && state->ultima_operacion_ok) {
        Rectangle ghost = {panel.x + panel.width - 98.0f + 16.0f * (1.0f - delete_fx),
                           panel.y + 12.0f - 5.0f * delete_fx, 84.0f, 34.0f};
        float alpha = 0.60f + 0.20f * delete_fx;

        DrawRectangleRounded(ghost, 0.18f, 8, Fade((Color){230, 204, 198, 255}, alpha));
        DrawRectangleRoundedLinesEx(ghost, 0.18f, 8, 2.0f,
                                    Fade((Color){176, 54, 44, 255}, alpha + 0.05f));
        ui_draw_text(TextFormat("%d", state->ultimo_valor), ghost.x + 28.0f, ghost.y + 8.0f,
                 18.0f, 0.08f, (Color){120, 40, 32, 255}, false);
        ui_draw_text("DEL", ghost.x + 6.0f, ghost.y + 9.0f, 12.0f, 0.15f,
                 (Color){176, 54, 44, 255}, false);
    }
}
