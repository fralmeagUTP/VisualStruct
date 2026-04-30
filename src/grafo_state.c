/**
 * @file grafo_state.c
 * @brief Implementación del estado visual del grafo
 */

#include "grafo_state.h"
#include <string.h>
#include <math.h>

/* ============================================================================
 * Inicialización y Destrucción
 * ============================================================================ */

GrafoState grafo_state_init(void) {
    GrafoState estado = {0};
    estado.cantidad_vertices = 0;
    estado.cantidad_aristas = 0;
    estado.es_dirigido = false;
    estado.algoritmo_activo = GRAFO_ALGO_NINGUNO;
    estado.paso_algoritmo = 0;
    estado.total_pasos = 0;
    
    for (int i = 0; i < 64; i++) {
        estado.vertices[i].id = -1;
        estado.vertices[i].estado = GRAFO_VÉRTICE_NORMAL;
        estado.vertices[i].visible = false;
        estado.vertices[i].distancia = 0;
        estado.vertices[i].predecesor = -1;
        estado.vertices[i].orden_visitacion = 0;
    }
    
    for (int i = 0; i < 256; i++) {
        estado.aristas[i].origen = -1;
        estado.aristas[i].destino = -1;
        estado.aristas[i].estado = GRAFO_ARISTA_NORMAL;
        estado.aristas[i].visible = false;
        estado.aristas[i].orden_examinacion = 0;
    }
    
    strcpy(estado.mensaje_estado, "Estado inicial");
    return estado;
}

void grafo_state_destruir(GrafoState *estado) {
    if (!estado) return;
    /* No hay memoria dinámica actualmente */
}

/* ============================================================================
 * Actualización de Estado
 * ============================================================================ */

bool grafo_state_marcar_vertice_visitado(GrafoState *estado, int id_vertice, int orden) {
    if (!estado || id_vertice < 0 || id_vertice >= 64) return false;
    
    for (int i = 0; i < estado->cantidad_vertices; i++) {
        if (estado->vertices[i].id == id_vertice) {
            estado->vertices[i].estado = GRAFO_VÉRTICE_VISITADO;
            estado->vertices[i].orden_visitacion = orden;
            return true;
        }
    }
    return false;
}

bool grafo_state_establecer_vertice_estado(GrafoState *estado, int id_vertice, 
                                          GrafoVerticeEstadoVisual nuevo_estado) {
    if (!estado || id_vertice < 0 || id_vertice >= 64) return false;
    
    for (int i = 0; i < estado->cantidad_vertices; i++) {
        if (estado->vertices[i].id == id_vertice) {
            estado->vertices[i].estado = nuevo_estado;
            return true;
        }
    }
    return false;
}

bool grafo_state_actualizar_distancia_vertice(GrafoState *estado, int id_vertice, 
                                             int nueva_distancia, int id_predecesor) {
    if (!estado || id_vertice < 0 || id_vertice >= 64) return false;
    
    for (int i = 0; i < estado->cantidad_vertices; i++) {
        if (estado->vertices[i].id == id_vertice) {
            estado->vertices[i].distancia = nueva_distancia;
            estado->vertices[i].predecesor = id_predecesor;
            return true;
        }
    }
    return false;
}

bool grafo_state_establecer_arista_estado(GrafoState *estado, int id_origen, 
                                         int id_destino, GrafoAristaEstadoVisual nuevo_estado) {
    if (!estado || id_origen < 0 || id_destino < 0) return false;
    
    for (int i = 0; i < estado->cantidad_aristas; i++) {
        if (estado->aristas[i].origen == id_origen && estado->aristas[i].destino == id_destino) {
            estado->aristas[i].estado = nuevo_estado;
            return true;
        }
    }
    return false;
}

void grafo_state_reiniciar_visuales(GrafoState *estado) {
    if (!estado) return;
    
    for (int i = 0; i < estado->cantidad_vertices; i++) {
        estado->vertices[i].estado = GRAFO_VÉRTICE_NORMAL;
        estado->vertices[i].distancia = 0;
        estado->vertices[i].predecesor = -1;
        estado->vertices[i].orden_visitacion = 0;
    }
    
    for (int i = 0; i < estado->cantidad_aristas; i++) {
        estado->aristas[i].estado = GRAFO_ARISTA_NORMAL;
        estado->aristas[i].orden_examinacion = 0;
    }
    
    estado->algoritmo_activo = GRAFO_ALGO_NINGUNO;
    estado->paso_algoritmo = 0;
    estado->total_pasos = 0;
    strcpy(estado->mensaje_estado, "Estado reiniciado");
}

bool grafo_state_cargar_vertices(GrafoState *estado, const void *grafo, int cantidad) {
    if (!estado || !grafo) return false;
    if (cantidad > 64) cantidad = 64;
    
    /* Implementación provisoria: se vinculará en Fase 7 */
    estado->cantidad_vertices = cantidad;
    return true;
}

bool grafo_state_cargar_aristas(GrafoState *estado, const void *grafo, int cantidad) {
    if (!estado || !grafo) return false;
    if (cantidad > 256) cantidad = 256;
    
    /* Implementación provisoria: se vinculará en Fase 7 */
    estado->cantidad_aristas = cantidad;
    return true;
}

/* ============================================================================
 * Consultas
 * ============================================================================ */

const GrafoVerticeVisual* grafo_state_obtener_vertice(const GrafoState *estado, int id_vertice) {
    if (!estado || id_vertice < 0 || id_vertice >= 64) return NULL;
    
    for (int i = 0; i < estado->cantidad_vertices; i++) {
        if (estado->vertices[i].id == id_vertice) {
            return &estado->vertices[i];
        }
    }
    return NULL;
}

const GrafoAristaVisual* grafo_state_obtener_arista(const GrafoState *estado, 
                                                    int id_origen, int id_destino) {
    if (!estado || id_origen < 0 || id_destino < 0) return NULL;
    
    for (int i = 0; i < estado->cantidad_aristas; i++) {
        if (estado->aristas[i].origen == id_origen && estado->aristas[i].destino == id_destino) {
            return &estado->aristas[i];
        }
    }
    return NULL;
}

int grafo_state_contar_visitados(const GrafoState *estado) {
    if (!estado) return 0;
    
    int contador = 0;
    for (int i = 0; i < estado->cantidad_vertices; i++) {
        if (estado->vertices[i].estado == GRAFO_VÉRTICE_VISITADO) {
            contador++;
        }
    }
    return contador;
}
