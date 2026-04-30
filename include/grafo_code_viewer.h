/**
 * @file grafo_code_viewer.h
 * @brief Visualización de pseudocódigo de algoritmos
 * @author VisualStruct v2
 * 
 * Muestra fragmentos de código C para cada algoritmo en el panel de
 * visualización junto con el estado actual de ejecución.
 */

#ifndef GRAFO_CODE_VIEWER_H
#define GRAFO_CODE_VIEWER_H

#include <stdbool.h>
#include <raylib.h>

/* ============================================================================
 * Estructura de Código
 * ============================================================================ */

/**
 * @struct GrafoCodigoLinea
 * @brief Una línea de código con información de resaltado
 */
typedef struct {
    char contenido[256];          /**< Texto de la línea */
    int numero_linea;             /**< Número de línea (1-based) */
    bool es_linea_actual;         /**< True si es la que se está ejecutando */
    bool esta_resaltada;          /**< True si tiene resaltado especial */
} GrafoCodigoLinea;

/**
 * @struct GrafoCodigoAlgoritmo
 * @brief Pseudocódigo completo de un algoritmo
 */
typedef struct {
    GrafoCodigoLinea lineas[100]; /**< Array de líneas de código */
    int cantidad_lineas;          /**< Cantidad de líneas */
    int linea_actual;             /**< Índice de línea ejecutándose */
    char nombre_algoritmo[64];    /**< Nombre del algoritmo */
} GrafoCodigoAlgoritmo;

/* ============================================================================
 * Funciones de Inicialización
 * ============================================================================ */

/**
 * @brief Obtiene el pseudocódigo de BFS
 * @return Estructura con código de BFS
 */
GrafoCodigoAlgoritmo grafo_codigo_bfs(void);

/**
 * @brief Obtiene el pseudocódigo de DFS
 * @return Estructura con código de DFS
 */
GrafoCodigoAlgoritmo grafo_codigo_dfs(void);

/**
 * @brief Obtiene el pseudocódigo de Dijkstra
 * @return Estructura con código de Dijkstra
 */
GrafoCodigoAlgoritmo grafo_codigo_dijkstra(void);

/**
 * @brief Obtiene el pseudocódigo de Bellman-Ford
 * @return Estructura con código de Bellman-Ford
 */
GrafoCodigoAlgoritmo grafo_codigo_bellman_ford(void);

/**
 * @brief Obtiene el pseudocódigo de Prim
 * @return Estructura con código de Prim
 */
GrafoCodigoAlgoritmo grafo_codigo_prim(void);

/**
 * @brief Obtiene el pseudocódigo de Kruskal
 * @return Estructura con código de Kruskal
 */
GrafoCodigoAlgoritmo grafo_codigo_kruskal(void);

/* ============================================================================
 * Funciones de Renderizado
 * ============================================================================ */

/**
 * @brief Dibuja el pseudocódigo en pantalla
 * @param codigo Estructura con el código
 * @param area_destino Rectángulo donde dibuja
 */
void grafo_codigo_dibujar(const GrafoCodigoAlgoritmo *codigo, Rectangle area_destino);

/**
 * @brief Establece la línea actual que se está ejecutando
 * @param codigo Puntero a la estructura de código
 * @param numero_linea Número de línea (1-based)
 */
void grafo_codigo_establecer_linea_actual(GrafoCodigoAlgoritmo *codigo, int numero_linea);

/**
 * @brief Resalta una línea específica
 * @param codigo Puntero a la estructura de código
 * @param numero_linea Número de línea (1-based)
 * @param resaltar true para resaltar, false para desresaltar
 */
void grafo_codigo_resaltar_linea(GrafoCodigoAlgoritmo *codigo, int numero_linea, bool resaltar);

#endif /* GRAFO_CODE_VIEWER_H */
