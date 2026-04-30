/**
 * @file grafo_state.h
 * @brief Estado visual del grafo para animaciones y visualización
 * @author VisualStruct v2
 * 
 * Maneja el estado visual de vértices y aristas durante la ejecución de algoritmos.
 * Limita a 64 vértices y 256 aristas para optimizar renderizado en tiempo real.
 * No depende de Raylib (independencia de capas de presentación).
 */

#ifndef GRAFO_STATE_H
#define GRAFO_STATE_H

#include <stdbool.h>
#include <stddef.h>

/* ============================================================================
 * Enumeraciones de Estados Visuales
 * ============================================================================ */

/**
 * @enum GrafoVerticeEstadoVisual
 * @brief Estados visuales de un vértice durante algoritmos
 */
typedef enum {
    GRAFO_VÉRTICE_NORMAL,      /**< Color base, no visitado */
    GRAFO_VÉRTICE_VISITADO,    /**< Color gris, ya procesado */
    GRAFO_VÉRTICE_INICIAL,     /**< Color azul, vértice de inicio */
    GRAFO_VÉRTICE_DESTINO,     /**< Color rojo, vértice meta */
    GRAFO_VÉRTICE_ACTUAL       /**< Color amarillo, procesándose ahora */
} GrafoVerticeEstadoVisual;

/**
 * @enum GrafoAristaEstadoVisual
 * @brief Estados visuales de una arista durante algoritmos
 */
typedef enum {
    GRAFO_ARISTA_NORMAL,       /**< Color base, no examinada */
    GRAFO_ARISTA_RELAJADA,     /**< Color naranja, distancia relajada en Dijkstra/BF */
    GRAFO_ARISTA_CAMINO_MINIMO, /**< Color verde, forma parte del camino mínimo */
    GRAFO_ARISTA_MST,          /**< Color púrpura, forma parte del MST */
    GRAFO_ARISTA_ERROR         /**< Color rojo, error en algoritmo (ciclo negativo) */
} GrafoAristaEstadoVisual;

/* ============================================================================
 * Estructuras de Estado Visual
 * ============================================================================ */

/**
 * @struct GrafoVerticeVisual
 * @brief Estado visual y datos de renderizado de un vértice
 * 
 * Limita a 64 vértices por razones de rendimiento.
 * Coordenadas (x, y) se calculan en la capa de layout.
 */
typedef struct {
    int id;                          /**< ID del vértice [0..63] */
    float x, y;                      /**< Posición en píxeles */
    float radio;                     /**< Radio de renderizado */
    GrafoVerticeEstadoVisual estado; /**< Estado visual actual */
    bool visible;                    /**< True si debe renderizarse */
    int distancia;                   /**< Distancia acumulada (Dijkstra/BF) */
    int predecesor;                  /**< Vértice predecesor en árbol de caminos (-1 si ninguno) */
    int orden_visitacion;            /**< Orden en que fue visitado (0 si no visitado) */
} GrafoVerticeVisual;

/**
 * @struct GrafoAristaVisual
 * @brief Estado visual de una arista durante visualización
 * 
 * Limita a 256 aristas. Las coordenadas de inicio/fin se derivan
 * del estado visual de vértices.
 */
typedef struct {
    int origen, destino;             /**< IDs de vértices conectados */
    int peso;                        /**< Peso de la arista */
    GrafoAristaEstadoVisual estado;  /**< Estado visual actual */
    bool visible;                    /**< True si debe renderizarse */
    bool es_dirigida;                /**< True si es arista dirigida */
    int orden_examinacion;           /**< Orden en que fue examinada (0 si no examinada) */
} GrafoAristaVisual;

/**
 * @struct GrafoState
 * @brief Estado completo de visualización de un grafo
 * 
 * Contiene todos los vértices y aristas con su estado visual actual.
 * Máximo 64 vértices, 256 aristas.
 */
typedef struct {
    GrafoVerticeVisual vertices[64];  /**< Array de vértices visuales */
    int cantidad_vertices;            /**< Cantidad de vértices activos */
    
    GrafoAristaVisual aristas[256];   /**< Array de aristas visuales */
    int cantidad_aristas;             /**< Cantidad de aristas activas */
    
    bool es_dirigido;                 /**< True si grafo es dirigido */
    int algoritmo_activo;             /**< Código del algoritmo ejecutándose (0=ninguno) */
    int paso_algoritmo;               /**< Paso actual en ejecución */
    int total_pasos;                  /**< Total de pasos del algoritmo */
    
    char mensaje_estado[256];         /**< Mensaje descriptivo del estado actual */
} GrafoState;

/* ============================================================================
 * Enumeraciones de Códigos de Algoritmo
 * ============================================================================ */

