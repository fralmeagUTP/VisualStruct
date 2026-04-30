/**
 * @file grafo_view.h
 * @brief Renderizado visual de grafos con Raylib
 * @author VisualStruct v2
 * 
 * Dibuja vértices, aristas, pesos y estados del grafo en pantalla.
 * Depende de: Raylib, grafo_state.h, grafo_layout.h
 */

#ifndef GRAFO_VIEW_H
#define GRAFO_VIEW_H

#include "grafo_state.h"
#include "grafo_layout.h"
#include <raylib.h>

/* ============================================================================
 * Esquema de Colores
 * ============================================================================ */

/**
 * @struct GrafoVistaColores
 * @brief Paleta de colores para la visualización
 */
typedef struct {
    Color vertice_normal;         /**< Vértice no visitado */
    Color vertice_visitado;       /**< Vértice visitado */
    Color vertice_inicial;        /**< Vértice inicial (BFS/DFS) */
    Color vertice_destino;        /**< Vértice destino */
    Color vertice_actual;         /**< Vértice siendo procesado */
    
    Color arista_normal;          /**< Arista normal */
    Color arista_relajada;        /**< Arista relajada (Dijkstra) */
    Color arista_camino_minimo;   /**< Arista en camino mínimo */
    Color arista_mst;             /**< Arista en MST */
    Color arista_error;           /**< Arista con error */
    
    Color texto_normal;           /**< Texto estándar */
    Color texto_destacado;        /**< Texto destacado */
    Color fondo_panel;            /**< Fondo del panel */
    Color borde_panel;            /**< Borde del panel */
} GrafoVistaColores;

/**
 * @struct GrafoVistaOpciones
 * @brief Opciones de renderizado
 */
typedef struct {
    bool mostrar_pesos;           /**< Mostrar etiquetas de peso en aristas */
    bool mostrar_distancias;      /**< Mostrar distancias en vértices (Dijkstra) */
    bool mostrar_ordenes;         /**< Mostrar orden de visitación */
    bool mostrar_flechas;         /**< Mostrar flechas en grafos dirigidos */
    bool mostrar_etiquetas;       /**< Mostrar IDs de vértices */
    bool animar_algoritmo;        /**< Modo animación (pasos) */
    bool resaltar_camino;         /**< Resaltar camino encontrado */
    
    float grosor_arista_normal;   /**< Grosor de línea estándar */
    float grosor_arista_destacada; /**< Grosor de línea destacada */
    float tamaño_fuente;          /**< Tamaño de fuente para etiquetas */
} GrafoVistaOpciones;

/**
 * @struct GrafoVista
 * @brief Contexto completo de visualización
 */
typedef struct {
    const GrafoState *estado;     /**< Estado del grafo (no propiedad de vista) */
    GrafoLayoutConfiguracion layout_config;
    GrafoVistaColores colores;
    GrafoVistaOpciones opciones;
    
    Rectangle area_renderizado;   /**< Área de pantalla donde renderizar */
    bool necesita_redibujarse;    /**< Flag para optimizar redibujado */
} GrafoVista;

/* ============================================================================
 * Funciones de Inicialización
 * ============================================================================ */

/**
 * @brief Crea esquema de colores por defecto
 * @return Estructura de colores inicializada
 */
GrafoVistaColores grafo_vista_colores_defecto(void);

/**
 * @brief Crea opciones de renderizado por defecto
 * @return Estructura de opciones inicializada
 */
GrafoVistaOpciones grafo_vista_opciones_defecto(void);

/**
 * @brief Inicializa contexto de vista
 * @param estado Puntero al estado del grafo (debe persistir)
 * @param area_renderizado Rectángulo en pantalla para renderizar
 * @return Estructura GrafoVista inicializada
 */
GrafoVista grafo_vista_init(const GrafoState *estado, Rectangle area_renderizado);

/* ============================================================================
 * Funciones de Renderizado Principal
 * ============================================================================ */

/**
 * @brief Dibuja el grafo completo en pantalla
 * 
 * Renderiza en siguiente orden:
 * 1. Fondo y borde del panel
 * 2. Aristas (líneas y pesos)
 * 3. Vértices (círculos)
 * 4. Etiquetas (IDs, distancias, órdenes)
 * 5. Información de estado
 * 
 * @param vista Puntero al contexto de vista
 */
