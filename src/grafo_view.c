/**
 * @file grafo_view.c
 * @brief Implementación del renderizado visual con Raylib
 */

#include "grafo_view.h"
#include "ui.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static Rectangle grafo_vista_area_lienzo(const GrafoVista *vista) {
    Rectangle canvas = vista->area_renderizado;
    canvas.x += 2.0f;
    canvas.y += 30.0f;
    canvas.width -= 16.0f;
    canvas.height -= 54.0f;
    if (canvas.width < 40.0f) {
        canvas.width = 40.0f;
    }
    if (canvas.height < 40.0f) {
        canvas.height = 40.0f;
    }
    return canvas;
}

static float grafo_vista_px(const GrafoVista *vista, float local_x) {
    return grafo_vista_area_lienzo(vista).x + vista->offset_x + local_x;
}

static float grafo_vista_py(const GrafoVista *vista, float local_y) {
    return grafo_vista_area_lienzo(vista).y + vista->offset_y + local_y;
}

static void grafo_vista_limites_contenido(const GrafoVista *vista, float *min_x, float *max_x,
                                          float *min_y, float *max_y) {
    bool found = false;
    float lx = 0.0f;
    float rx = 0.0f;
    float ty = 0.0f;
    float by = 0.0f;

    if (min_x == NULL || max_x == NULL || min_y == NULL || max_y == NULL) {
        return;
    }
    if (vista == NULL || vista->estado == NULL || vista->estado->cantidad_vertices <= 0) {
        *min_x = 0.0f;
        *max_x = 0.0f;
        *min_y = 0.0f;
        *max_y = 0.0f;
        return;
    }

    for (int i = 0; i < vista->estado->cantidad_vertices; i++) {
        const GrafoVerticeVisual *v = &vista->estado->vertices[i];
        float left;
        float right;
        float top;
        float bottom;
        if (!v->visible) {
            continue;
        }
        left = v->x - v->radio - 20.0f;
        right = v->x + v->radio + 20.0f;
        top = v->y - v->radio - 20.0f;
        bottom = v->y + v->radio + 36.0f;

        if (!found) {
            lx = left;
            rx = right;
            ty = top;
            by = bottom;
            found = true;
            continue;
        }
        if (left < lx) {
            lx = left;
        }
        if (right > rx) {
            rx = right;
        }
        if (top < ty) {
            ty = top;
        }
        if (bottom > by) {
            by = bottom;
        }
    }

    if (!found) {
        lx = 0.0f;
        rx = 0.0f;
        ty = 0.0f;
        by = 0.0f;
    }

    *min_x = lx;
    *max_x = rx;
    *min_y = ty;
    *max_y = by;
}

