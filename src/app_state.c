#include "app_state.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * @file app_state.c
 * @brief Implementacion del estado global y despacho de operaciones de alto nivel.
 */

/** @brief Duracion base de la animacion visual posterior a una operacion. */
#define APP_FEEDBACK_DURATION 0.55f
/** @brief Duracion del fundido al cambiar de estructura visualizada. */
#define APP_SWITCH_DURATION 0.42f
/** @brief Duracion del pulso de panel al ejecutar operaciones validas. */
#define APP_PANEL_PULSE_DURATION 0.52f

/** @brief Limita un entero a un rango cerrado. */
static int clamp_int(int value, int min, int max) {
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

/** @brief Actualiza el mensaje principal mostrado en la UI. */
static void set_message(AppState *state, const char *text) {
    snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion), "%s", text);
}

static void trigger_feedback(AppState *state, TipoOperacion operacion);

static bool app_state_recrear_grafo(AppState *state, bool dirigido) {
    int *vertices = NULL;
    size_t n_vertices = 0;
    GrafoArista *aristas = NULL;
    size_t n_aristas = 0;
    Grafo *nuevo_grafo;
    int i;

    if (state == NULL || state->grafo == NULL) {
        return false;
    }

    if (grafo_obtener_vertices(state->grafo, &vertices, &n_vertices) != GRAFO_OK) {
        vertices = NULL;
        n_vertices = 0;
    }
    if (grafo_obtener_aristas(state->grafo, &aristas, &n_aristas) != GRAFO_OK) {
        aristas = NULL;
        n_aristas = 0;
    }

    nuevo_grafo = grafo_crear(dirigido);
    if (nuevo_grafo == NULL) {
        free(vertices);
        free(aristas);
        return false;
    }

    for (i = 0; i < (int)n_vertices; i++) {
        grafo_insertar_vertice(nuevo_grafo, vertices[i]);
    }
    for (i = 0; i < (int)n_aristas; i++) {
        grafo_insertar_arista(nuevo_grafo, aristas[i].origen, aristas[i].destino, aristas[i].peso);
    }

    grafo_controller_destruir(&state->grafo_controller_state);
    grafo_destruir(&state->grafo);
    state->grafo = nuevo_grafo;
    state->grafo_controller_state =
        grafo_controller_crear(state->grafo, (Rectangle){0.0f, 0.0f, 0.0f, 0.0f});
    state->grafo_dirigido = dirigido;

    free(vertices);
    free(aristas);
    return true;
}

static void app_state_grafo_insertar_demo(AppState *state, const int *vertices, int vertices_count,
                                          const GrafoArista *aristas, int aristas_count,
                                          const char *mensaje) {
    int i;

    if (state == NULL) {
        return;
    }

    app_state_operacion_inicializar(state);
    for (i = 0; i < vertices_count; i++) {
        grafo_controller_agregar_vertice(&state->grafo_controller_state, vertices[i]);
    }
    for (i = 0; i < aristas_count; i++) {
        grafo_controller_agregar_arista(&state->grafo_controller_state, aristas[i].origen,
                                        aristas[i].destino, aristas[i].peso);
    }
    state->ultima_operacion_ok = true;
    state->operacion_actual = OPERACION_INICIALIZAR;
    state->operacion_serial++;
    set_message(state, mensaje);
    trigger_feedback(state, OPERACION_INICIALIZAR);
}

/** @brief Dispara un feedback visual corto para la operacion indicada. */
static void trigger_feedback(AppState *state, TipoOperacion operacion) {
    state->operacion_animada = operacion;
    state->animacion_feedback = 1.0f;
    state->animacion_pulso_panel = 1.0f;
}

