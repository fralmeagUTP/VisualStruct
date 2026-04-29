#include "code_viewer.h"

/**
 * @file code_viewer.c
 * @brief Catalogo de snippets C asociados a las operaciones visibles en la UI.
 */

/** @brief Selecciona el snippet C apropiado para la estructura y operacion actual. */
const char *code_viewer_get_snippet(TipoEstructura estructura, TipoOperacion operacion) {
    switch (estructura) {
    case ESTRUCTURA_PILA:
        if (operacion == OPERACION_INSERTAR) {
            return "Pila pila;\n"
                   "pila_inicializar(&pila);\n"
                   "bool ok = pila_push(&pila, valor);";
        }
        if (operacion == OPERACION_ELIMINAR) {
            return "int out = 0;\n"
                   "bool ok = pila_pop(&pila, &out);";
        }
        if (operacion == OPERACION_VACIAR) {
            return "pila_destruir(&pila);\n"
                   "pila_inicializar(&pila);";
        }
        return "Pila pila;\n"
               "pila_inicializar(&pila);";
    case ESTRUCTURA_COLA:
        if (operacion == OPERACION_INSERTAR) {
            return "Cola cola;\n"
                   "cola_inicializar(&cola);\n"
                   "bool ok = cola_encolar(&cola, valor);";
        }
        if (operacion == OPERACION_ELIMINAR) {
            return "int out = 0;\n"
                   "bool ok = cola_desencolar(&cola, &out);";
        }
        if (operacion == OPERACION_VACIAR) {
            return "cola_vaciar(&cola);\n"
                   "cola_inicializar(&cola);";
        }
        return "Cola cola;\n"
               "cola_inicializar(&cola);";
    case ESTRUCTURA_COLA_PRIORIDAD:
        if (operacion == OPERACION_INSERTAR) {
            return "ColaPrioridad cp;\n"
                   "cp_inicializar(&cp);\n"
                   "bool ok = cp_encolar(&cp, valor, prioridad);";
        }
        if (operacion == OPERACION_ELIMINAR) {
            return "int v = 0;\n"
                   "int p = 0;\n"
                   "bool ok = cp_desencolar(&cp, &v, &p);";
        }
        if (operacion == OPERACION_VACIAR) {
            return "cp_vaciar(&cp);\n"
                   "cp_inicializar(&cp);";
        }
        return "ColaPrioridad cp;\n"
               "cp_inicializar(&cp);";
    case ESTRUCTURA_LISTA:
        if (operacion == OPERACION_INSERTAR_INICIO) {
            return "Lista lista;\n"
                   "lista_inicializar(&lista);\n"
                   "bool ok = lista_insertar_inicio(&lista, valor);";
        }
        if (operacion == OPERACION_INSERTAR_FINAL) {
            return "Lista lista;\n"
                   "lista_inicializar(&lista);\n"
                   "bool ok = lista_insertar_final(&lista, valor);";
        }
        if (operacion == OPERACION_INSERTAR) {
            return "bool ok = lista_insertar_final(&lista, valor);";
        }
        if (operacion == OPERACION_ELIMINAR) {
            return "bool ok = lista_eliminar_primero(&lista, valor);";
        }
        if (operacion == OPERACION_BUSCAR) {
            return "int n = lista_buscar_posiciones(&lista, valor, NULL, 0);\n"
                   "/* n = numero de coincidencias */";
        }
        if (operacion == OPERACION_INVERTIR) {
            return "lista_invertir(&lista);\n"
                   "/* la cabeza cambia al antiguo ultimo nodo */";
        }
        if (operacion == OPERACION_VACIAR) {
            return "lista_destruir(&lista);\n"
                   "lista_inicializar(&lista);";
        }
        return "Lista lista;\n"
               "lista_inicializar(&lista);";
    default:
        return "// Operacion no disponible";
    }
}