static float grafo_vista_clampf(float value, float min_value, float max_value) {
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

static void grafo_vista_ajustar_offset(GrafoVista *vista) {
    Rectangle canvas;
    float min_x;
    float max_x;
    float min_y;
    float max_y;
    float graph_width;
    float graph_height;
    const float margin = 14.0f;

    if (vista == NULL) {
        return;
    }

    canvas = grafo_vista_area_lienzo(vista);
    grafo_vista_limites_contenido(vista, &min_x, &max_x, &min_y, &max_y);
    graph_width = max_x - min_x;
    graph_height = max_y - min_y;

    if (graph_width + margin * 2.0f <= canvas.width) {
        vista->offset_x = (canvas.width - graph_width) * 0.5f - min_x;
    } else {
        float min_offset_x = canvas.width - margin - max_x;
        float max_offset_x = margin - min_x;
        vista->offset_x = grafo_vista_clampf(vista->offset_x, min_offset_x, max_offset_x);
    }

    if (graph_height + margin * 2.0f <= canvas.height) {
        vista->offset_y = (canvas.height - graph_height) * 0.5f - min_y;
    } else {
        float min_offset_y = canvas.height - margin - max_y;
        float max_offset_y = margin - min_y;
        vista->offset_y = grafo_vista_clampf(vista->offset_y, min_offset_y, max_offset_y);
    }
}

static void grafo_vista_dibujar_scrolls(const GrafoVista *vista) {
    Rectangle canvas;
    float min_x;
    float max_x;
    float min_y;
    float max_y;
    float graph_width;
    float graph_height;
    const float margin = 14.0f;

    if (vista == NULL) {
        return;
    }

    canvas = grafo_vista_area_lienzo(vista);
    grafo_vista_limites_contenido(vista, &min_x, &max_x, &min_y, &max_y);
    graph_width = max_x - min_x;
    graph_height = max_y - min_y;

    if (graph_width + margin * 2.0f > canvas.width) {
        float min_offset_x = canvas.width - margin - max_x;
        float max_offset_x = margin - min_x;
        float t = (max_offset_x - min_offset_x) > 0.001f
                      ? (vista->offset_x - min_offset_x) / (max_offset_x - min_offset_x)
                      : 0.0f;
        float track_w = canvas.width - 12.0f;
        float thumb_w = track_w * (canvas.width / (graph_width + margin * 2.0f));
        float thumb_x;
        if (thumb_w < 22.0f) {
            thumb_w = 22.0f;
        }
        if (t < 0.0f) {
            t = 0.0f;
        } else if (t > 1.0f) {
            t = 1.0f;
        }
        DrawRectangleRounded((Rectangle){canvas.x + 6.0f, canvas.y + canvas.height - 6.0f, track_w, 4.0f},
                             0.6f, 4, Fade((Color){100, 120, 140, 255}, 0.26f));
        thumb_x = canvas.x + 6.0f + t * (track_w - thumb_w);
        DrawRectangleRounded((Rectangle){thumb_x, canvas.y + canvas.height - 6.0f, thumb_w, 4.0f},
                             0.6f, 4, Fade((Color){42, 98, 158, 255}, 0.70f));
    }

    if (graph_height + margin * 2.0f > canvas.height) {
        float min_offset_y = canvas.height - margin - max_y;
        float max_offset_y = margin - min_y;
        float t = (max_offset_y - min_offset_y) > 0.001f
                      ? (vista->offset_y - min_offset_y) / (max_offset_y - min_offset_y)
                      : 0.0f;
        float track_h = canvas.height - 12.0f;
        float thumb_h = track_h * (canvas.height / (graph_height + margin * 2.0f));
        float thumb_y;
        if (thumb_h < 22.0f) {
            thumb_h = 22.0f;
        }
        if (t < 0.0f) {
            t = 0.0f;
        } else if (t > 1.0f) {
            t = 1.0f;
        }
        DrawRectangleRounded((Rectangle){canvas.x + canvas.width - 6.0f, canvas.y + 6.0f, 4.0f, track_h},
                             0.6f, 4, Fade((Color){100, 120, 140, 255}, 0.26f));
        thumb_y = canvas.y + 6.0f + t * (track_h - thumb_h);
        DrawRectangleRounded((Rectangle){canvas.x + canvas.width - 6.0f, thumb_y, 4.0f, thumb_h},
                             0.6f, 4, Fade((Color){42, 98, 158, 255}, 0.70f));
    }
}

/* ============================================================================
 * Inicialización
 * ============================================================================ */

GrafoVistaColores grafo_vista_colores_defecto(void) {
    GrafoVistaColores c;
    c.vertice_normal = (Color){168, 168, 168, 255};
    c.vertice_visitado = DARKGRAY;
    c.vertice_inicial = BLUE;
    c.vertice_destino = RED;
    c.vertice_actual = YELLOW;
    
    c.arista_normal = DARKGRAY;
    c.arista_relajada = ORANGE;
    c.arista_camino_minimo = GREEN;
    c.arista_mst = PURPLE;
    c.arista_error = RED;
    
    c.texto_normal = BLACK;
    c.texto_destacado = WHITE;
    c.fondo_panel = (Color){240, 240, 240, 255};
    c.borde_panel = (Color){100, 100, 100, 255};
    return c;
}

GrafoVistaOpciones grafo_vista_opciones_defecto(void) {
    GrafoVistaOpciones o;
    o.mostrar_pesos = true;
    o.mostrar_distancias = true;
    o.mostrar_ordenes = false;
    o.mostrar_flechas = true;
    o.mostrar_etiquetas = true;
    o.animar_algoritmo = true;
    o.resaltar_camino = true;
    
    o.grosor_arista_normal = 2.0f;
    o.grosor_arista_destacada = 4.0f;
    o.tamaño_fuente = 12.0f;
    return o;
}

GrafoVista grafo_vista_init(const GrafoState *estado, Rectangle area_renderizado) {
    GrafoVista vista;
    vista.estado = estado;
    vista.area_renderizado = area_renderizado;
    vista.offset_x = 0.0f;
    vista.offset_y = 0.0f;
    vista.layout_config = grafo_layout_config_defecto((int)area_renderizado.width, 
                                                      (int)area_renderizado.height);
    vista.colores = grafo_vista_colores_defecto();
    vista.opciones = grafo_vista_opciones_defecto();
    vista.necesita_redibujarse = true;
    grafo_vista_ajustar_offset(&vista);
    return vista;
}

/* ============================================================================
 * Funciones Auxiliares de Color
 * ============================================================================ */

Color grafo_vista_color_vertice(const GrafoVista *vista, GrafoVerticeEstadoVisual estado) {
    if (!vista) return GRAY;
    
    switch (estado) {
        case GRAFO_VÉRTICE_NORMAL: return vista->colores.vertice_normal;
        case GRAFO_VÉRTICE_VISITADO: return vista->colores.vertice_visitado;
        case GRAFO_VÉRTICE_INICIAL: return vista->colores.vertice_inicial;
        case GRAFO_VÉRTICE_DESTINO: return vista->colores.vertice_destino;
        case GRAFO_VÉRTICE_ACTUAL: return vista->colores.vertice_actual;
        default: return GRAY;
    }
}

Color grafo_vista_color_arista(const GrafoVista *vista, GrafoAristaEstadoVisual estado) {
    if (!vista) return DARKGRAY;
    
    switch (estado) {
        case GRAFO_ARISTA_NORMAL: return vista->colores.arista_normal;
        case GRAFO_ARISTA_RELAJADA: return vista->colores.arista_relajada;
        case GRAFO_ARISTA_CAMINO_MINIMO: return vista->colores.arista_camino_minimo;
        case GRAFO_ARISTA_MST: return vista->colores.arista_mst;
        case GRAFO_ARISTA_ERROR: return vista->colores.arista_error;
        default: return DARKGRAY;
    }
}

/* ============================================================================
 * Funciones Auxiliares de Geometría
 * ============================================================================ */

void grafo_vista_punto_en_linea(float x1, float y1, float x2, float y2, 
                               float distancia, float *px, float *py) {
    if (!px || !py) return;
    
    float dx = x2 - x1;
    float dy = y2 - y1;
    float len = sqrtf(dx * dx + dy * dy);
    
    if (len < 0.001f) {
        *px = x1;
        *py = y1;
        return;
    }
    
    *px = x1 + (dx / len) * distancia;
    *py = y1 + (dy / len) * distancia;
}

void grafo_vista_dibujar_flecha(float x1, float y1, float x2, float y2, 
                               float grosor, Color color) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float len = sqrtf(dx * dx + dy * dy);

    if (len <= 0.001f) {
        return;
    }

    {
        float ux = dx / len;
        float uy = dy / len;
        float head_len = 11.0f;
        float head_half = 4.0f;
        float shaft_end_x = x2 - ux * head_len;
        float shaft_end_y = y2 - uy * head_len;
        Vector2 tip = {x2, y2};
        Vector2 left = {shaft_end_x + uy * head_half, shaft_end_y - ux * head_half};
        Vector2 right = {shaft_end_x - uy * head_half, shaft_end_y + ux * head_half};

        DrawLineEx((Vector2){x1, y1}, (Vector2){shaft_end_x, shaft_end_y}, grosor, color);
        DrawTriangle(tip, left, right, color);
    }
}
int grafo_vista_detectar_vertice(const GrafoVista *vista, Vector2 mouse_pos) {
    if (!vista || !vista->estado) return -1;
    
    for (int i = 0; i < vista->estado->cantidad_vertices; i++) {
        const GrafoVerticeVisual *v = &vista->estado->vertices[i];
        if (!v->visible) continue;
        
        float dx = mouse_pos.x - grafo_vista_px(vista, v->x);
        float dy = mouse_pos.y - grafo_vista_py(vista, v->y);
        float dist = sqrtf(dx * dx + dy * dy);
        
        if (dist <= v->radio * 1.2f) {
            return v->id;
        }
    }
    return -1;
}