/** @brief Sincroniza el TAD de grafo con el estado visual del controlador. */
static void app_state_sync_grafo_visual(AppState *state) {
    int *vertices = NULL;
    size_t n_vertices = 0;
    GrafoArista *aristas = NULL;
    size_t n_aristas = 0;
    GrafoVerticeVisual old_vertices[64];
    GrafoAristaVisual old_aristas[256];
    int old_count_vertices;
    int old_count_aristas;
    int i;

    if (state == NULL || state->grafo == NULL) {
        return;
    }

    old_count_vertices = state->grafo_controller_state.estado_visual.cantidad_vertices;
    old_count_aristas = state->grafo_controller_state.estado_visual.cantidad_aristas;
    memcpy(old_vertices, state->grafo_controller_state.estado_visual.vertices, sizeof(old_vertices));
    memcpy(old_aristas, state->grafo_controller_state.estado_visual.aristas, sizeof(old_aristas));

    state->grafo_controller_state.estado_visual.cantidad_vertices = 0;
    state->grafo_controller_state.estado_visual.cantidad_aristas = 0;
    state->grafo_controller_state.estado_visual.es_dirigido = grafo_es_dirigido(state->grafo);

    if (grafo_obtener_vertices(state->grafo, &vertices, &n_vertices) == GRAFO_OK &&
        vertices != NULL) {
        int limite = (n_vertices > 64) ? 64 : (int)n_vertices;
        for (i = 0; i < limite; i++) {
            GrafoVerticeVisual *v = &state->grafo_controller_state.estado_visual.vertices[i];
            int j;

            v->id = vertices[i];
            v->estado = GRAFO_VÉRTICE_NORMAL;
            v->visible = true;
            v->distancia = 0;
            v->predecesor = -1;
            v->orden_visitacion = 0;
            v->radio = 15.0f;

            for (j = 0; j < old_count_vertices; j++) {
                if (old_vertices[j].id == v->id) {
                    v->estado = old_vertices[j].estado;
                    v->distancia = old_vertices[j].distancia;
                    v->predecesor = old_vertices[j].predecesor;
                    v->orden_visitacion = old_vertices[j].orden_visitacion;
                    break;
                }
            }
        }
        state->grafo_controller_state.estado_visual.cantidad_vertices = limite;
        free(vertices);
    }

    if (grafo_obtener_aristas(state->grafo, &aristas, &n_aristas) == GRAFO_OK && aristas != NULL) {
        int limite = (n_aristas > 256) ? 256 : (int)n_aristas;
        for (i = 0; i < limite; i++) {
            GrafoAristaVisual *a = &state->grafo_controller_state.estado_visual.aristas[i];
            int j;

            a->origen = aristas[i].origen;
            a->destino = aristas[i].destino;
            a->peso = aristas[i].peso;
            a->estado = GRAFO_ARISTA_NORMAL;
            a->visible = true;
            a->es_dirigida = grafo_es_dirigido(state->grafo);
            a->orden_examinacion = 0;

            for (j = 0; j < old_count_aristas; j++) {
                if ((old_aristas[j].origen == a->origen && old_aristas[j].destino == a->destino) ||
                    (!a->es_dirigida && old_aristas[j].origen == a->destino &&
                     old_aristas[j].destino == a->origen)) {
                    a->estado = old_aristas[j].estado;
                    a->orden_examinacion = old_aristas[j].orden_examinacion;
                    break;
                }
            }
        }
        state->grafo_controller_state.estado_visual.cantidad_aristas = limite;
        free(aristas);
    }

    if (state->grafo_controller_state.vista.layout_config.ancho_panel > 0 &&
        state->grafo_controller_state.vista.layout_config.alto_panel > 0) {
        grafo_layout_calcular_circular(&state->grafo_controller_state.estado_visual,
                                       &state->grafo_controller_state.vista.layout_config);
    }
}

/** @brief Inicializa la aplicacion y todos los TAD gestionados. */
void app_state_init(AppState *state) {
    if (state == NULL) {
        return;
    }

    memset(state, 0, sizeof(*state));
    state->estructura_activa = ESTRUCTURA_PILA;
    state->operacion_actual = OPERACION_INICIALIZAR;
    state->operacion_animada = OPERACION_NINGUNA;
    state->input_valor = 10;
    state->input_prioridad = 1;
    state->animacion_feedback = 0.0f;
    state->animacion_cambio_estructura = 0.0f;
    state->animacion_pulso_panel = 0.0f;
    state->ultima_operacion_ok = true;
    state->operacion_serial = 0;
    state->coincidencias_busqueda = 0;
    state->sublista_padre_activo = 0;
    state->sublista_padre_activo_ok = false;
    snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
             "Aplicacion inicializada");

    pila_inicializar(&state->pila);
    cola_inicializar(&state->cola);
    cp_inicializar(&state->cola_prioridad);
    lista_inicializar(&state->lista);
    lcir_inicializar(&state->lista_circular);
    sublista_inicializar(&state->sublista);
    state->grafo = grafo_crear(false);
    state->grafo_controller_state =
        grafo_controller_crear(state->grafo, (Rectangle){0.0f, 0.0f, 0.0f, 0.0f});
    state->grafo_vertice_inicio = -1;
    state->grafo_vertice_destino = -1;
    state->grafo_algoritmo_seleccionado = GRAFO_ALGO_NINGUNO;
    state->grafo_dirigido = false;
    state->grafo_demo_idx = 0;
}

/** @brief Libera la memoria de todas las estructuras al cerrar la app. */
void app_state_shutdown(AppState *state) {
    if (state == NULL) {
        return;
    }

    pila_destruir(&state->pila);
    cola_vaciar(&state->cola);
    cp_vaciar(&state->cola_prioridad);
    lista_destruir(&state->lista);
    lcir_destruir(&state->lista_circular);
    sublista_destruir(&state->sublista);
    grafo_controller_destruir(&state->grafo_controller_state);
    grafo_destruir(&state->grafo);
}

/** @brief Cambia la estructura activa y reinicia el contexto operativo. */
void app_state_set_estructura(AppState *state, TipoEstructura estructura) {
    if (state == NULL) {
        return;
    }

    if (state->estructura_activa != estructura) {
        state->animacion_cambio_estructura = 1.0f;
    }
    state->estructura_activa = estructura;
    state->operacion_actual = OPERACION_NINGUNA;
    state->operacion_animada = OPERACION_NINGUNA;
    state->animacion_feedback = 0.0f;
    set_message(state, "Estructura seleccionada");
}

/** @brief Ajusta incrementalmente el valor de entrada. */
void app_state_ajustar_valor(AppState *state, int delta) {
    if (state == NULL) {
        return;
    }

    state->input_valor += delta;
}

/** @brief Asigna el valor de entrada principal. */
void app_state_set_valor(AppState *state, int value) {
    if (state == NULL) {
        return;
    }

    state->input_valor = value;
}

/** @brief Ajusta incrementalmente la prioridad dentro del rango permitido. */
void app_state_ajustar_prioridad(AppState *state, int delta) {
    if (state == NULL) {
        return;
    }

    state->input_prioridad = clamp_int(state->input_prioridad + delta, 1, 99);
}

/** @brief Asigna la prioridad de entrada aplicando validacion de rango. */
void app_state_set_prioridad(AppState *state, int value) {
    if (state == NULL) {
        return;
    }

    state->input_prioridad = clamp_int(value, 1, 99);
}

