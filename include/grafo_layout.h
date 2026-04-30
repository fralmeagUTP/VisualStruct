/**
 * @file grafo_layout.h
 * @brief Algoritmo de layout circular para posicionamiento de vértices
 * @author VisualStruct v2
 * 
 * Calcula las posiciones de vértices en disposición circular o fuerza dirigida.
 * Independiente de Raylib (solo usa matemática).
 */

#ifndef GRAFO_LAYOUT_H
#define GRAFO_LAYOUT_H

#include "grafo_state.h"

/* ============================================================================
 * Parámetros de Layout
 * ============================================================================ */

/**
 * @struct GrafoLayoutConfiguracion
 * @brief Parámetros para cálculo de layout
 */
typedef struct {
    int ancho_panel;               /**< Ancho del panel en píxeles */
    int alto_panel;                /**< Alto del panel en píxeles */
    float margen_borde;            /**< Margen desde bordes (en fracción: 0.0-1.0) */
    float radio_vertice;           /**< Radio de renderizado de vértice */
    bool usa_fuerza_dirigida;      /**< Si false: layout circular; si true: fuerzas (futuro) */
} GrafoLayoutConfiguracion;

/* ============================================================================
 * Enumeraciones de Algoritmos de Layout
 * ============================================================================ */

/**
 * @enum GrafoLayoutAlgoritmo
 * @brief Tipos de algoritmos de posicionamiento
 */
typedef enum {
    GRAFO_LAYOUT_CIRCULAR,         /**< Disposición circular */
    GRAFO_LAYOUT_FUERZA_DIRIGIDA,  /**< Repulsión/atracción (futuro) */
    GRAFO_LAYOUT_FILA,             /**< Línea horizontal (futuro) */
    GRAFO_LAYOUT_GRID              /**< Cuadrícula (futuro) */
} GrafoLayoutAlgoritmo;

/* ============================================================================
 * Funciones de Layout
 * ============================================================================ */

/**
 * @brief Crea configuración de layout por defecto
 * @param ancho_panel Ancho en píxeles
 * @param alto_panel Alto en píxeles
 * @return Estructura de configuración inicializada
 */
GrafoLayoutConfiguracion grafo_layout_config_defecto(int ancho_panel, int alto_panel);

/**
 * @brief Calcula posiciones circulares de vértices
 * 
 * Distribuye vértices uniformemente en un círculo dentro del panel.
 * Radio del círculo se adapta al tamaño del panel y cantidad de vértices.
 * 
 * @param estado Puntero al estado visual (se modifican x, y)
 * @param config Configuración de layout
 * @return true si se calculó exitosamente
 */
bool grafo_layout_calcular_circular(GrafoState *estado, const GrafoLayoutConfiguracion *config);

/**
 * @brief Calcula distancia euclideana entre dos posiciones
 * @param x1, y1 Coordenadas del primer punto
 * @param x2, y2 Coordenadas del segundo punto
 * @return Distancia euclideana
 */
float grafo_layout_distancia(float x1, float y1, float x2, float y2);

/**
 * @brief Ajusta posición para no colisionar con otro vértice
 * @param estado Puntero al estado visual
 * @param idx_vertice Índice del vértice a ajustar
 * @param distancia_minima Distancia mínima entre centros
 * @return true si ajustó exitosamente
 */
bool grafo_layout_evitar_colisiones(GrafoState *estado, int idx_vertice, float distancia_minima);

/**
 * @brief Obtiene centro del panel
 * @param config Configuración de layout
 * @param[out] centro_x Coordenada X del centro
 * @param[out] centro_y Coordenada Y del centro
 */
void grafo_layout_obtener_centro(const GrafoLayoutConfiguracion *config, 
                                 float *centro_x, float *centro_y);

/**
 * @brief Calcula radio máximo disponible para disposición circular
 * @param config Configuración de layout
 * @param cantidad_vertices Cantidad de vértices a distribuir
 * @return Radio en píxeles
 */
float grafo_layout_calcular_radio_maximo(const GrafoLayoutConfiguracion *config, 
                                        int cantidad_vertices);

#endif /* GRAFO_LAYOUT_H */
