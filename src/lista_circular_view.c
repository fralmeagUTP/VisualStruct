#include "lista_circular_view.h"

#include "lista_circular.h"
#include "ui.h"

static float CIRC_LIST_SCROLL_OFFSET = 0.0f;
static bool CIRC_LIST_SCROLL_DRAGGING = false;
static float CIRC_LIST_SCROLL_DRAG_GRAB = 0.0f;

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
 * @file lista_circular_view.c
 * @brief Renderizado de la lista circular en el panel central.
 */

void lista_circular_view_draw(const AppState *state, Rectangle panel) {
    int valores[128];
    int total;
    int i;
    float insert_fx;
    float delete_fx;
    float start_x;
    float x;
    const float y = 62.0f;
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

    total = lcir_copiar_valores(&state->lista_circular, valores, 128);
    insert_fx = ((state->operacion_animada == OPERACION_INSERTAR_INICIO) ||
                 (state->operacion_animada == OPERACION_INSERTAR_FINAL))
                    ? state->animacion_feedback
                    : 0.0f;
    delete_fx = (state->operacion_animada == OPERACION_ELIMINAR) ? state->animacion_feedback : 0.0f;

    DrawRectangleRounded(panel, 0.03f, 8, (Color){234, 244, 252, 255});
    DrawRectangleRoundedLinesEx(panel, 0.03f, 8, 2.0f, (Color){26, 88, 140, 255});

    if (total == 0) {
        CIRC_LIST_SCROLL_OFFSET = 0.0f;
        CIRC_LIST_SCROLL_DRAGGING = false;
        ui_draw_text("Lista circular vacia", panel.x + 16.0f, panel.y + 16.0f, 21.0f, 0.12f,
                     (Color){26, 88, 140, 255}, false);
        ui_draw_text("HEAD -> (vacia) -> HEAD", panel.x + 16.0f, panel.y + 48.0f, 18.0f, 0.2f,
                     (Color){26, 88, 140, 255}, false);
        return;
    }

    viewport_width = panel.width - 24.0f;
    content_width = 16.0f + total * w + (total - 1) * gap + 80.0f;
    max_scroll = content_width - viewport_width;
    if (max_scroll < 0.0f) {
        max_scroll = 0.0f;
    }

    wheel = GetMouseWheelMove();
    mouse = GetMousePosition();
    if (CheckCollisionPointRec(mouse, panel) && wheel != 0.0f) {
        CIRC_LIST_SCROLL_OFFSET -= wheel * 24.0f;
    }
    CIRC_LIST_SCROLL_OFFSET = clamp_float_local(CIRC_LIST_SCROLL_OFFSET, 0.0f, max_scroll);

    start_x = panel.x + 16.0f - CIRC_LIST_SCROLL_OFFSET;
    x = start_x;
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

    if (total > 1) {
        float first_center = start_x + w * 0.5f;
        float last_center = start_x + (total - 1) * (w + gap) + w * 0.5f;
        float arc_top = panel.y + y - 28.0f;
        float node_top = panel.y + y - 6.0f;

        DrawLine((int)last_center, (int)node_top, (int)last_center, (int)arc_top,
                 (Color){42, 98, 158, 255});
        DrawLine((int)last_center, (int)arc_top, (int)first_center, (int)arc_top,
                 (Color){42, 98, 158, 255});
        DrawLine((int)first_center, (int)arc_top, (int)first_center, (int)node_top,
                 (Color){42, 98, 158, 255});
        DrawTriangle((Vector2){first_center, node_top + 2.0f},
                     (Vector2){first_center - 4.0f, node_top - 4.0f},
                     (Vector2){first_center + 4.0f, node_top - 4.0f},
                     (Color){42, 98, 158, 255});
        ui_draw_text("CIRCULAR", first_center + 10.0f, arc_top - 10.0f, 11.0f, 0.08f,
                     (Color){42, 98, 158, 255}, false);
    }
    EndScissorMode();

    if (max_scroll > 0.0f) {
        track = (Rectangle){panel.x + 8.0f, panel.y + panel.height - 8.0f, panel.width - 16.0f,
                            4.0f};
        thumb_width = track.width * (viewport_width / content_width);
        if (thumb_width < 30.0f) {
            thumb_width = 30.0f;
        }
        thumb_travel = track.width - thumb_width;
        t = (max_scroll > 0.0f) ? (CIRC_LIST_SCROLL_OFFSET / max_scroll) : 0.0f;
        thumb_x = track.x + thumb_travel * t;
        thumb = (Rectangle){thumb_x, track.y, thumb_width, track.height};

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mouse, thumb)) {
                CIRC_LIST_SCROLL_DRAGGING = true;
                CIRC_LIST_SCROLL_DRAG_GRAB = mouse.x - thumb.x;
            } else if (CheckCollisionPointRec(mouse, track)) {
                float target_t;

                target_t = (mouse.x - track.x - thumb_width * 0.5f) /
                           (thumb_travel > 0.0f ? thumb_travel : 1.0f);
                target_t = clamp_float_local(target_t, 0.0f, 1.0f);
                CIRC_LIST_SCROLL_OFFSET = target_t * max_scroll;
            }
        }
        if (CIRC_LIST_SCROLL_DRAGGING) {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                float new_thumb_x = mouse.x - CIRC_LIST_SCROLL_DRAG_GRAB;
                float new_t;

                new_thumb_x = clamp_float_local(new_thumb_x, track.x, track.x + thumb_travel);
                new_t = (thumb_travel > 0.0f) ? ((new_thumb_x - track.x) / thumb_travel) : 0.0f;
                CIRC_LIST_SCROLL_OFFSET = new_t * max_scroll;
                thumb.x = new_thumb_x;
            } else {
                CIRC_LIST_SCROLL_DRAGGING = false;
            }
        }

        DrawRectangleRounded(track, 0.70f, 6, Fade((Color){183, 200, 219, 255}, 0.70f));
        DrawRectangleRounded(thumb, 0.70f, 6, (Color){105, 138, 176, 255});
    } else {
        CIRC_LIST_SCROLL_DRAGGING = false;
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
