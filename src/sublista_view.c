#include "sublista_view.h"

#include "ui.h"

static float SUBLISTA_SCROLL_Y = 0.0f;

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
 * @file sublista_view.c
 * @brief Renderizado de padres e hijos del TAD Sublista.
 */

void sublista_view_draw(const AppState *state, Rectangle panel) {
    const Nodo *padre;
    float y;
    float content_h = 20.0f;
    float viewport_h = panel.height - 16.0f;
    float max_scroll;
    float wheel;

    if (state == NULL) {
        return;
    }

    DrawRectangleRounded(panel, 0.03f, 8, (Color){234, 244, 252, 255});
    DrawRectangleRoundedLinesEx(panel, 0.03f, 8, 2.0f, (Color){26, 88, 140, 255});

    padre = state->sublista;
    while (padre != NULL) {
        int hijos = sublista_contar_hijos(padre);
        content_h += 58.0f + (hijos > 0 ? 20.0f : 0.0f);
        padre = padre->sgte;
    }
    if (content_h < viewport_h) {
        content_h = viewport_h;
    }
    max_scroll = content_h - viewport_h;
    if (max_scroll < 0.0f) {
        max_scroll = 0.0f;
    }

    wheel = GetMouseWheelMove();
    if (CheckCollisionPointRec(GetMousePosition(), panel) && wheel != 0.0f) {
        SUBLISTA_SCROLL_Y -= wheel * 24.0f;
    }
    SUBLISTA_SCROLL_Y = clamp_float_local(SUBLISTA_SCROLL_Y, 0.0f, max_scroll);

    if (state->sublista == NULL) {
        SUBLISTA_SCROLL_Y = 0.0f;
        ui_draw_text("Sublistas vacias", panel.x + 16.0f, panel.y + 16.0f, 20.0f, 0.12f,
                     (Color){26, 88, 140, 255}, false);
        ui_draw_text("Cree un padre con (A) y luego seleccione con (B)", panel.x + 16.0f,
                     panel.y + 46.0f, 15.0f, 0.10f, (Color){54, 72, 94, 255}, false);
        return;
    }

    BeginScissorMode((int)(panel.x + 2.0f), (int)(panel.y + 2.0f), (int)(panel.width - 4.0f),
                     (int)(panel.height - 6.0f));

    y = panel.y + 12.0f - SUBLISTA_SCROLL_Y;
    padre = state->sublista;
    while (padre != NULL) {
        Rectangle parent_box = {panel.x + 14.0f, y, 136.0f, 34.0f};
        bool activo =
            state->sublista_padre_activo_ok && padre->nro == state->sublista_padre_activo;
        Color pfill = activo ? (Color){214, 239, 216, 255} : (Color){201, 224, 244, 255};
        const Sublista *hijo = padre->sub;
        float hx = parent_box.x + parent_box.width + 22.0f;
        float hy = y + 2.0f;

        DrawRectangleRounded(parent_box, 0.18f, 8, pfill);
        DrawRectangleRoundedLinesEx(parent_box, 0.18f, 8, 2.0f, (Color){26, 88, 140, 255});
        ui_draw_text(TextFormat("PADRE %d", padre->nro), parent_box.x + 12.0f, parent_box.y + 8.0f,
                     15.0f, 0.08f, (Color){21, 52, 77, 255}, false);
        if (activo) {
            ui_draw_text("ACT", parent_box.x + 98.0f, parent_box.y + 8.0f, 11.0f, 0.08f,
                         (Color){22, 112, 57, 255}, false);
        }

        DrawLineEx((Vector2){parent_box.x + parent_box.width, parent_box.y + 17.0f},
                   (Vector2){parent_box.x + parent_box.width + 14.0f, parent_box.y + 17.0f},
                   1.8f, (Color){26, 88, 140, 255});
        DrawTriangle((Vector2){parent_box.x + parent_box.width + 14.0f, parent_box.y + 17.0f},
                     (Vector2){parent_box.x + parent_box.width + 9.0f, parent_box.y + 13.0f},
                     (Vector2){parent_box.x + parent_box.width + 9.0f, parent_box.y + 21.0f},
                     (Color){26, 88, 140, 255});

        if (hijo == NULL) {
            ui_draw_text("(sin hijos)", hx, hy + 8.0f, 14.0f, 0.08f, (Color){74, 88, 106, 255},
                         false);
        } else {
            while (hijo != NULL) {
                Rectangle child_box = {hx, hy, 70.0f, 28.0f};
                DrawRectangleRounded(child_box, 0.18f, 8, (Color){239, 245, 252, 255});
                DrawRectangleRoundedLinesEx(child_box, 0.18f, 8, 1.8f, (Color){54, 102, 152, 255});
                ui_draw_text(TextFormat("%d", hijo->nro), child_box.x + 23.0f, child_box.y + 7.0f,
                             14.0f, 0.08f, (Color){31, 60, 90, 255}, false);

                hx += 82.0f;
                if (hijo->sgte != NULL) {
                    DrawLine((int)(hx - 12.0f), (int)(hy + 14.0f), (int)(hx - 2.0f),
                             (int)(hy + 14.0f), (Color){54, 102, 152, 255});
                    DrawTriangle((Vector2){hx - 2.0f, hy + 14.0f}, (Vector2){hx - 7.0f, hy + 10.0f},
                                 (Vector2){hx - 7.0f, hy + 18.0f}, (Color){54, 102, 152, 255});
                }
                hijo = hijo->sgte;
            }
        }

        y += 58.0f;
        padre = padre->sgte;
    }
    EndScissorMode();

    if (max_scroll > 0.0f) {
        Rectangle track = {panel.x + panel.width - 10.0f, panel.y + 10.0f, 6.0f, panel.height - 20.0f};
        float thumb_h = track.height * (viewport_h / content_h);
        float t = (max_scroll > 0.0f) ? (SUBLISTA_SCROLL_Y / max_scroll) : 0.0f;
        Rectangle thumb;

        if (thumb_h < 24.0f) {
            thumb_h = 24.0f;
        }
        thumb = (Rectangle){track.x, track.y + (track.height - thumb_h) * t, track.width, thumb_h};
        DrawRectangleRounded(track, 0.60f, 6, Fade((Color){183, 200, 219, 255}, 0.70f));
        DrawRectangleRounded(thumb, 0.60f, 6, (Color){105, 138, 176, 255});
    }
}