/* ============================================================================
 * Renderizado de Componentes
 * ============================================================================ */

void grafo_vista_dibujar_fondo(const GrafoVista *vista) {
    if (!vista) return;
    
    DrawRectangleRec(vista->area_renderizado, vista->colores.fondo_panel);
    DrawRectangleLinesEx(vista->area_renderizado, 2.0f, vista->colores.borde_panel);
}

void grafo_vista_dibujar_arista_individual(const GrafoVista *vista, 
                                           const GrafoAristaVisual *arista,
                                           const GrafoVerticeVisual *v_origen,
                                           const GrafoVerticeVisual *v_destino) {
    if (!vista || !arista || !v_origen || !v_destino) return;
    if (!arista->visible || !v_origen->visible || !v_destino->visible) return;
    
    Color color = grafo_vista_color_arista(vista, arista->estado);
    float grosor = (arista->estado == GRAFO_ARISTA_NORMAL) 
                    ? vista->opciones.grosor_arista_normal 
                    : vista->opciones.grosor_arista_destacada;
    
    DrawLineEx((Vector2){grafo_vista_px(vista, v_origen->x), grafo_vista_py(vista, v_origen->y)},
              (Vector2){grafo_vista_px(vista, v_destino->x), grafo_vista_py(vista, v_destino->y)},
              grosor, color);
}

