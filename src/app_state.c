#include "app_state.h"

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

/** @brief Dispara un feedback visual corto para la operacion indicada. */
static void trigger_feedback(AppState *state, TipoOperacion operacion) {
    state->operacion_animada = operacion;
    state->animacion_feedback = 1.0f;
    state->animacion_pulso_panel = 1.0f;
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
    default:
        break;
    }
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
    default:
        break;
    }

    trigger_feedback(state, OPERACION_VACIAR);
}