/** @brief Avanza el estado de animaciones efimeras del feedback visual. */
void app_state_update_visuals(AppState *state, float delta_time) {
    if (state == NULL) {
        return;
    }

    if (state->animacion_feedback > 0.0f) {
        state->animacion_feedback -= delta_time / APP_FEEDBACK_DURATION;
        if (state->animacion_feedback <= 0.0f) {
            state->animacion_feedback = 0.0f;
            state->operacion_animada = OPERACION_NINGUNA;
        }
    }

    if (state->animacion_cambio_estructura > 0.0f) {
        state->animacion_cambio_estructura -= delta_time / APP_SWITCH_DURATION;
        if (state->animacion_cambio_estructura < 0.0f) {
            state->animacion_cambio_estructura = 0.0f;
        }
    }

    if (state->animacion_pulso_panel > 0.0f) {
        state->animacion_pulso_panel -= delta_time / APP_PANEL_PULSE_DURATION;
        if (state->animacion_pulso_panel < 0.0f) {
            state->animacion_pulso_panel = 0.0f;
        }
    }

    app_state_sync_grafo_visual(state);
}

/** @brief Reinicializa la estructura activa sin cambiar de modulo. */
void app_state_operacion_inicializar(AppState *state) {
    if (state == NULL) {
        return;
    }

    state->operacion_actual = OPERACION_INICIALIZAR;
    state->operacion_serial++;
    state->ultima_operacion_ok = true;

    switch (state->estructura_activa) {
    case ESTRUCTURA_PILA:
        pila_destruir(&state->pila);
        pila_inicializar(&state->pila);
        set_message(state, "Pila inicializada");
        break;
    case ESTRUCTURA_COLA:
        cola_vaciar(&state->cola);
        cola_inicializar(&state->cola);
        set_message(state, "Cola inicializada");
        break;
    case ESTRUCTURA_COLA_PRIORIDAD:
        cp_vaciar(&state->cola_prioridad);
        cp_inicializar(&state->cola_prioridad);
        set_message(state, "Cola de prioridad inicializada");
        break;
    case ESTRUCTURA_LISTA:
        lista_destruir(&state->lista);
        lista_inicializar(&state->lista);
        set_message(state, "Lista inicializada");
        break;
    case ESTRUCTURA_LISTA_CIRCULAR:
        lcir_destruir(&state->lista_circular);
        lcir_inicializar(&state->lista_circular);
        set_message(state, "Lista circular inicializada");
        break;
    case ESTRUCTURA_SUBLISTA:
        sublista_destruir(&state->sublista);
        sublista_inicializar(&state->sublista);
        state->sublista_padre_activo = 0;
        state->sublista_padre_activo_ok = false;
        set_message(state, "Sublista inicializada");
        break;
    case ESTRUCTURA_GRAFO:
        grafo_controller_destruir(&state->grafo_controller_state);
        grafo_destruir(&state->grafo);
        state->grafo = grafo_crear(false);
        state->grafo_controller_state =
            grafo_controller_crear(state->grafo, (Rectangle){0.0f, 0.0f, 0.0f, 0.0f});
        state->grafo_vertice_inicio = -1;
        state->grafo_vertice_destino = -1;
        state->grafo_algoritmo_seleccionado = GRAFO_ALGO_NINGUNO;
        state->grafo_dirigido = false;
        set_message(state, "Grafo inicializado");
        break;
    default:
        break;
    }

    trigger_feedback(state, OPERACION_INICIALIZAR);
}

/** @brief Ejecuta la insercion base de la estructura activa. */
void app_state_operacion_insertar(AppState *state) {
    if (state == NULL) {
        return;
    }

    state->operacion_actual = OPERACION_INSERTAR;
    state->operacion_serial++;

    switch (state->estructura_activa) {
    case ESTRUCTURA_PILA:
        state->ultima_operacion_ok = pila_push(&state->pila, state->input_valor);
        if (state->ultima_operacion_ok) {
            state->ultimo_valor = state->input_valor;
            trigger_feedback(state, OPERACION_INSERTAR);
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "push(%d) ejecutado", state->input_valor);
        } else {
            set_message(state, "push fallido");
        }
        break;
    case ESTRUCTURA_COLA:
        state->ultima_operacion_ok = cola_encolar(&state->cola, state->input_valor);
        if (state->ultima_operacion_ok) {
            state->ultimo_valor = state->input_valor;
            trigger_feedback(state, OPERACION_INSERTAR);
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "encolar(%d) ejecutado", state->input_valor);
        } else {
            set_message(state, "encolar fallido");
        }
        break;
    case ESTRUCTURA_COLA_PRIORIDAD:
        state->ultima_operacion_ok = cp_encolar(&state->cola_prioridad, state->input_valor,
                                                state->input_prioridad);
        if (state->ultima_operacion_ok) {
            state->ultimo_valor = state->input_valor;
            state->ultima_prioridad = state->input_prioridad;
            trigger_feedback(state, OPERACION_INSERTAR);
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "cp_encolar(valor=%d, prioridad=%d)", state->input_valor,
                     state->input_prioridad);
        } else {
            set_message(state, "cp_encolar fallido");
        }
        break;
    case ESTRUCTURA_LISTA:
        state->operacion_actual = OPERACION_INSERTAR_FINAL;
        state->ultima_operacion_ok = lista_insertar_final(&state->lista, state->input_valor);
        if (state->ultima_operacion_ok) {
            state->ultimo_valor = state->input_valor;
            trigger_feedback(state, OPERACION_INSERTAR_FINAL);
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "lista_insertar_final(%d)", state->input_valor);
        } else {
            set_message(state, "lista_insertar_final fallido");
        }
        break;
    case ESTRUCTURA_LISTA_CIRCULAR:
        state->operacion_actual = OPERACION_INSERTAR_FINAL;
        state->ultima_operacion_ok = lcir_insertar_final(&state->lista_circular, state->input_valor);
        if (state->ultima_operacion_ok) {
            state->ultimo_valor = state->input_valor;
            trigger_feedback(state, OPERACION_INSERTAR_FINAL);
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "lcir_insertar_final(%d)", state->input_valor);
        } else {
            set_message(state, "lcir_insertar_final fallido");
        }
        break;
    case ESTRUCTURA_SUBLISTA: {
        Nodo *padre = sublista_insertar_padre_final(&state->sublista, state->input_valor);
        state->ultima_operacion_ok = (padre != NULL);
        if (state->ultima_operacion_ok) {
            state->ultimo_valor = state->input_valor;
            state->sublista_padre_activo = state->input_valor;
            state->sublista_padre_activo_ok = true;
            trigger_feedback(state, OPERACION_INSERTAR);
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "sublista_insertar_padre_final(%d)", state->input_valor);
        } else {
            set_message(state, "insertar padre fallido");
        }
        break;
    }
    case ESTRUCTURA_GRAFO: {
        bool ok = grafo_controller_agregar_vertice(&state->grafo_controller_state,
                                                   state->input_valor);
        state->ultima_operacion_ok = ok;
        if (state->ultima_operacion_ok) {
            state->ultimo_valor = state->input_valor;
            trigger_feedback(state, OPERACION_INSERTAR);
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "grafo_insertar_vertice(%d)", state->input_valor);
        } else {
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "Error grafo: %s",
                     grafo_controller_obtener_error(&state->grafo_controller_state));
        }
        break;
    }
    default:
        break;
    }
}

