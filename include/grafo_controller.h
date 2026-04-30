/**
 * @file grafo_controller.h
 * @brief Controlador de interacción del grafo y ejecución de algoritmos
 * @author VisualStruct v2
 * 
 * Vincula entrada de usuario (UI) con operaciones del TAD Grafo y actualización
 * de estado visual para animación de algoritmos.
 */

#ifndef GRAFO_CONTROLLER_H
#define GRAFO_CONTROLLER_H

#include "grafo.h"
#include "grafo_state.h"
#include "grafo_view.h"
#include <stdbool.h>

/* ============================================================================
 * Estados del Controlador
 * ============================================================================ */

/**
 * @enum GrafoControllerModo
 * @brief Modo de operación del controlador
 */
typedef enum {
    GRAFO_MODO_EDICION,           /**< Agregando/editando vértices y aristas */
    GRAFO_MODO_ALGORITMO,         /**< Ejecutando algoritmo paso a paso */
    GRAFO_MODO_VISUALIZACION,     /**< Solo visualización, sin edición */
    GRAFO_MODO_PAUSA              /**< Pausa durante ejecución de algoritmo */
} GrafoControllerModo;

typedef enum {
    GRAFO_PASO_CONSOLIDACION = 0,
    GRAFO_PASO_VERTICE,
    GRAFO_PASO_ARISTA
} GrafoPasoTipo;

/**
 * @struct GrafoController
 * @brief Contexto completo del controlador
 */
typedef struct {
    Grafo *grafo_tad;             /**< Referencia al TAD Grafo */
    GrafoState estado_visual;      /**< Estado visual actual */
    GrafoVista vista;             /**< Contexto de visualización */
    
    GrafoControllerModo modo;     /**< Modo de operación */
    int algoritmo_seleccionado;   /**< Código del algoritmo */
    int vertice_inicio;           /**< Vértice inicial para BFS/DFS */
    int vertice_destino;          /**< Vértice destino para caminos */
    
    bool esta_ejecutando;         /**< True si algoritmo en ejecución */
    int paso_actual;              /**< Paso actual del algoritmo */
    int total_pasos;              /**< Total de pasos calculados */
    bool autoplay_activo;         /**< Avance automático habilitado */
    float autoplay_intervalo;     /**< Segundos entre pasos */
    float autoplay_acumulado;     /**< Acumulador interno para autoplay */
    int autoplay_velocidad_idx;   /**< Índice de velocidad actual */
    GrafoPasoTipo paso_tipo_actual; /**< Tipo de paso resaltado */
    bool paso_mejora;             /**< True si el paso actual produjo mejora */
    bool arista_actual_valida;    /**< True si hay arista actual activa */
    int arista_actual_origen;     /**< Origen de arista actual */
    int arista_actual_destino;    /**< Destino de arista actual */

    int script_vertices[256];     /**< Secuencia de vertices por paso (BFS/DFS/caminos) */
    int script_vertices_count;    /**< Cantidad de vertices de la secuencia */
    GrafoArista script_aristas[256]; /**< Secuencia de aristas por paso (caminos/MST) */
    int script_aristas_count;     /**< Cantidad de aristas de la secuencia */
    
    char mensaje_error[256];      /**< Último mensaje de error */
} GrafoController;

/* ============================================================================
 * Inicialización y Destrucción
 * ============================================================================ */

/**
 * @brief Crea un nuevo controlador
 * @param grafo Puntero al TAD Grafo (debe existir previamente)
 * @param area_renderizado Área en pantalla para renderizar
 * @return Estructura GrafoController inicializada
 */
GrafoController grafo_controller_crear(Grafo *grafo, Rectangle area_renderizado);

/**
 * @brief Destruye un controlador
 * @param controller Puntero al controlador a destruir
 */
void grafo_controller_destruir(GrafoController *controller);

/* ============================================================================
 * Operaciones de Grafo (Modo Edición)
 * ============================================================================ */

/**
 * @brief Agrega un vértice al grafo
 * @param controller Puntero al controlador
 * @param id_vertice ID del nuevo vértice
 * @return true si se agregó exitosamente
 */
bool grafo_controller_agregar_vertice(GrafoController *controller, int id_vertice);

/**
 * @brief Elimina un vértice del grafo
 * @param controller Puntero al controlador
 * @param id_vertice ID del vértice a eliminar
 * @return true si se eliminó exitosamente
 */
bool grafo_controller_eliminar_vertice(GrafoController *controller, int id_vertice);