void grafo_vista_dibujar(GrafoVista *vista);

/**
 * @brief Dibuja solo el fondo y bordes del panel
 * @param vista Puntero al contexto de vista
 */
void grafo_vista_dibujar_fondo(const GrafoVista *vista);

/**
 * @brief Dibuja todas las aristas del grafo
 * @param vista Puntero al contexto de vista
 */
void grafo_vista_dibujar_aristas(const GrafoVista *vista);

/**
 * @brief Dibuja una arista específica
 * @param vista Puntero al contexto de vista
 * @param arista Arista visual a dibujar
 * @param v_origen Vértice de origen
 * @param v_destino Vértice de destino
 */
void grafo_vista_dibujar_arista_individual(const GrafoVista *vista, 
                                           const GrafoAristaVisual *arista,
                                           const GrafoVerticeVisual *v_origen,
                                           const GrafoVerticeVisual *v_destino);

/**
 * @brief Dibuja todas las aristas con flechas (para grafo dirigido)
 * @param vista Puntero al contexto de vista
 */
void grafo_vista_dibujar_flechas(const GrafoVista *vista);

/**
 * @brief Dibuja todos los vértices como círculos
 * @param vista Puntero al contexto de vista
 */
void grafo_vista_dibujar_vertices(const GrafoVista *vista);

/**
 * @brief Dibuja un vértice individual
 * @param vista Puntero al contexto de vista
 * @param vertice Vértice visual a dibujar
 */
void grafo_vista_dibujar_vertice_individual(const GrafoVista *vista, 
                                            const GrafoVerticeVisual *vertice);

/**
 * @brief Dibuja etiquetas (IDs, distancias, órdenes)
 * @param vista Puntero al contexto de vista
 */
void grafo_vista_dibujar_etiquetas(const GrafoVista *vista);

/**
 * @brief Dibuja pesos de aristas en centro de cada arista
 * @param vista Puntero al contexto de vista
 */
void grafo_vista_dibujar_pesos(const GrafoVista *vista);

/**
 * @brief Dibuja información de estado (algoritmo activo, paso actual, etc.)
 * @param vista Puntero al contexto de vista
 */
void grafo_vista_dibujar_estado(const GrafoVista *vista);

/* ============================================================================
 * Funciones Auxiliares de Renderizado
 * ============================================================================ */

/**
 * @brief Obtiene color para un vértice según su estado visual
 * @param vista Puntero al contexto de vista
 * @param estado Estado visual del vértice
 * @return Color correspondiente
 */
Color grafo_vista_color_vertice(const GrafoVista *vista, GrafoVerticeEstadoVisual estado);

/**
 * @brief Obtiene color para una arista según su estado visual
 * @param vista Puntero al contexto de vista
 * @param estado Estado visual de la arista
 * @return Color correspondiente
 */
Color grafo_vista_color_arista(const GrafoVista *vista, GrafoAristaEstadoVisual estado);

/**
 * @brief Dibuja flecha desde (x1,y1) a (x2,y2)
 * @param x1, y1 Punto de origen
 * @param x2, y2 Punto de destino
 * @param grosor Grosor de línea
 * @param color Color de la flecha
 */
void grafo_vista_dibujar_flecha(float x1, float y1, float x2, float y2, 
                               float grosor, Color color);

/**
 * @brief Calcula punto en línea a cierta distancia del origen
 * @param x1, y1 Punto de origen
 * @param x2, y2 Punto de destino
 * @param distancia Distancia desde origen
 * @param[out] px, py Punto calculado
 */
void grafo_vista_punto_en_linea(float x1, float y1, float x2, float y2, 
                               float distancia, float *px, float *py);

/**
 * @brief Detecta si cursor del mouse está sobre un vértice
 * @param vista Puntero al contexto de vista
 * @param mouse_pos Posición del mouse
 * @return ID del vértice (-1 si ninguno)
 */
int grafo_vista_detectar_vertice(const GrafoVista *vista, Vector2 mouse_pos);

/**
 * @brief Actualiza área de renderizado (cuando se redimensiona ventana)
 * @param vista Puntero al contexto de vista
 * @param nueva_area Nuevo rectángulo
 */
void grafo_vista_actualizar_area(GrafoVista *vista, Rectangle nueva_area);

#endif /* GRAFO_VIEW_H */