/** @brief Inserta un nodo al inicio cuando la estructura activa es lista o lista circular. */
void app_state_operacion_lista_insertar_inicio(AppState *state) {
    if (state == NULL) {
        return;
    }
    state->operacion_serial++;
    if (state->estructura_activa != ESTRUCTURA_LISTA &&
        state->estructura_activa != ESTRUCTURA_LISTA_CIRCULAR) {
        state->ultima_operacion_ok = false;
        set_message(state, "Operacion valida solo para Lista/Lista Circular");
        return;
    }

    state->operacion_actual = OPERACION_INSERTAR_INICIO;
    if (state->estructura_activa == ESTRUCTURA_LISTA_CIRCULAR) {
        state->ultima_operacion_ok =
            lcir_insertar_inicio(&state->lista_circular, state->input_valor);
    } else {
        state->ultima_operacion_ok = lista_insertar_inicio(&state->lista, state->input_valor);
    }
    if (state->ultima_operacion_ok) {
        state->ultimo_valor = state->input_valor;
        trigger_feedback(state, OPERACION_INSERTAR_INICIO);
        if (state->estructura_activa == ESTRUCTURA_LISTA_CIRCULAR) {
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "lcir_insertar_inicio(%d)", state->input_valor);
        } else {
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "lista_insertar_inicio(%d)", state->input_valor);
        }
    } else {
        set_message(state, "insertar_inicio fallido");
    }
}

/** @brief Inserta un nodo al final cuando la estructura activa es lista o lista circular. */
void app_state_operacion_lista_insertar_final(AppState *state) {
    if (state == NULL) {
        return;
    }
    state->operacion_serial++;
    if (state->estructura_activa != ESTRUCTURA_LISTA &&
        state->estructura_activa != ESTRUCTURA_LISTA_CIRCULAR) {
        state->ultima_operacion_ok = false;
        set_message(state, "Operacion valida solo para Lista/Lista Circular");
        return;
    }

    state->operacion_actual = OPERACION_INSERTAR_FINAL;
    if (state->estructura_activa == ESTRUCTURA_LISTA_CIRCULAR) {
        state->ultima_operacion_ok =
            lcir_insertar_final(&state->lista_circular, state->input_valor);
    } else {
        state->ultima_operacion_ok = lista_insertar_final(&state->lista, state->input_valor);
    }
    if (state->ultima_operacion_ok) {
        state->ultimo_valor = state->input_valor;
        trigger_feedback(state, OPERACION_INSERTAR_FINAL);
        if (state->estructura_activa == ESTRUCTURA_LISTA_CIRCULAR) {
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "lcir_insertar_final(%d)", state->input_valor);
        } else {
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "lista_insertar_final(%d)", state->input_valor);
        }
    } else {
        set_message(state, "insertar_final fallido");
    }
}