void grafo_vista_dibujar_aristas(const GrafoVista *vista) {
    if (!vista || !vista->estado) return;
    
    for (int i = 0; i < vista->estado->cantidad_aristas; i++) {
        const GrafoAristaVisual *arista = &vista->estado->aristas[i];
        if (!arista->visible) continue;
        
        const GrafoVerticeVisual *v_origen = grafo_state_obtener_vertice(vista->estado, arista->origen);
        const GrafoVerticeVisual *v_destino = grafo_state_obtener_vertice(vista->estado, arista->destino);
        
        if (!v_origen || !v_destino) continue;
        
        grafo_vista_dibujar_arista_individual(vista, arista, v_origen, v_destino);
    }
}

void grafo_vista_dibujar_flechas(const GrafoVista *vista) {
    if (!vista || !vista->estado || !vista->estado->es_dirigido) return;
    if (!vista->opciones.mostrar_flechas) return;
    
    for (int i = 0; i < vista->estado->cantidad_aristas; i++) {
        const GrafoAristaVisual *arista = &vista->estado->aristas[i];
        if (!arista->visible) continue;
        
        const GrafoVerticeVisual *v_origen = grafo_state_obtener_vertice(vista->estado, arista->origen);
        const GrafoVerticeVisual *v_destino = grafo_state_obtener_vertice(vista->estado, arista->destino);
        
        if (!v_origen || !v_destino) continue;
        
        Color color = grafo_vista_color_arista(vista, arista->estado);
        float ox = grafo_vista_px(vista, v_origen->x);
        float oy = grafo_vista_py(vista, v_origen->y);
        float dx = grafo_vista_px(vista, v_destino->x);
        float dy = grafo_vista_py(vista, v_destino->y);
        float vx = dx - ox;
        float vy = dy - oy;
        float len = sqrtf(vx * vx + vy * vy);
        float px_inicio, py_inicio, px_fin, py_fin;

        if (len <= 0.001f) continue;

        grafo_vista_punto_en_linea(ox, oy, dx, dy, v_origen->radio + 2.0f, &px_inicio, &py_inicio);
        grafo_vista_punto_en_linea(ox, oy, dx, dy, len - (v_destino->radio + 2.0f), &px_fin, &py_fin);

        grafo_vista_dibujar_flecha(px_inicio, py_inicio, px_fin, py_fin, 2.0f, color);
    }
}
void grafo_vista_dibujar_vertice_individual(const GrafoVista *vista, 
                                            const GrafoVerticeVisual *vertice) {
    if (!vista || !vertice || !vertice->visible) return;
    
    Color color = grafo_vista_color_vertice(vista, vertice->estado);
    DrawCircle((int)grafo_vista_px(vista, vertice->x), (int)grafo_vista_py(vista, vertice->y),
               vertice->radio, color);
    DrawCircleLines((int)grafo_vista_px(vista, vertice->x), (int)grafo_vista_py(vista, vertice->y),
                    vertice->radio, vista->colores.texto_normal);
}

