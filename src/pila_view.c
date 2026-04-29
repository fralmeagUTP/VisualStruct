#include "pila_view.h"

#include "ui.h"

static float PILE_SCROLL_OFFSET = 0.0f;
static bool PILE_SCROLL_DRAGGING = false;
static float PILE_SCROLL_DRAG_GRAB = 0.0f;

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
 * @file pila_view.c
 * @brief Renderizado de la pila dentro del panel central.
 */

/**
 * @brief Dibuja la pila como una secuencia vertical de nodos.
 * @param state Estado global de la aplicación.
 * @param panel Región de dibujo asignada a la vista activa.
 */
void pila_view_draw(const AppState *state, Rectangle panel) {
    int valores[128];
    int total;
    int i;
    float insert_fx;
    float delete_fx;
    float x;
    float y;
    float viewport_height;
    float content_height;
    float max_scroll;
    float wheel;
    Vector2 mouse;
    Rectangle track;
    Rectangle thumb;
    float thumb_height;
    float thumb_y;
    float thumb_travel;
    float t;
    const float w = 120.0f;
    const float h = 38.0f;
    const float gap = 8.0f;

    if (state == NULL) {
        return;
    }

    total = pila_copiar_valores(&state->pila, valores, 128);
    insert_fx = (state->operacion_animada == OPERACION_INSERTAR) ? state->animacion_feedback : 0.0f;
    delete_fx = (state->operacion_animada == OPERACION_ELIMINAR) ? state->animacion_feedback : 0.0f;

    DrawRectangleRounded(panel, 0.03f, 8, (Color){234, 244, 252, 255});
    DrawRectangleRoundedLinesEx(panel, 0.03f, 8, 2.0f, (Color){26, 88, 140, 255});

    if (total == 0) {
        PILE_SCROLL_OFFSET = 0.0f;
        ui_draw_text("Pila vacia", panel.x + 16.0f, panel.y + 16.0f, 21.0f, 0.12f,
                 (Color){26, 88, 140, 255}, false);
        return;
    }

    viewport_height = panel.height - 16.0f;
    content_height = 16.0f + total * h + (total - 1) * gap + 8.0f;
    max_scroll = content_height - viewport_height;
    if (max_scroll < 0.0f) {
        max_scroll = 0.0f;
    }

    wheel = GetMouseWheelMove();
    mouse = GetMousePosition();
    if (CheckCollisionPointRec(GetMousePosition(), panel) && wheel != 0.0f) {
        PILE_SCROLL_OFFSET -= wheel * 24.0f;
    }
    PILE_SCROLL_OFFSET = clamp_float_local(PILE_SCROLL_OFFSET, 0.0f, max_scroll);

    x = panel.x + panel.width * 0.5f - w * 0.5f;
    y = panel.y + 16.0f - PILE_SCROLL_OFFSET;

    BeginScissorMode((int)(panel.x + 2.0f), (int)(panel.y + 2.0f), (int)(panel.width - 14.0f),
                     (int)(panel.height - 4.0f));
    for (i = 0; i < total; i++) {
        Rectangle node = {x, y, w, h};
        bool inserted =
            state->operacion_actual == OPERACION_INSERTAR && state->ultima_operacion_ok && i == 0;
        Color fill = (i == 0) ? (Color){255, 230, 196, 255} : (Color){197, 220, 240, 255};
        Color border = (i == 0) ? (Color){226, 122, 16, 255} : (Color){26, 88, 140, 255};

        if (node.y + node.height < panel.y + 2.0f || node.y > panel.y + panel.height - 2.0f) {
            y += h + gap;
            continue;
        }

        if (inserted) {
            fill = (Color){215, 244, 221, 255};
            border = (Color){32, 132, 72, 255};
            node.x -= 8.0f * insert_fx;
            node.y -= 5.0f * insert_fx;
            node.width += 16.0f * insert_fx;
            node.height += 10.0f * insert_fx;
        }

        DrawRectangleRounded(node, 0.18f, 8, fill);
        DrawRectangleRoundedLinesEx(node, 0.18f, 8, 2.0f, border);
        ui_draw_text(TextFormat("%d", valores[i]), node.x + 44.0f, node.y + 9.0f, 20.0f, 0.08f,
                 (Color){24, 56, 82, 255}, false);
        if (i == 0) {
            ui_draw_text("TOPE", node.x + 6.0f, node.y + 10.0f, 13.0f, 0.15f,
                         (Color){174, 52, 18, 255}, false);
        }
        if (inserted) {
            ui_draw_text("NEW", node.x + 78.0f, node.y + 10.0f, 12.0f, 0.15f,
                         (Color){22, 112, 57, 255}, false);
        }
        y += h + gap;
    }
    EndScissorMode();

    if (max_scroll > 0.0f) {
        track = (Rectangle){panel.x + panel.width - 8.0f, panel.y + 8.0f, 4.0f, panel.height - 16.0f};
        thumb_height = track.height * (viewport_height / content_height);
        if (thumb_height < 24.0f) {
            thumb_height = 24.0f;
        }
        thumb_travel = track.height - thumb_height;
        t = (max_scroll > 0.0f) ? (PILE_SCROLL_OFFSET / max_scroll) : 0.0f;
        thumb_y = track.y + thumb_travel * t;
        thumb = (Rectangle){track.x, thumb_y, track.width, thumb_height};

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mouse, thumb)) {
                PILE_SCROLL_DRAGGING = true;
                PILE_SCROLL_DRAG_GRAB = mouse.y - thumb.y;
            } else if (CheckCollisionPointRec(mouse, track)) {
                float target_t;

                target_t = (mouse.y - track.y - thumb_height * 0.5f) /
                           (thumb_travel > 0.0f ? thumb_travel : 1.0f);
                target_t = clamp_float_local(target_t, 0.0f, 1.0f);
                PILE_SCROLL_OFFSET = target_t * max_scroll;
            }
        }
        if (PILE_SCROLL_DRAGGING) {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                float new_thumb_y = mouse.y - PILE_SCROLL_DRAG_GRAB;
                float new_t;

                new_thumb_y = clamp_float_local(new_thumb_y, track.y, track.y + thumb_travel);
                new_t = (thumb_travel > 0.0f) ? ((new_thumb_y - track.y) / thumb_travel) : 0.0f;
                PILE_SCROLL_OFFSET = new_t * max_scroll;
                thumb.y = new_thumb_y;
            } else {
                PILE_SCROLL_DRAGGING = false;
            }
        }

        DrawRectangleRounded(track, 0.70f, 6, Fade((Color){183, 200, 219, 255}, 0.70f));
        DrawRectangleRounded(thumb, 0.70f, 6, (Color){105, 138, 176, 255});
    } else {
        PILE_SCROLL_DRAGGING = false;
    }

    if (state->operacion_actual == OPERACION_ELIMINAR && state->ultima_operacion_ok) {
        Rectangle ghost = {panel.x + panel.width - 98.0f + 16.0f * (1.0f - delete_fx),
                           panel.y + 14.0f - 5.0f * delete_fx, 84.0f, 34.0f};
        float alpha = 0.60f + 0.20f * delete_fx;

        DrawRectangleRounded(ghost, 0.18f, 8, Fade((Color){230, 204, 198, 255}, alpha));
        DrawRectangleRoundedLinesEx(ghost, 0.18f, 8, 2.0f,
                                    Fade((Color){176, 54, 44, 255}, alpha + 0.05f));
        ui_draw_text(TextFormat("%d", state->ultimo_valor), ghost.x + 28.0f, ghost.y + 8.0f,
                 18.0f, 0.08f, (Color){120, 40, 32, 255}, false);
        ui_draw_text("POP", ghost.x + 6.0f, ghost.y + 9.0f, 12.0f, 0.15f,
                 (Color){176, 54, 44, 255}, false);
    }
}