/** @brief Ejecuta la operacion de eliminacion correspondiente a la estructura activa. */
void app_state_operacion_eliminar(AppState *state) {
    int valor = 0;
    int prioridad = 0;

    if (state == NULL) {
        return;
    }

    state->operacion_actual = OPERACION_ELIMINAR;
    state->operacion_serial++;

    switch (state->estructura_activa) {
    case ESTRUCTURA_PILA:
        state->ultima_operacion_ok = pila_pop(&state->pila, &valor);
        if (state->ultima_operacion_ok) {
            state->ultimo_valor = valor;
            trigger_feedback(state, OPERACION_ELIMINAR);
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "pop() -> %d", valor);
        } else {
            set_message(state, "pop no disponible: pila vacia");
        }
        break;
    case ESTRUCTURA_COLA:
        state->ultima_operacion_ok = cola_desencolar(&state->cola, &valor);
        if (state->ultima_operacion_ok) {
            state->ultimo_valor = valor;
            trigger_feedback(state, OPERACION_ELIMINAR);
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "desencolar() -> %d", valor);
        } else {
            set_message(state, "desencolar no disponible: cola vacia");
        }
        break;
    case ESTRUCTURA_COLA_PRIORIDAD:
        state->ultima_operacion_ok = cp_desencolar(&state->cola_prioridad, &valor, &prioridad);
        if (state->ultima_operacion_ok) {
            state->ultimo_valor = valor;
            state->ultima_prioridad = prioridad;
            trigger_feedback(state, OPERACION_ELIMINAR);
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "cp_desencolar() -> (%d, p=%d)", valor, prioridad);
        } else {
            set_message(state, "cp_desencolar no disponible: cola vacia");
        }
        break;
    case ESTRUCTURA_LISTA:
        state->ultima_operacion_ok = lista_eliminar_primero(&state->lista, state->input_valor);
        if (state->ultima_operacion_ok) {
            state->ultimo_valor = state->input_valor;
            trigger_feedback(state, OPERACION_ELIMINAR);
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "lista_eliminar_primero(%d)", state->input_valor);
        } else {
            set_message(state, "valor no encontrado en lista");
        }
        break;
    case ESTRUCTURA_LISTA_CIRCULAR:
        state->ultima_operacion_ok =
            lcir_eliminar_primero(&state->lista_circular, state->input_valor);
        if (state->ultima_operacion_ok) {
            state->ultimo_valor = state->input_valor;
            trigger_feedback(state, OPERACION_ELIMINAR);
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "lcir_eliminar_primero(%d)", state->input_valor);
        } else {
            set_message(state, "valor no encontrado en lista circular");
        }
        break;
    case ESTRUCTURA_SUBLISTA:
        state->ultima_operacion_ok =
            sublista_eliminar_padre_primero(&state->sublista, state->input_valor);
        if (state->ultima_operacion_ok) {
            state->ultimo_valor = state->input_valor;
            if (state->sublista_padre_activo_ok &&
                state->sublista_padre_activo == state->input_valor) {
                state->sublista_padre_activo_ok = false;
            }
            trigger_feedback(state, OPERACION_ELIMINAR);
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "sublista_eliminar_padre_primero(%d)", state->input_valor);
        } else {
            set_message(state, "padre no encontrado en sublista");
        }
        break;
    case ESTRUCTURA_GRAFO: {
        state->ultima_operacion_ok = grafo_controller_eliminar_vertice(
            &state->grafo_controller_state, state->input_valor);
        if (state->ultima_operacion_ok) {
            state->ultimo_valor = state->input_valor;
            trigger_feedback(state, OPERACION_ELIMINAR);
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "grafo_eliminar_vertice(%d)", state->input_valor);
        } else {
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "Error grafo: %s",
                     grafo_controller_obtener_error(&state->grafo_controller_state));
        }
        break;
    }
    default:
        break;
    }
}

void app_state_operacion_grafo_insertar_arista(AppState *state, int origen, int destino,
                                               int peso) {
    if (state == NULL || state->estructura_activa != ESTRUCTURA_GRAFO) {
        return;
    }

    if (origen < 0 || destino < 0) {
        state->ultima_operacion_ok = false;
        set_message(state, "Error grafo: origen y destino deben ser enteros no negativos");
        return;
    }
    if (origen == destino) {
        state->ultima_operacion_ok = false;
        set_message(state, "Error grafo: una arista requiere vertices distintos");
        return;
    }
    if (peso < -999 || peso > 999) {
        state->ultima_operacion_ok = false;
        set_message(state, "Error grafo: el peso debe estar entre -999 y 999");
        return;
    }

    state->operacion_actual = OPERACION_INSERTAR;
    state->operacion_serial++;
    state->ultima_operacion_ok = grafo_controller_agregar_arista(
        &state->grafo_controller_state, origen, destino, peso);

    if (state->ultima_operacion_ok) {
        trigger_feedback(state, OPERACION_INSERTAR);
        snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                 "grafo_insertar_arista(%d,%d,%d)", origen, destino, peso);
    } else {
        snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion), "Error grafo: %s",
                 grafo_controller_obtener_error(&state->grafo_controller_state));
    }
}

void app_state_operacion_grafo_eliminar_arista(AppState *state, int origen, int destino) {
        if (origen < 0 || destino < 0) {
            state->ultima_operacion_ok = false;
            set_message(state, "Error grafo: origen y destino invalidos para eliminar arista");
            return;
        }

    if (state == NULL || state->estructura_activa != ESTRUCTURA_GRAFO) {
        return;
    }

    state->operacion_actual = OPERACION_ELIMINAR;
    state->operacion_serial++;
    state->ultima_operacion_ok = grafo_controller_eliminar_arista(
        &state->grafo_controller_state, origen, destino);

    if (state->ultima_operacion_ok) {
        trigger_feedback(state, OPERACION_ELIMINAR);
        snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                 "grafo_eliminar_arista(%d,%d)", origen, destino);
    } else {
        snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion), "Error grafo: %s",
                 grafo_controller_obtener_error(&state->grafo_controller_state));
    }
}