void grafo_vista_dibujar_vertices(const GrafoVista *vista) {
    if (!vista || !vista->estado) return;
    
    for (int i = 0; i < vista->estado->cantidad_vertices; i++) {
        grafo_vista_dibujar_vertice_individual(vista, &vista->estado->vertices[i]);
    }
}

void grafo_vista_dibujar_pesos(const GrafoVista *vista) {
    if (!vista || !vista->estado || !vista->opciones.mostrar_pesos) return;
    
    for (int i = 0; i < vista->estado->cantidad_aristas; i++) {
        const GrafoAristaVisual *arista = &vista->estado->aristas[i];
        if (!arista->visible) continue;
        
        const GrafoVerticeVisual *v_origen = grafo_state_obtener_vertice(vista->estado, arista->origen);
        const GrafoVerticeVisual *v_destino = grafo_state_obtener_vertice(vista->estado, arista->destino);
        
        if (!v_origen || !v_destino) continue;
        
        /* Punto central de la arista con desplazamiento perpendicular para separar el peso */
        float ox = grafo_vista_px(vista, v_origen->x);
        float oy = grafo_vista_py(vista, v_origen->y);
        float dx = grafo_vista_px(vista, v_destino->x);
        float dy = grafo_vista_py(vista, v_destino->y);
        float cx = (ox + dx) * 0.5f;
        float cy = (oy + dy) * 0.5f;
        float vx = dx - ox;
        float vy = dy - oy;
        float len = sqrtf(vx * vx + vy * vy);
        float nx = 0.0f;
        float ny = 0.0f;
        float weight_offset = 12.0f;

        if (len > 0.001f) {
            nx = -vy / len;
            ny = vx / len;
        }
        
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%d", arista->peso);
        ui_draw_text(buffer, cx + nx * weight_offset - 8.0f, cy + ny * weight_offset - 8.0f, 14.0f,
                     0.08f, vista->colores.texto_normal, false);
    }
}