/**
 * @brief Agrega una arista entre dos vértices
 * @param controller Puntero al controlador
 * @param id_origen ID del vértice origen
 * @param id_destino ID del vértice destino
 * @param peso Peso de la arista
 * @return true si se agregó exitosamente
 */
bool grafo_controller_agregar_arista(GrafoController *controller, 
                                    int id_origen, int id_destino, int peso);

/**
 * @brief Elimina una arista entre dos vértices
 * @param controller Puntero al controlador
 * @param id_origen ID del vértice origen
 * @param id_destino ID del vértice destino
 * @return true si se eliminó exitosamente
 */
bool grafo_controller_eliminar_arista(GrafoController *controller, 
                                     int id_origen, int id_destino);

/**
 * @brief Limpia el grafo (elimina todos los vértices y aristas)
 * @param controller Puntero al controlador
 */
void grafo_controller_limpiar(GrafoController *controller);

/* ============================================================================
 * Ejecución de Algoritmos
 * ============================================================================ */

/**
 * @brief Selecciona un algoritmo para ejecutar
 * @param controller Puntero al controlador
 * @param codigo_algoritmo Código de algoritmo (GRAFO_ALGO_*)
 * @param id_inicio Vértice inicial
 * @param id_destino Vértice destino (para algoritmos que lo requieren)
 * @return true si se seleccionó exitosamente
 */
bool grafo_controller_seleccionar_algoritmo(GrafoController *controller, 
                                            int codigo_algoritmo, 
                                            int id_inicio, int id_destino);

/**
 * @brief Inicia la ejecución de un algoritmo
 * @param controller Puntero al controlador
 * @return true si se inició exitosamente
 */
bool grafo_controller_iniciar_algoritmo(GrafoController *controller);

/**
 * @brief Avanza un paso en la ejecución del algoritmo
 * @param controller Puntero al controlador
 * @return true si avanzó, false si ya terminó
 */
bool grafo_controller_paso_siguiente(GrafoController *controller);

/**
 * @brief Retrocede un paso en la ejecución del algoritmo
 * @param controller Puntero al controlador
 * @return true si retrocedió, false si está en inicio
 */
bool grafo_controller_paso_anterior(GrafoController *controller);

/**
 * @brief Pausa la ejecución del algoritmo
 * @param controller Puntero al controlador
 */
void grafo_controller_pausar(GrafoController *controller);

/**
 * @brief Reanuda la ejecución del algoritmo
 * @param controller Puntero al controlador
 */
void grafo_controller_reanudar(GrafoController *controller);

/**
 * @brief Detiene y reinicia el algoritmo
 * @param controller Puntero al controlador
 */
void grafo_controller_reiniciar(GrafoController *controller);
void grafo_controller_ir_inicio(GrafoController *controller);
void grafo_controller_ir_final(GrafoController *controller);
void grafo_controller_toggle_autoplay(GrafoController *controller);
void grafo_controller_cambiar_velocidad(GrafoController *controller);
void grafo_controller_actualizar(GrafoController *controller, float delta_time);

/* ============================================================================
 * Interacción de Usuario
 * ============================================================================ */

/**
 * @brief Procesa entrada del mouse (clic sobre vértices)
 * @param controller Puntero al controlador
 * @param mouse_pos Posición del mouse en pantalla
 * @param boton_pulsado true si botón izquierdo pulsado
 */
void grafo_controller_procesar_mouse(GrafoController *controller, 
                                    Vector2 mouse_pos, bool boton_pulsado);

/**
 * @brief Establece modo de operación
 * @param controller Puntero al controlador
 * @param nuevo_modo Nuevo modo
 */
void grafo_controller_establecer_modo(GrafoController *controller, GrafoControllerModo nuevo_modo);

/* ============================================================================
 * Consultas y Visualización
 * ============================================================================ */

/**
 * @brief Obtiene mensaje de error del último error
 * @param controller Puntero al controlador
 * @return Puntero a cadena de error (stateless, no liberar)
 */
const char* grafo_controller_obtener_error(const GrafoController *controller);

/**
 * @brief Dibuja el grafo en pantalla
 * @param controller Puntero al controlador
 */
void grafo_controller_dibujar(GrafoController *controller);

/**
 * @brief Actualiza área de renderizado (para redimensionamiento)
 * @param controller Puntero al controlador
 * @param nueva_area Nuevo área
 */
void grafo_controller_actualizar_area(GrafoController *controller, Rectangle nueva_area);
const char *grafo_controller_tipo_paso_cadena(const GrafoController *controller);

#endif /* GRAFO_CONTROLLER_H */