void app_state_operacion_grafo_ejecutar_algoritmo(AppState *state, int algoritmo, int inicio,
                                                  int destino) {
    GrafoRecorrido recorrido;
    GrafoCamino camino;

    if (state == NULL || state->estructura_activa != ESTRUCTURA_GRAFO) {
        return;
    }

    if (grafo_orden(state->grafo) == 0) {
        state->ultima_operacion_ok = false;
        set_message(state, "Error algoritmo: el grafo esta vacio");
        return;
    }
    if ((algoritmo == GRAFO_ALGO_BFS || algoritmo == GRAFO_ALGO_DFS || algoritmo == GRAFO_ALGO_DIJKSTRA ||
         algoritmo == GRAFO_ALGO_BELLMAN_FORD || algoritmo == GRAFO_ALGO_PRIM) && inicio < 0) {
        state->ultima_operacion_ok = false;
        set_message(state, "Error algoritmo: define un vertice de inicio valido");
        return;
    }
    if ((algoritmo == GRAFO_ALGO_DIJKSTRA || algoritmo == GRAFO_ALGO_BELLMAN_FORD) && destino < 0) {
        state->ultima_operacion_ok = false;
        set_message(state, "Error algoritmo: define un vertice destino valido");
        return;
    }

    state->grafo_algoritmo_seleccionado = algoritmo;
    state->grafo_vertice_inicio = inicio;
    state->grafo_vertice_destino = destino;
    state->operacion_actual = OPERACION_BUSCAR;
    state->operacion_serial++;

    grafo_controller_seleccionar_algoritmo(&state->grafo_controller_state, algoritmo, inicio,
                                           destino);
    grafo_controller_iniciar_algoritmo(&state->grafo_controller_state);

    state->ultima_operacion_ok = true;
    switch (algoritmo) {
    case GRAFO_ALGO_BFS:
        recorrido = grafo_bfs(state->grafo, inicio);
        state->ultima_operacion_ok = (recorrido.estado == GRAFO_OK);
        snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                 "BFS(inicio=%d) -> %d vertice(s)", inicio, (int)recorrido.cantidad);
        grafo_liberar_recorrido(&recorrido);
        break;
    case GRAFO_ALGO_DFS:
        recorrido = grafo_dfs(state->grafo, inicio);
        state->ultima_operacion_ok = (recorrido.estado == GRAFO_OK);
        snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                 "DFS(inicio=%d) -> %d vertice(s)", inicio, (int)recorrido.cantidad);
        grafo_liberar_recorrido(&recorrido);
        break;
    case GRAFO_ALGO_DIJKSTRA:
        camino = grafo_dijkstra(state->grafo, inicio, destino);
        state->ultima_operacion_ok = (camino.estado == GRAFO_OK);
        if (camino.estado == GRAFO_ERROR_PESO_NEGATIVO) {
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "Dijkstra no admite pesos negativos");
        } else if (camino.existe) {
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "Dijkstra(%d->%d) costo=%d", inicio, destino, camino.costo_total);
        } else {
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "Dijkstra(%d->%d) sin camino", inicio, destino);
        }
        grafo_liberar_camino(&camino);
        break;
    case GRAFO_ALGO_BELLMAN_FORD:
        camino = grafo_bellman_ford(state->grafo, inicio, destino);
        state->ultima_operacion_ok = (camino.estado == GRAFO_OK);
        if (camino.estado == GRAFO_ERROR_CICLO_NEGATIVO) {
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "Bellman-Ford detecto ciclo negativo");
        } else if (camino.existe) {
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "Bellman-Ford(%d->%d) costo=%d", inicio, destino, camino.costo_total);
        } else {
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "Bellman-Ford(%d->%d) sin camino", inicio, destino);
        }
        grafo_liberar_camino(&camino);
        break;
    case GRAFO_ALGO_PRIM:
        camino = grafo_prim(state->grafo, inicio);
        state->ultima_operacion_ok = (camino.estado == GRAFO_OK);
        snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion), "Prim(inicio=%d)",
                 inicio);
        grafo_liberar_camino(&camino);
        break;
    case GRAFO_ALGO_KRUSKAL:
        camino = grafo_kruskal(state->grafo);
        state->ultima_operacion_ok = (camino.estado == GRAFO_OK);
        snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion), "Kruskal() aristas=%d",
                 (int)camino.cantidad);
        grafo_liberar_camino(&camino);
        break;
    default:
        state->ultima_operacion_ok = false;
        set_message(state, "Algoritmo no soportado");
        break;
    }

    if (!state->ultima_operacion_ok && strncmp(state->mensaje_operacion, "Bellman-Ford detecto ciclo negativo", 36) != 0 &&
        strncmp(state->mensaje_operacion, "Dijkstra no admite pesos negativos", 34) != 0) {
        snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion), "Error algoritmo");
    } else {
        trigger_feedback(state, OPERACION_BUSCAR);
    }
}

void app_state_grafo_toggle_dirigido(AppState *state) {
    if (state == NULL || state->estructura_activa != ESTRUCTURA_GRAFO) {
        return;
    }

    state->operacion_serial++;
    state->operacion_actual = OPERACION_INICIALIZAR;
    state->ultima_operacion_ok = app_state_recrear_grafo(state, !state->grafo_dirigido);
    if (state->ultima_operacion_ok) {
        snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                 "Modo de grafo: %s", state->grafo_dirigido ? "Dirigido" : "No dirigido");
        trigger_feedback(state, OPERACION_INICIALIZAR);
    } else {
        set_message(state, "Error grafo: no se pudo cambiar el modo dirigido");
    }
}