void grafo_vista_dibujar_etiquetas(const GrafoVista *vista) {
    if (!vista || !vista->estado || !vista->opciones.mostrar_etiquetas) return;
    
    for (int i = 0; i < vista->estado->cantidad_vertices; i++) {
        const GrafoVerticeVisual *v = &vista->estado->vertices[i];
        if (!v->visible) continue;
        
        char buffer[64];
        int offset_y = -10;
        
        /* ID del vértice */
        snprintf(buffer, sizeof(buffer), "V%d", v->id);
        ui_draw_text(buffer, grafo_vista_px(vista, v->x) - 10.0f,
                     grafo_vista_py(vista, v->y) + (float)offset_y - 1.0f, 14.0f, 0.08f,
                     vista->colores.texto_normal, false);
        
        /* Distancia (si está activa) */
        if (vista->opciones.mostrar_distancias && v->distancia > 0) {
            const float dist_font_size = 12.0f;
            const float dist_padding_x = 4.0f;
            const float dist_padding_y = 2.0f;
            const float dist_gap = 8.0f;
            const float panel_left = vista->area_renderizado.x + 4.0f;
            const float panel_top = vista->area_renderizado.y + 4.0f;
            const float panel_right = vista->area_renderizado.x + vista->area_renderizado.width - 4.0f;
            const float panel_bottom = vista->area_renderizado.y + vista->area_renderizado.height - 4.0f;
            float vx = grafo_vista_px(vista, v->x);
            float vy = grafo_vista_py(vista, v->y);
            float text_x;
            float text_y;
            int text_w;
            float bg_x;
            float bg_y;
            float bg_w;
            float bg_h;

            snprintf(buffer, sizeof(buffer), "d=%d", v->distancia);
            text_w = ui_measure_text(buffer, dist_font_size, 0.08f, false);

            /* Ubicar la distancia fuera del vertice: arriba-derecha por defecto */
            text_x = vx + v->radio + dist_gap;
            text_y = vy - v->radio - dist_font_size - dist_gap;

            if (text_x + (float)text_w > panel_right) {
                text_x = vx - v->radio - (float)text_w - dist_gap;
            }
            if (text_x < panel_left) {
                text_x = panel_left;
            }
            if (text_y < panel_top) {
                text_y = vy + v->radio + dist_gap;
            }
            if (text_y + dist_font_size > panel_bottom) {
                text_y = panel_bottom - dist_font_size;
            }

            bg_x = text_x - dist_padding_x;
            bg_y = text_y - dist_padding_y;
            bg_w = (float)text_w + dist_padding_x * 2.0f;
            bg_h = dist_font_size + dist_padding_y * 2.0f;
            DrawRectangle((int)bg_x, (int)bg_y, (int)bg_w, (int)bg_h, Fade(RAYWHITE, 0.86f));
            DrawRectangleLines((int)bg_x, (int)bg_y, (int)bg_w, (int)bg_h, Fade(BLUE, 0.35f));

            ui_draw_text(buffer, text_x, text_y, dist_font_size, 0.08f, BLUE, false);
        }
        
        /* Orden de visitación */
        if (vista->opciones.mostrar_ordenes && v->orden_visitacion > 0) {
            snprintf(buffer, sizeof(buffer), "#%d", v->orden_visitacion);
            ui_draw_text(buffer, grafo_vista_px(vista, v->x) - 12.0f,
                         grafo_vista_py(vista, v->y) + (float)offset_y + 28.0f, 10.0f, 0.08f,
                         RED, false);
        }
    }
}

