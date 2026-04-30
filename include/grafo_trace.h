/**
 * @file grafo_trace.h
 * @brief Traza pedagógica de ejecución de algoritmos
 * @author VisualStruct v2
 * 
 * Almacena y visualiza la ejecución paso a paso de algoritmos con explicaciones
 * detalladas en español para propósitos educativos.
 */

#ifndef GRAFO_TRACE_H
#define GRAFO_TRACE_H

#include <stdbool.h>
#include <raylib.h>

/* ============================================================================
 * Estructura de Paso de Traza
 * ============================================================================ */

/**
 * @struct GrafoTracePaso
 * @brief Un paso individual en la ejecución de un algoritmo
 */
typedef struct {
    int numero_paso;              /**< Número del paso (1-based) */
    char descripcion[256];        /**< Descripción en español de qué pasa */
    char variables[512];          /**< Estado de variables clave */
    char vertices_procesados[64]; /**< IDs de vértices procesados */
    char aristas_examinadas[128]; /**< Aristas examinadas en este paso */
    int linea_codigo_actual;      /**< Línea del pseudocódigo siendo ejecutada */
} GrafoTracePaso;

/**
 * @struct GrafoTrace
 * @brief Traza completa de ejecución de un algoritmo
 */
typedef struct {
    GrafoTracePaso pasos[200];    /**< Array de pasos */
    int cantidad_pasos;           /**< Cantidad de pasos registrados */
    int paso_actual;              /**< Índice del paso actual */
    char nombre_algoritmo[64];    /**< Nombre del algoritmo */
    char resultado_final[256];    /**< Descripción del resultado */
} GrafoTrace;

/* ============================================================================
 * Funciones de Inicialización
 * ============================================================================ */

/**
 * @brief Crea una traza vacía para un algoritmo
 * @param nombre_algoritmo Nombre del algoritmo
 * @return Estructura de traza inicializada
 */
GrafoTrace grafo_trace_crear(const char *nombre_algoritmo);

/**
 * @brief Agrega un paso a la traza
 * @param traza Puntero a la estructura de traza
 * @param descripcion Descripción del paso
 * @param variables Estado de variables
 * @param linea_codigo Línea del pseudocódigo
 * @return true si se agregó exitosamente
 */
bool grafo_trace_agregar_paso(GrafoTrace *traza, const char *descripcion, 
                             const char *variables, int linea_codigo);

/**
 * @brief Establece el resultado final de la ejecución
 * @param traza Puntero a la estructura de traza
 * @param resultado Descripción del resultado
 */
void grafo_trace_establecer_resultado(GrafoTrace *traza, const char *resultado);

/* ============================================================================
 * Navegación
 * ============================================================================ */

/**
 * @brief Avanza al siguiente paso
 * @param traza Puntero a la estructura de traza
 * @return true si hay siguiente paso
 */
bool grafo_trace_paso_siguiente(GrafoTrace *traza);

/**
 * @brief Retrocede al paso anterior
 * @param traza Puntero a la estructura de traza
 * @return true si hay paso anterior
 */
bool grafo_trace_paso_anterior(GrafoTrace *traza);

/**
 * @brief Salta a un paso específico
 * @param traza Puntero a la estructura de traza
 * @param numero_paso Número de paso (1-based)
 * @return true si el paso existe
 */
bool grafo_trace_saltar_paso(GrafoTrace *traza, int numero_paso);

/* ============================================================================
 * Renderizado
 * ============================================================================ */

/**
 * @brief Dibuja la información del paso actual
 * @param traza Estructura de traza
 * @param area_destino Rectángulo donde dibuja
 */
void grafo_trace_dibujar(const GrafoTrace *traza, Rectangle area_destino);

/**
 * @brief Dibuja barra de progreso de pasos
 * @param traza Estructura de traza
 * @param area_barra Rectángulo para la barra
 */
void grafo_trace_dibujar_barra_progreso(const GrafoTrace *traza, Rectangle area_barra);

/* ============================================================================
 * Consultas
 * ============================================================================ */

/**
 * @brief Obtiene el paso actual
 * @param traza Estructura de traza
 * @return Puntero al paso actual
 */
const GrafoTracePaso* grafo_trace_obtener_paso_actual(const GrafoTrace *traza);

/**
 * @brief Obtiene información de progreso
 * @param traza Estructura de traza
 * @return Cadena con "paso X de Y"
 */
const char* grafo_trace_obtener_progreso(const GrafoTrace *traza);

#endif /* GRAFO_TRACE_H */
