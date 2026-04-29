#include "algorithm_trace.h"

/**
 * @file algorithm_trace.c
 * @brief Tabla de mensajes pedagógicos y complejidades por operacion.
 */

/** @brief Retorna traza paso a paso y complejidad para la operacion activa. */
AlgorithmInfo algorithm_trace_get_info(TipoEstructura estructura, TipoOperacion operacion) {
    AlgorithmInfo info;

    info.pasos = "1) Validar estado. 2) Ejecutar API publica TAD. 3) Refrescar paneles.";
    info.tiempo = "O(1)";
    info.espacio = "O(1)";

    if (operacion == OPERACION_INICIALIZAR || operacion == OPERACION_VACIAR) {
        info.pasos = "1) Recorrer estructura. 2) Liberar nodos. 3) Dejar punteros en NULL.";
        info.tiempo = "O(n)";
        info.espacio = "O(1)";
        return info;
    }

    switch (estructura) {
    case ESTRUCTURA_PILA:
        if (operacion == OPERACION_INSERTAR) {
            info.pasos = "1) Reservar nodo. 2) Enlazar al tope actual. 3) Actualizar tope.";
            info.tiempo = "O(1)";
        } else if (operacion == OPERACION_ELIMINAR) {
            info.pasos = "1) Validar no vacia. 2) Tomar tope. 3) Mover tope y liberar nodo.";
            info.tiempo = "O(1)";
        }
        break;
    case ESTRUCTURA_COLA:
        if (operacion == OPERACION_INSERTAR) {
            info.pasos = "1) Reservar nodo. 2) Enlazar al final. 3) Actualizar puntero atras.";
            info.tiempo = "O(1)";
        } else if (operacion == OPERACION_ELIMINAR) {
            info.pasos = "1) Validar no vacia. 2) Retirar frente. 3) Actualizar frente/atras.";
            info.tiempo = "O(1)";
        }
        break;
    case ESTRUCTURA_COLA_PRIORIDAD:
        if (operacion == OPERACION_INSERTAR) {
            info.pasos = "1) Encolar al final. 2) Guardar prioridad. 3) Mantener orden FIFO interno.";
            info.tiempo = "O(1)";
        } else if (operacion == OPERACION_ELIMINAR) {
            info.pasos = "1) Recorrer para mejor prioridad. 2) Desenlazar objetivo. 3) Liberar nodo.";
            info.tiempo = "O(n)";
        }
        break;
    case ESTRUCTURA_LISTA:
        if (operacion == OPERACION_INSERTAR_INICIO) {
            info.pasos = "1) Reservar nodo. 2) Enlazar con cabeza actual. 3) Actualizar cabeza.";
            info.tiempo = "O(1)";
        } else if (operacion == OPERACION_INSERTAR_FINAL || operacion == OPERACION_INSERTAR) {
            info.pasos = "1) Recorrer hasta final. 2) Enlazar nuevo nodo. 3) Mantener cabeza.";
            info.tiempo = "O(n)";
        } else if (operacion == OPERACION_ELIMINAR) {
            info.pasos = "1) Buscar primera ocurrencia. 2) Reenlazar nodos. 3) Liberar nodo.";
            info.tiempo = "O(n)";
        } else if (operacion == OPERACION_BUSCAR) {
            info.pasos = "1) Recorrer nodo a nodo. 2) Comparar valor. 3) Contar coincidencias.";
            info.tiempo = "O(n)";
        } else if (operacion == OPERACION_INVERTIR) {
            info.pasos = "1) Usar prev/curr/next. 2) Invertir enlaces. 3) Actualizar cabeza.";
            info.tiempo = "O(n)";
        }
        break;
    default:
        break;
    }

    return info;
}