void app_state_grafo_cargar_demo(AppState *state) {
    static const int demo_bfs_vertices[] = {1, 2, 3, 4, 5};
    static const GrafoArista demo_bfs_aristas[] = {
        {1, 2, 1}, {1, 3, 1}, {2, 4, 1}, {3, 5, 1}
    };
    static const int demo_camino_vertices[] = {1, 2, 3, 4, 5, 6};
    static const GrafoArista demo_camino_aristas[] = {
        {1, 2, 4}, {1, 3, 2}, {3, 2, 1}, {2, 4, 5}, {3, 5, 8}, {5, 4, 1}, {4, 6, 3}
    };
    static const int demo_bf_vertices[] = {1, 2, 3, 4, 5};
    static const GrafoArista demo_bf_aristas[] = {
        {1, 2, 6}, {1, 3, 7}, {2, 4, 5}, {2, 5, -2}, {3, 4, -3}, {4, 2, -2}, {5, 4, 7}
    };
    static const int demo_mst_vertices[] = {1, 2, 3, 4, 5, 6};
    static const GrafoArista demo_mst_aristas[] = {
        {1, 2, 3}, {1, 3, 1}, {2, 3, 7}, {2, 4, 5}, {3, 4, 2}, {3, 5, 4}, {4, 6, 6}, {5, 6, 2}
    };

    if (state == NULL || state->estructura_activa != ESTRUCTURA_GRAFO) {
        return;
    }

    if (state->grafo_demo_idx == 0) {
        app_state_recrear_grafo(state, false);
        app_state_grafo_insertar_demo(state, demo_bfs_vertices, 5, demo_bfs_aristas, 4,
                                      "Demo BFS cargada");
        state->grafo_vertice_inicio = 1;
        state->grafo_vertice_destino = 5;
    } else if (state->grafo_demo_idx == 1) {
        app_state_recrear_grafo(state, true);
        app_state_grafo_insertar_demo(state, demo_camino_vertices, 6, demo_camino_aristas, 7,
                                      "Demo de caminos cargada");
        state->grafo_vertice_inicio = 1;
        state->grafo_vertice_destino = 6;
    } else if (state->grafo_demo_idx == 2) {
        app_state_recrear_grafo(state, true);
        app_state_grafo_insertar_demo(state, demo_bf_vertices, 5, demo_bf_aristas, 7,
                                      "Demo Bellman-Ford cargada");
        state->grafo_vertice_inicio = 1;
        state->grafo_vertice_destino = 4;
    } else {
        app_state_recrear_grafo(state, false);
        app_state_grafo_insertar_demo(state, demo_mst_vertices, 6, demo_mst_aristas, 8,
                                      "Demo MST cargada");
        state->grafo_vertice_inicio = 1;
        state->grafo_vertice_destino = 6;
    }
    state->grafo_demo_idx = (state->grafo_demo_idx + 1) % 4;
    state->grafo_algoritmo_seleccionado = GRAFO_ALGO_NINGUNO;
}

/** @brief Busca coincidencias en lista o lista circular. */
void app_state_operacion_buscar(AppState *state) {
    if (state == NULL) {
        return;
    }

    state->operacion_actual = OPERACION_BUSCAR;
    state->operacion_serial++;
    state->coincidencias_busqueda = 0;
    state->ultima_operacion_ok = false;

    if (state->estructura_activa == ESTRUCTURA_SUBLISTA) {
        Nodo *padre = sublista_buscar_padre(state->sublista, state->input_valor);
        state->coincidencias_busqueda = (padre != NULL) ? 1 : 0;
        state->ultima_operacion_ok = true;
        if (padre != NULL) {
            state->sublista_padre_activo = state->input_valor;
            state->sublista_padre_activo_ok = true;
            trigger_feedback(state, OPERACION_BUSCAR);
            snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                     "padre activo = %d", state->sublista_padre_activo);
        } else {
            trigger_feedback(state, OPERACION_BUSCAR);
            set_message(state, "padre no encontrado");
        }
        return;
    }

    if (state->estructura_activa == ESTRUCTURA_GRAFO) {
        state->coincidencias_busqueda =
            grafo_existe_vertice(state->grafo, state->input_valor) ? 1 : 0;
        state->ultima_operacion_ok = true;
        trigger_feedback(state, OPERACION_BUSCAR);
        snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                 "buscar_vertice(%d) -> %d", state->input_valor,
                 state->coincidencias_busqueda);
        return;
    }

    if (state->estructura_activa != ESTRUCTURA_LISTA &&
        state->estructura_activa != ESTRUCTURA_LISTA_CIRCULAR) {
        set_message(state, "Buscar disponible solo en Lista/Lista Circular");
        return;
    }

    if (state->estructura_activa == ESTRUCTURA_LISTA_CIRCULAR) {
        state->coincidencias_busqueda =
            lcir_buscar_posiciones(&state->lista_circular, state->input_valor, NULL, 0);
    } else {
        state->coincidencias_busqueda =
            lista_buscar_posiciones(&state->lista, state->input_valor, NULL, 0);
    }
    state->ultima_operacion_ok = true;
    trigger_feedback(state, OPERACION_BUSCAR);
    snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
             "buscar(%d) -> %d coincidencia(s)", state->input_valor,
             state->coincidencias_busqueda);
}

