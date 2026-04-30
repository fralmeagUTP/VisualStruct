/**
 * @file grafo_layout.c
 * @brief Implementación del layout circular para grafos
 */

#include "grafo_layout.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ============================================================================
 * Configuración
 * ============================================================================ */

GrafoLayoutConfiguracion grafo_layout_config_defecto(int ancho_panel, int alto_panel) {
    GrafoLayoutConfiguracion config;
    config.ancho_panel = ancho_panel;
    config.alto_panel = alto_panel;
    config.margen_borde = 0.1f;  /* 10% de margen */
    config.radio_vertice = 15.0f;
    config.usa_fuerza_dirigida = false;
    return config;
}

/* ============================================================================
 * Funciones Auxiliares
 * ============================================================================ */

void grafo_layout_obtener_centro(const GrafoLayoutConfiguracion *config, 
                                 float *centro_x, float *centro_y) {
    if (!config || !centro_x || !centro_y) return;
    
    *centro_x = config->ancho_panel / 2.0f;
    *centro_y = config->alto_panel / 2.0f;
}

float grafo_layout_distancia(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

float grafo_layout_calcular_radio_maximo(const GrafoLayoutConfiguracion *config, 
                                        int cantidad_vertices) {
    if (!config || cantidad_vertices <= 0) return 50.0f;
    
    float ancho_util = config->ancho_panel * (1.0f - 2.0f * config->margen_borde);
    float alto_util = config->alto_panel * (1.0f - 2.0f * config->margen_borde);
    
    /* Radio máximo es el menor de ancho/alto dividido entre 2 */
    float radio_max = (ancho_util < alto_util ? ancho_util : alto_util) / 2.0f;
    
    /* Ajustar si hay muchos vértices (ángulo inter-vértice muy pequeño) */
    if (cantidad_vertices > 30) {
        radio_max *= 0.8f;  /* Reducir un 20% para evitar solapamientos */
    }
    
    return radio_max;
}

/* ============================================================================
 * Layout Circular
 * ============================================================================ */

bool grafo_layout_calcular_circular(GrafoState *estado, const GrafoLayoutConfiguracion *config) {
    if (!estado || !config) return false;
    if (estado->cantidad_vertices == 0) return true;
    
    float centro_x, centro_y;
    grafo_layout_obtener_centro(config, &centro_x, &centro_y);
    
    float radio = grafo_layout_calcular_radio_maximo(config, estado->cantidad_vertices);
    
    /* Distribuir vértices uniformemente en círculo */
    for (int i = 0; i < estado->cantidad_vertices; i++) {
        float angulo = 2.0f * M_PI * i / estado->cantidad_vertices;
        
        estado->vertices[i].x = centro_x + radio * cosf(angulo);
        estado->vertices[i].y = centro_y + radio * sinf(angulo);
        estado->vertices[i].radio = config->radio_vertice;
        estado->vertices[i].visible = true;
    }
    
    /* Marcar aristas como visibles */
    for (int i = 0; i < estado->cantidad_aristas; i++) {
        estado->aristas[i].visible = true;
    }
    
    return true;
}

bool grafo_layout_evitar_colisiones(GrafoState *estado, int idx_vertice, float distancia_minima) {
    if (!estado || idx_vertice < 0 || idx_vertice >= estado->cantidad_vertices) {
        return false;
    }
    
    GrafoVerticeVisual *v = &estado->vertices[idx_vertice];
    
    /* Verificar colisiones con otros vértices */
    bool hay_colision = true;
    int iteraciones = 0;
    
    while (hay_colision && iteraciones < 10) {
        hay_colision = false;
        
        for (int i = 0; i < estado->cantidad_vertices; i++) {
            if (i == idx_vertice) continue;
            
            float dist = grafo_layout_distancia(v->x, v->y, 
                                               estado->vertices[i].x, estado->vertices[i].y);
            
            if (dist < distancia_minima) {
                hay_colision = true;
                
                /* Empujar hacia afuera */
                float dx = v->x - estado->vertices[i].x;
                float dy = v->y - estado->vertices[i].y;
                float len = sqrtf(dx * dx + dy * dy);
                
                if (len > 0.001f) {
                    dx /= len;
                    dy /= len;
                    v->x = estado->vertices[i].x + dx * distancia_minima;
                    v->y = estado->vertices[i].y + dy * distancia_minima;
                }
            }
        }
        iteraciones++;
    }
    
    return !hay_colision;
}