void grafo_vista_dibujar_estado(const GrafoVista *vista) {
    if (!vista || !vista->estado) return;
    
    int y_offset = (int)vista->area_renderizado.y + 10;
    int legend_y = (int)(vista->area_renderizado.y + vista->area_renderizado.height - 18.0f);
    
    /* Mensaje de estado */
    ui_draw_text(vista->estado->mensaje_estado, vista->area_renderizado.x + 10.0f, (float)y_offset,
                 14.0f, 0.08f, vista->colores.texto_normal, false);
    
    /* Información de algoritmo */
    if (vista->estado->algoritmo_activo != GRAFO_ALGO_NINGUNO) {
        char algo_str[128];
        const char *algo_nombre = "";
        
        switch (vista->estado->algoritmo_activo) {
            case GRAFO_ALGO_BFS: algo_nombre = "BFS"; break;
            case GRAFO_ALGO_DFS: algo_nombre = "DFS"; break;
            case GRAFO_ALGO_DIJKSTRA: algo_nombre = "Dijkstra"; break;
            case GRAFO_ALGO_BELLMAN_FORD: algo_nombre = "Bellman-Ford"; break;
            case GRAFO_ALGO_PRIM: algo_nombre = "Prim"; break;
            case GRAFO_ALGO_KRUSKAL: algo_nombre = "Kruskal"; break;
            default: break;
        }
        
        snprintf(algo_str, sizeof(algo_str), "Algoritmo: %s | Progreso %d/%d", 
            algo_nombre, vista->estado->paso_algoritmo + 1, vista->estado->total_pasos);
        ui_draw_text(algo_str, vista->area_renderizado.x + 10.0f, (float)y_offset + 20.0f, 12.0f,
                     0.08f, vista->colores.texto_destacado, false);
    }

        DrawRectangleRounded((Rectangle){vista->area_renderizado.x + 10.0f, (float)legend_y, 9.0f, 9.0f},
                 0.25f, 4, Fade(vista->colores.vertice_actual, 0.55f));
        ui_draw_text("Activo", vista->area_renderizado.x + 24.0f, (float)legend_y - 2.0f, 10.0f,
                     0.08f, vista->colores.texto_normal, false);
        DrawRectangleRounded((Rectangle){vista->area_renderizado.x + 82.0f, (float)legend_y, 9.0f, 9.0f},
                 0.25f, 4, Fade(vista->colores.arista_relajada, 0.55f));
        ui_draw_text("Procesada", vista->area_renderizado.x + 96.0f, (float)legend_y - 2.0f, 10.0f,
                     0.08f, vista->colores.texto_normal, false);
        DrawRectangleRounded((Rectangle){vista->area_renderizado.x + 172.0f, (float)legend_y, 9.0f, 9.0f},
                 0.25f, 4, Fade(vista->colores.arista_camino_minimo, 0.55f));
        ui_draw_text("Mejora", vista->area_renderizado.x + 186.0f, (float)legend_y - 2.0f, 10.0f,
                     0.08f, vista->colores.texto_normal, false);
}

/* ============================================================================
 * Renderizado Principal
 * ============================================================================ */

void grafo_vista_dibujar(GrafoVista *vista) {
    Rectangle canvas;
    if (!vista || !vista->estado) return;

    grafo_vista_ajustar_offset(vista);
    canvas = grafo_vista_area_lienzo(vista);
    grafo_vista_dibujar_fondo(vista);
    BeginScissorMode((int)canvas.x, (int)canvas.y, (int)canvas.width, (int)canvas.height);
    grafo_vista_dibujar_aristas(vista);
    grafo_vista_dibujar_pesos(vista);
    grafo_vista_dibujar_flechas(vista);
    grafo_vista_dibujar_vertices(vista);
    grafo_vista_dibujar_etiquetas(vista);
    EndScissorMode();
    grafo_vista_dibujar_estado(vista);
    grafo_vista_dibujar_scrolls(vista);
}

/* ============================================================================
 * Utilidades
 * ============================================================================ */

void grafo_vista_actualizar_area(GrafoVista *vista, Rectangle nueva_area) {
    if (!vista) return;
    
    vista->area_renderizado = nueva_area;
    vista->layout_config.ancho_panel = (int)nueva_area.width;
    vista->layout_config.alto_panel = (int)nueva_area.height;
    grafo_vista_ajustar_offset(vista);
    vista->necesita_redibujarse = true;
}

void grafo_vista_desplazar(GrafoVista *vista, float delta_x, float delta_y) {
    if (vista == NULL) {
        return;
    }
    vista->offset_x += delta_x;
    vista->offset_y += delta_y;
    grafo_vista_ajustar_offset(vista);
}

void grafo_vista_scroll_rueda(GrafoVista *vista, float wheel_delta, bool horizontal) {
    const float step = 32.0f;
    if (vista == NULL || wheel_delta == 0.0f) {
        return;
    }
    if (horizontal) {
        grafo_vista_desplazar(vista, wheel_delta * step, 0.0f);
    } else {
        grafo_vista_desplazar(vista, 0.0f, wheel_delta * step);
    }
}
