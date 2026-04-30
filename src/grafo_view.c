/**
 * @file grafo_view.c
 * @brief Implementación del renderizado visual con Raylib
 */

#include "grafo_view.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ============================================================================
 * Inicialización
 * ============================================================================ */

GrafoVistaColores grafo_vista_colores_defecto(void) {
    GrafoVistaColores c;
    c.vertice_normal = GRAY;
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
    vista.layout_config = grafo_layout_config_defecto((int)area_renderizado.width, 
                                                      (int)area_renderizado.height);
    vista.colores = grafo_vista_colores_defecto();
    vista.opciones = grafo_vista_opciones_defecto();
    vista.necesita_redibujarse = true;
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
    DrawLineEx((Vector2){x1, y1}, (Vector2){x2, y2}, grosor, color);
    
    /* Calcular punta de flecha */
    float dx = x2 - x1;
    float dy = y2 - y1;
    float len = sqrtf(dx * dx + dy * dy);
    
    if (len > 0.001f) {
        float px = x2 - (dx / len) * 15.0f;
        float py = y2 - (dy / len) * 15.0f;
        
        /* Líneas laterales de punta */
        float px1 = px + (dy / len) * 5.0f;
        float py1 = py - (dx / len) * 5.0f;
        float px2 = px - (dy / len) * 5.0f;
        float py2 = py + (dx / len) * 5.0f;
        
        DrawLineEx((Vector2){x2, y2}, (Vector2){px1, py1}, grosor, color);
        DrawLineEx((Vector2){x2, y2}, (Vector2){px2, py2}, grosor, color);
    }
}

int grafo_vista_detectar_vertice(const GrafoVista *vista, Vector2 mouse_pos) {
    if (!vista || !vista->estado) return -1;
    
    for (int i = 0; i < vista->estado->cantidad_vertices; i++) {
        const GrafoVerticeVisual *v = &vista->estado->vertices[i];
        if (!v->visible) continue;
        
        float dx = mouse_pos.x - v->x;
        float dy = mouse_pos.y - v->y;
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
    
    DrawLineEx((Vector2){v_origen->x, v_origen->y}, 
              (Vector2){v_destino->x, v_destino->y}, 
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
        
        /* Calcular punto de inicio de flecha (desde borde del círculo) */
        float px, py;
        grafo_vista_punto_en_linea(v_origen->x, v_origen->y, v_destino->x, v_destino->y,
                                  v_origen->radio + 2.0f, &px, &py);
        
        grafo_vista_dibujar_flecha(px, py, v_destino->x, v_destino->y, 2.0f, color);
    }
}

void grafo_vista_dibujar_vertice_individual(const GrafoVista *vista, 
                                            const GrafoVerticeVisual *vertice) {
    if (!vista || !vertice || !vertice->visible) return;
    
    Color color = grafo_vista_color_vertice(vista, vertice->estado);
    DrawCircle((int)vertice->x, (int)vertice->y, vertice->radio, color);
    DrawCircleLines((int)vertice->x, (int)vertice->y, vertice->radio, vista->colores.texto_normal);
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
        
        /* Punto central de la arista */
        float cx = (v_origen->x + v_destino->x) / 2.0f;
        float cy = (v_origen->y + v_destino->y) / 2.0f;
        
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%d", arista->peso);
        DrawText(buffer, (int)(cx - 8), (int)(cy - 8), 12, vista->colores.texto_normal);
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
        DrawText(buffer, (int)(v->x - 8), (int)(v->y + offset_y), 12, vista->colores.texto_normal);
        
        /* Distancia (si está activa) */
        if (vista->opciones.mostrar_distancias && v->distancia > 0) {
            snprintf(buffer, sizeof(buffer), "d=%d", v->distancia);
            DrawText(buffer, (int)(v->x - 16), (int)(v->y + offset_y + 15), 10, BLUE);
        }
        
        /* Orden de visitación */
        if (vista->opciones.mostrar_ordenes && v->orden_visitacion > 0) {
            snprintf(buffer, sizeof(buffer), "#%d", v->orden_visitacion);
            DrawText(buffer, (int)(v->x - 12), (int)(v->y + offset_y + 28), 10, RED);
        }
    }
}

void grafo_vista_dibujar_estado(const GrafoVista *vista) {
    if (!vista || !vista->estado) return;
    
    int y_offset = (int)vista->area_renderizado.y + 10;
    int legend_y = (int)(vista->area_renderizado.y + vista->area_renderizado.height - 18.0f);
    
    /* Mensaje de estado */
    DrawText(vista->estado->mensaje_estado, 
            (int)vista->area_renderizado.x + 10, y_offset, 14, vista->colores.texto_normal);
    
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
        DrawText(algo_str, (int)vista->area_renderizado.x + 10, y_offset + 20, 12, 
                vista->colores.texto_destacado);
    }

        DrawRectangleRounded((Rectangle){vista->area_renderizado.x + 10.0f, (float)legend_y, 9.0f, 9.0f},
                 0.25f, 4, Fade(vista->colores.vertice_actual, 0.55f));
        DrawText("Activo", (int)vista->area_renderizado.x + 24, legend_y - 2, 10,
             vista->colores.texto_normal);
        DrawRectangleRounded((Rectangle){vista->area_renderizado.x + 82.0f, (float)legend_y, 9.0f, 9.0f},
                 0.25f, 4, Fade(vista->colores.arista_relajada, 0.55f));
        DrawText("Procesada", (int)vista->area_renderizado.x + 96, legend_y - 2, 10,
             vista->colores.texto_normal);
        DrawRectangleRounded((Rectangle){vista->area_renderizado.x + 172.0f, (float)legend_y, 9.0f, 9.0f},
                 0.25f, 4, Fade(vista->colores.arista_camino_minimo, 0.55f));
        DrawText("Mejora", (int)vista->area_renderizado.x + 186, legend_y - 2, 10,
             vista->colores.texto_normal);
}

/* ============================================================================
 * Renderizado Principal
 * ============================================================================ */

void grafo_vista_dibujar(GrafoVista *vista) {
    if (!vista || !vista->estado) return;
    
    grafo_vista_dibujar_fondo(vista);
    grafo_vista_dibujar_aristas(vista);
    grafo_vista_dibujar_pesos(vista);
    grafo_vista_dibujar_flechas(vista);
    grafo_vista_dibujar_vertices(vista);
    grafo_vista_dibujar_etiquetas(vista);
    grafo_vista_dibujar_estado(vista);
}

/* ============================================================================
 * Utilidades
 * ============================================================================ */

void grafo_vista_actualizar_area(GrafoVista *vista, Rectangle nueva_area) {
    if (!vista) return;
    
    vista->area_renderizado = nueva_area;
    vista->layout_config.ancho_panel = (int)nueva_area.width;
    vista->layout_config.alto_panel = (int)nueva_area.height;
    vista->necesita_redibujarse = true;
}