typedef enum {
    GRAFO_ALGO_NINGUNO = 0,
    GRAFO_ALGO_BFS = 1,
    GRAFO_ALGO_DFS = 2,
    GRAFO_ALGO_DIJKSTRA = 3,
    GRAFO_ALGO_BELLMAN_FORD = 4,
    GRAFO_ALGO_PRIM = 5,
    GRAFO_ALGO_KRUSKAL = 6
} GrafoAlgoritmoActivo;

/* ============================================================================
 * Funciones de Inicialización y Destrucción
 * ============================================================================ */

/**
 * @brief Inicializa un estado visual vacío
 * @return Estructura GrafoState inicializada
 */
GrafoState grafo_state_init(void);

/**
 * @brief Destruye un estado visual (actualmente no hace nada dinámico)
 * @param estado Puntero al estado a destruir
 */
void grafo_state_destruir(GrafoState *estado);

/* ============================================================================
 * Funciones de Actualización de Estado
 * ============================================================================ */

/**
 * @brief Marca un vértice como visitado y asigna su orden de visitación
 * @param estado Puntero al estado visual
 * @param id_vertice ID del vértice
 * @param orden Número de orden en la secuencia
 * @return true si se actualizó, false si vértice no existe
 */
bool grafo_state_marcar_vertice_visitado(GrafoState *estado, int id_vertice, int orden);

/**
 * @brief Establece el estado visual de un vértice
 * @param estado Puntero al estado visual
 * @param id_vertice ID del vértice
 * @param nuevo_estado Nuevo estado visual
 * @return true si se actualizó, false si vértice no existe
 */
bool grafo_state_establecer_vertice_estado(GrafoState *estado, int id_vertice, 
                                          GrafoVerticeEstadoVisual nuevo_estado);

/**
 * @brief Actualiza distancia y predecesor de un vértice
 * @param estado Puntero al estado visual
 * @param id_vertice ID del vértice
 * @param nueva_distancia Nueva distancia acumulada
 * @param id_predecesor ID del vértice predecesor
 * @return true si se actualizó, false si vértice no existe
 */
bool grafo_state_actualizar_distancia_vertice(GrafoState *estado, int id_vertice, 
                                             int nueva_distancia, int id_predecesor);

/**
 * @brief Establece el estado visual de una arista
 * @param estado Puntero al estado visual
 * @param id_origen ID vértice origen
 * @param id_destino ID vértice destino
 * @param nuevo_estado Nuevo estado visual
 * @return true si se actualizó, false si arista no existe
 */
bool grafo_state_establecer_arista_estado(GrafoState *estado, int id_origen, 
                                         int id_destino, GrafoAristaEstadoVisual nuevo_estado);

/**
 * @brief Reinicia todos los vértices y aristas a estado NORMAL
 * @param estado Puntero al estado visual
 */
void grafo_state_reiniciar_visuales(GrafoState *estado);

/**
 * @brief Carga vértices desde un grafo (implementado en fase de integración)
 * @param estado Puntero al estado visual destino
 * @param grafo Grafo fuente (puntero opaco)
 * @param cantidad Cantidad de vértices a cargar
 * @return true si se cargó exitosamente
 */
bool grafo_state_cargar_vertices(GrafoState *estado, const void *grafo, int cantidad);

/**
 * @brief Carga aristas desde un grafo (implementado en fase de integración)
 * @param estado Puntero al estado visual destino
 * @param grafo Grafo fuente (puntero opaco)
 * @param cantidad Cantidad de aristas a cargar
 * @return true si se cargó exitosamente
 */
bool grafo_state_cargar_aristas(GrafoState *estado, const void *grafo, int cantidad);

/* ============================================================================
 * Funciones de Consulta
 * ============================================================================ */

/**
 * @brief Obtiene el vértice visual con dado ID
 * @param estado Puntero al estado visual
 * @param id_vertice ID del vértice
 * @return Puntero a GrafoVerticeVisual si existe, NULL si no existe
 */
const GrafoVerticeVisual* grafo_state_obtener_vertice(const GrafoState *estado, int id_vertice);

/**
 * @brief Obtiene una arista visual dados origen y destino
 * @param estado Puntero al estado visual
 * @param id_origen ID vértice origen
 * @param id_destino ID vértice destino
 * @return Puntero a GrafoAristaVisual si existe, NULL si no existe
 */
const GrafoAristaVisual* grafo_state_obtener_arista(const GrafoState *estado, 
                                                    int id_origen, int id_destino);

/**
 * @brief Cuenta cuántos vértices están en estado VISITADO
 * @param estado Puntero al estado visual
 * @return Cantidad de vértices visitados
 */
int grafo_state_contar_visitados(const GrafoState *estado);

#endif /* GRAFO_STATE_H */