/** @brief Invierte lista o lista circular cuando corresponde. */
void app_state_operacion_invertir(AppState *state) {
    if (state == NULL) {
        return;
    }

    state->operacion_actual = OPERACION_INVERTIR;
    state->operacion_serial++;

    if (state->estructura_activa != ESTRUCTURA_LISTA &&
        state->estructura_activa != ESTRUCTURA_LISTA_CIRCULAR) {
        state->ultima_operacion_ok = false;
        set_message(state, "Invertir disponible solo en Lista/Lista Circular");
        return;
    }

    if (state->estructura_activa == ESTRUCTURA_LISTA_CIRCULAR) {
        lcir_invertir(&state->lista_circular);
    } else {
        lista_invertir(&state->lista);
    }
    state->ultima_operacion_ok = true;
    trigger_feedback(state, OPERACION_INVERTIR);
    if (state->estructura_activa == ESTRUCTURA_LISTA_CIRCULAR) {
        set_message(state, "lcir_invertir() ejecutado");
    } else {
        set_message(state, "lista_invertir() ejecutado");
    }
}

/** @brief Inserta un hijo sobre el padre activo en el TAD sublista. */
void app_state_operacion_sublista_insertar_hijo(AppState *state) {
    Nodo *padre;

    if (state == NULL) {
        return;
    }

    state->operacion_actual = OPERACION_SUBLISTA_INSERTAR_HIJO;
    state->operacion_serial++;

    if (state->estructura_activa != ESTRUCTURA_SUBLISTA) {
        state->ultima_operacion_ok = false;
        set_message(state, "Operacion valida solo para Sublistas");
        return;
    }
    if (!state->sublista_padre_activo_ok) {
        state->ultima_operacion_ok = false;
        set_message(state, "Seleccione padre con Buscar (B)");
        return;
    }

    padre = sublista_buscar_padre(state->sublista, state->sublista_padre_activo);
    if (padre == NULL) {
        state->sublista_padre_activo_ok = false;
        state->ultima_operacion_ok = false;
        set_message(state, "Padre activo invalido, seleccione nuevamente");
        return;
    }

    state->ultima_operacion_ok = sublista_insertar_hijo_final(padre, state->input_valor);
    if (state->ultima_operacion_ok) {
        state->ultimo_valor = state->input_valor;
        trigger_feedback(state, OPERACION_SUBLISTA_INSERTAR_HIJO);
        snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                 "insertar_hijo(padre=%d, valor=%d)", state->sublista_padre_activo,
                 state->input_valor);
    } else {
        set_message(state, "insertar_hijo fallido");
    }
}

/** @brief Elimina la primera ocurrencia de un hijo en el padre activo. */
void app_state_operacion_sublista_eliminar_hijo(AppState *state) {
    Nodo *padre;

    if (state == NULL) {
        return;
    }

    state->operacion_actual = OPERACION_SUBLISTA_ELIMINAR_HIJO;
    state->operacion_serial++;

    if (state->estructura_activa != ESTRUCTURA_SUBLISTA) {
        state->ultima_operacion_ok = false;
        set_message(state, "Operacion valida solo para Sublistas");
        return;
    }
    if (!state->sublista_padre_activo_ok) {
        state->ultima_operacion_ok = false;
        set_message(state, "Seleccione padre con Buscar (B)");
        return;
    }

    padre = sublista_buscar_padre(state->sublista, state->sublista_padre_activo);
    if (padre == NULL) {
        state->sublista_padre_activo_ok = false;
        state->ultima_operacion_ok = false;
        set_message(state, "Padre activo invalido, seleccione nuevamente");
        return;
    }

    state->ultima_operacion_ok = sublista_eliminar_hijo_primero(padre, state->input_valor);
    if (state->ultima_operacion_ok) {
        state->ultimo_valor = state->input_valor;
        trigger_feedback(state, OPERACION_SUBLISTA_ELIMINAR_HIJO);
        snprintf(state->mensaje_operacion, sizeof(state->mensaje_operacion),
                 "eliminar_hijo(padre=%d, valor=%d)", state->sublista_padre_activo,
                 state->input_valor);
    } else {
        set_message(state, "hijo no encontrado en padre activo");
    }
}

/** @brief Vacia la estructura activa liberando su memoria dinamica. */
void app_state_operacion_vaciar(AppState *state) {
    if (state == NULL) {
        return;
    }

    state->operacion_actual = OPERACION_VACIAR;
    state->operacion_serial++;
    state->ultima_operacion_ok = true;

    switch (state->estructura_activa) {
    case ESTRUCTURA_PILA:
        pila_destruir(&state->pila);
        set_message(state, "pila vaciada");
        break;
    case ESTRUCTURA_COLA:
        cola_vaciar(&state->cola);
        set_message(state, "cola vaciada");
        break;
    case ESTRUCTURA_COLA_PRIORIDAD:
        cp_vaciar(&state->cola_prioridad);
        set_message(state, "cola de prioridad vaciada");
        break;
    case ESTRUCTURA_LISTA:
        lista_destruir(&state->lista);
        set_message(state, "lista vaciada");
        break;
    case ESTRUCTURA_LISTA_CIRCULAR:
        lcir_destruir(&state->lista_circular);
        set_message(state, "lista circular vaciada");
        break;
    case ESTRUCTURA_SUBLISTA:
        sublista_destruir(&state->sublista);
        sublista_inicializar(&state->sublista);
        state->sublista_padre_activo = 0;
        state->sublista_padre_activo_ok = false;
        set_message(state, "sublista vaciada");
        break;
    case ESTRUCTURA_GRAFO:
        grafo_controller_destruir(&state->grafo_controller_state);
        grafo_destruir(&state->grafo);
        state->grafo = grafo_crear(false);
        state->grafo_controller_state =
            grafo_controller_crear(state->grafo, (Rectangle){0.0f, 0.0f, 0.0f, 0.0f});
        set_message(state, "grafo vaciado");
        break;
    default:
        break;
    }

    trigger_feedback(state, OPERACION_VACIAR);
}
