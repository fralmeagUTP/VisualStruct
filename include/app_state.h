#ifndef APP_STATE_H
#define APP_STATE_H

#include <stdbool.h>

#include "cola.h"
#include "cola_prioridad.h"
#include "lista.h"
#include "lista_circular.h"
#include "pila.h"
#include "sublista.h"

/**
 * @file app_state.h
 * @brief Estado global y acciones de alto nivel de la aplicacion.
 */

typedef enum {
    ESTRUCTURA_PILA = 0,
    ESTRUCTURA_COLA,
    ESTRUCTURA_COLA_PRIORIDAD,
    ESTRUCTURA_LISTA,
    ESTRUCTURA_LISTA_CIRCULAR,
    ESTRUCTURA_SUBLISTA
} TipoEstructura;

typedef enum {
    OPERACION_NINGUNA = 0,
    OPERACION_INICIALIZAR,
    OPERACION_INSERTAR,
    OPERACION_INSERTAR_INICIO,
    OPERACION_INSERTAR_FINAL,
    OPERACION_ELIMINAR,
    OPERACION_BUSCAR,
    OPERACION_INVERTIR,
    OPERACION_SUBLISTA_INSERTAR_HIJO,
    OPERACION_SUBLISTA_ELIMINAR_HIJO,
    OPERACION_VACIAR
} TipoOperacion;

typedef struct {
    TipoEstructura estructura_activa;
    TipoOperacion operacion_actual;
    TipoOperacion operacion_animada;
    int input_valor;
    int input_prioridad;
    int ultimo_valor;
    int ultima_prioridad;
    int coincidencias_busqueda;
    int sublista_padre_activo;
    float animacion_feedback;
    float animacion_cambio_estructura;
    float animacion_pulso_panel;
    bool ultima_operacion_ok;
    bool sublista_padre_activo_ok;
    unsigned int operacion_serial;
    char mensaje_operacion[256];

    Pila pila;
    Cola cola;
    ColaPrioridad cola_prioridad;
    Lista lista;
    ListaCircular lista_circular;
    Nodo *sublista;
} AppState;

/** @brief Inicializa estado global y TADs base. */
void app_state_init(AppState *state);
/** @brief Libera memoria de TADs y limpia estado. */
void app_state_shutdown(AppState *state);
/** @brief Cambia la estructura seleccionada en el menu. */
void app_state_set_estructura(AppState *state, TipoEstructura estructura);
/** @brief Incrementa/decrementa el valor de entrada principal. */
void app_state_ajustar_valor(AppState *state, int delta);
/** @brief Asigna un valor de entrada principal. */
void app_state_set_valor(AppState *state, int value);
/** @brief Incrementa/decrementa prioridad de entrada [1..99]. */
void app_state_ajustar_prioridad(AppState *state, int delta);
/** @brief Asigna prioridad de entrada dentro del rango valido. */
void app_state_set_prioridad(AppState *state, int value);
/** @brief Avanza animaciones visuales efimeras ligadas al estado. */
void app_state_update_visuals(AppState *state, float delta_time);

/** @brief Ejecuta operacion de inicializar para la estructura activa. */
void app_state_operacion_inicializar(AppState *state);
/** @brief Ejecuta operacion insertar/encolar/push segun estructura activa. */
void app_state_operacion_insertar(AppState *state);
/** @brief Inserta al inicio cuando la estructura activa es lista o lista circular. */
void app_state_operacion_lista_insertar_inicio(AppState *state);
/** @brief Inserta al final cuando la estructura activa es lista o lista circular. */
void app_state_operacion_lista_insertar_final(AppState *state);
/** @brief Ejecuta operacion eliminar/desencolar/pop segun estructura activa. */
void app_state_operacion_eliminar(AppState *state);
/** @brief Ejecuta busqueda en lista, lista circular o seleccion de padre en sublistas. */
void app_state_operacion_buscar(AppState *state);
/** @brief Ejecuta inversion en lista o lista circular. */
void app_state_operacion_invertir(AppState *state);
/** @brief Inserta un hijo en el padre activo del TAD Sublista. */
void app_state_operacion_sublista_insertar_hijo(AppState *state);
/** @brief Elimina un hijo del padre activo del TAD Sublista. */
void app_state_operacion_sublista_eliminar_hijo(AppState *state);
/** @brief Ejecuta vaciado/destruccion logica segun estructura activa. */
void app_state_operacion_vaciar(AppState *state);

#endif
