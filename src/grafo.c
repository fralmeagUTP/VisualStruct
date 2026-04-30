/**
 * @file grafo.c
 * @brief Implementación del TAD Grafo con algoritmos clásicos.
 *
 * Implementa grafos dirigidos y no dirigidos usando lista de adyacencia.
 * Los vértices se almacenan en una lista enlazada, y cada vértice tiene
 * una lista enlazada de aristas salientes.
 *
 * @author Francisco Alejandro Medina Aguirre
 * @date 2026
 * @version 1.0
 */

#include "grafo.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* ============================================================================
 * Estructuras internas (no visibles al usuario)
 * ============================================================================ */

/**
 * @brief Nodo de una arista en la lista de adyacencia.
 */
typedef struct NodoArista {
    int destino;                    /**< Vértice destino */
    int peso;                       /**< Peso de la arista */
    struct NodoArista *siguiente;   /**< Siguiente arista */
} NodoArista;

/**
 * @brief Nodo de un vértice en la lista de vértices.
 */
typedef struct NodoVertice {
    int vertice;                    /**< Identificador del vértice */
    NodoArista *aristas;            /**< Lista de aristas salientes */
    struct NodoVertice *siguiente;  /**< Siguiente vértice */
} NodoVertice;

/**
 * @brief Estructura interna del grafo.
 */
struct Grafo {
    NodoVertice *cabeza;            /**< Cabeza de la lista de vértices */
    bool dirigido;                  /**< true si dirigido; false si no dirigido */
};

/* ============================================================================
 * Funciones auxiliares privadas
 * ============================================================================ */

/**
 * @brief Busca un vértice en el grafo.
 *
 * @param[in] grafo Puntero al grafo
 * @param[in] vertice Identificador del vértice
 * @return Puntero al nodo del vértice; NULL si no existe
 */
static NodoVertice *grafo_buscar_vertice(const Grafo *grafo, int vertice)
{
    if (!grafo) return NULL;
    
    NodoVertice *actual = grafo->cabeza;
    while (actual) {
        if (actual->vertice == vertice) {
            return actual;
        }
        actual = actual->siguiente;
    }
    return NULL;
}

/**
 * @brief Busca una arista en la lista de adyacencia de un vértice.
 *
 * @param[in] nodo_vertice Puntero al nodo del vértice
 * @param[in] destino Vértice destino de la arista
 * @return Puntero a la arista; NULL si no existe
 */
static NodoArista *grafo_buscar_arista(const NodoVertice *nodo_vertice, int destino)
{
    if (!nodo_vertice) return NULL;
    
    NodoArista *actual = nodo_vertice->aristas;
    while (actual) {
        if (actual->destino == destino) {
            return actual;
        }
        actual = actual->siguiente;
    }
    return NULL;
}

/**
 * @brief Inserta una arista en la lista de adyacencia (sin validar duplicados).
 *
 * @param[in,out] nodo_vertice Puntero al nodo del vértice
 * @param[in] destino Vértice destino
 * @param[in] peso Peso de la arista
 * @return true si la inserción fue exitosa; false si falla asignación de memoria
 */
static bool grafo_insertar_arista_interna(NodoVertice *nodo_vertice, int destino, int peso)
{
    if (!nodo_vertice) return false;
    
    NodoArista *nueva = (NodoArista *)malloc(sizeof(NodoArista));
    if (!nueva) return false;
    
    nueva->destino = destino;
    nueva->peso = peso;
    nueva->siguiente = nodo_vertice->aristas;
    nodo_vertice->aristas = nueva;
    
    return true;
}

/**
 * @brief Elimina una arista de la lista de adyacencia.
 *
 * @param[in,out] nodo_vertice Puntero al nodo del vértice
 * @param[in] destino Vértice destino de la arista a eliminar
 * @return true si se eliminó; false si la arista no existe
 */
static bool grafo_eliminar_arista_interna(NodoVertice *nodo_vertice, int destino)
{
    if (!nodo_vertice) return false;
    
    if (nodo_vertice->aristas && nodo_vertice->aristas->destino == destino) {
        NodoArista *temp = nodo_vertice->aristas;
        nodo_vertice->aristas = nodo_vertice->aristas->siguiente;
        free(temp);
        return true;
    }
    
    NodoArista *actual = nodo_vertice->aristas;
    while (actual && actual->siguiente) {
        if (actual->siguiente->destino == destino) {
            NodoArista *temp = actual->siguiente;
            actual->siguiente = actual->siguiente->siguiente;
            free(temp);
            return true;
        }
        actual = actual->siguiente;
    }
    
    return false;
}

/**
 * @brief Libera todas las aristas de un vértice.
 *
 * @param[in,out] nodo_vertice Puntero al nodo del vértice
 */
static void grafo_liberar_aristas(NodoVertice *nodo_vertice)
{
    if (!nodo_vertice) return;
    
    NodoArista *actual = nodo_vertice->aristas;
    while (actual) {
        NodoArista *temp = actual;
        actual = actual->siguiente;
        free(temp);
    }
    nodo_vertice->aristas = NULL;
}

/* ============================================================================
 * Creación y destrucción
 * ============================================================================ */

Grafo *grafo_crear(bool dirigido)
{
    Grafo *nuevo = (Grafo *)malloc(sizeof(Grafo));
    if (!nuevo) return NULL;
    
    nuevo->cabeza = NULL;
    nuevo->dirigido = dirigido;
    
    return nuevo;
}

void grafo_destruir(Grafo **grafo)
{
    if (!grafo || !*grafo) return;
    
    NodoVertice *actual = (*grafo)->cabeza;
    while (actual) {
        NodoVertice *temp = actual;
        actual = actual->siguiente;
        grafo_liberar_aristas(temp);
        free(temp);
    }
    
    free(*grafo);
    *grafo = NULL;
}

bool grafo_es_dirigido(const Grafo *grafo)
{
    if (!grafo) return false;
    return grafo->dirigido;
}

/* ============================================================================
 * Operaciones estructurales
 * ============================================================================ */

GrafoEstado grafo_insertar_vertice(Grafo *grafo, int vertice)
{
    if (!grafo) return GRAFO_ERROR_NULO;
    
    if (grafo_existe_vertice(grafo, vertice)) {
        return GRAFO_ERROR_YA_EXISTE;
    }
    
    NodoVertice *nuevo = (NodoVertice *)malloc(sizeof(NodoVertice));
    if (!nuevo) return GRAFO_ERROR_MEMORIA;
    
    nuevo->vertice = vertice;
    nuevo->aristas = NULL;
    nuevo->siguiente = grafo->cabeza;
    grafo->cabeza = nuevo;
    
    return GRAFO_OK;
}

GrafoEstado grafo_insertar_arista(Grafo *grafo, int origen, int destino, int peso)
{
    if (!grafo) return GRAFO_ERROR_NULO;
    
    NodoVertice *nodo_origen = grafo_buscar_vertice(grafo, origen);
    if (!nodo_origen) return GRAFO_ERROR_NO_EXISTE;
    
    NodoVertice *nodo_destino = grafo_buscar_vertice(grafo, destino);
    if (!nodo_destino) return GRAFO_ERROR_NO_EXISTE;
    
    if (grafo_existe_arista(grafo, origen, destino)) {
        return GRAFO_ERROR_YA_EXISTE;
    }
    
    if (!grafo_insertar_arista_interna(nodo_origen, destino, peso)) {
        return GRAFO_ERROR_MEMORIA;
    }
    
    if (!grafo->dirigido) {
        if (!grafo_insertar_arista_interna(nodo_destino, origen, peso)) {
            grafo_eliminar_arista_interna(nodo_origen, destino);
            return GRAFO_ERROR_MEMORIA;
        }
    }
    
    return GRAFO_OK;
}

GrafoEstado grafo_eliminar_vertice(Grafo *grafo, int vertice)
{
    if (!grafo) return GRAFO_ERROR_NULO;
    
    NodoVertice *nodo = grafo_buscar_vertice(grafo, vertice);
    if (!nodo) return GRAFO_ERROR_NO_EXISTE;
    
    /* Eliminar aristas incidentes de otros vértices */
    NodoVertice *actual = grafo->cabeza;
    while (actual) {
        if (actual->vertice != vertice) {
            grafo_eliminar_arista_interna(actual, vertice);
        }
        actual = actual->siguiente;
    }
    
    /* Eliminar el vértice de la lista */
    if (grafo->cabeza->vertice == vertice) {
        NodoVertice *temp = grafo->cabeza;
        grafo->cabeza = grafo->cabeza->siguiente;
        grafo_liberar_aristas(temp);
        free(temp);
    } else {
        NodoVertice *anterior = grafo->cabeza;
        while (anterior && anterior->siguiente->vertice != vertice) {
            anterior = anterior->siguiente;
        }
        if (anterior) {
            NodoVertice *temp = anterior->siguiente;
            anterior->siguiente = anterior->siguiente->siguiente;
            grafo_liberar_aristas(temp);
            free(temp);
        }
    }
    
    return GRAFO_OK;
}

GrafoEstado grafo_eliminar_arista(Grafo *grafo, int origen, int destino)
{
    if (!grafo) return GRAFO_ERROR_NULO;
    
    NodoVertice *nodo_origen = grafo_buscar_vertice(grafo, origen);
    if (!nodo_origen) return GRAFO_ERROR_NO_EXISTE;
    
    if (!grafo_existe_arista(grafo, origen, destino)) {
        return GRAFO_ERROR_NO_EXISTE;
    }
    
    grafo_eliminar_arista_interna(nodo_origen, destino);
    
    if (!grafo->dirigido) {
        NodoVertice *nodo_destino = grafo_buscar_vertice(grafo, destino);
        if (nodo_destino) {
            grafo_eliminar_arista_interna(nodo_destino, origen);
        }
    }
    
    return GRAFO_OK;
}

/* ============================================================================
 * Consultas
 * ============================================================================ */

bool grafo_existe_vertice(const Grafo *grafo, int vertice)
{
    return grafo_buscar_vertice(grafo, vertice) != NULL;
}

bool grafo_existe_arista(const Grafo *grafo, int origen, int destino)
{
    if (!grafo) return false;
    
    NodoVertice *nodo_origen = grafo_buscar_vertice(grafo, origen);
    if (!nodo_origen) return false;
    
    return grafo_buscar_arista(nodo_origen, destino) != NULL;
}

GrafoEstado grafo_obtener_peso(const Grafo *grafo, int origen, int destino, int *peso)
{
    if (!grafo || !peso) return GRAFO_ERROR_NULO;
    
    NodoVertice *nodo_origen = grafo_buscar_vertice(grafo, origen);
    if (!nodo_origen) return GRAFO_ERROR_NO_EXISTE;
    
    NodoArista *arista = grafo_buscar_arista(nodo_origen, destino);
    if (!arista) return GRAFO_ERROR_NO_EXISTE;
    
    *peso = arista->peso;
    return GRAFO_OK;
}

size_t grafo_orden(const Grafo *grafo)
{
    if (!grafo) return 0;
    
    size_t cantidad = 0;
    NodoVertice *actual = grafo->cabeza;
    while (actual) {
        cantidad++;
        actual = actual->siguiente;
    }
    return cantidad;
}

size_t grafo_tamano(const Grafo *grafo)
{
    if (!grafo) return 0;
    
    size_t cantidad = 0;
    NodoVertice *actual = grafo->cabeza;
    while (actual) {
        NodoArista *arista = actual->aristas;
        while (arista) {
            cantidad++;
            arista = arista->siguiente;
        }
        actual = actual->siguiente;
    }
    
    /* En grafos no dirigidos, cada arista se cuenta dos veces */
    if (!grafo->dirigido) {
        cantidad /= 2;
    }
    
    return cantidad;
}

GrafoEstado grafo_grado_salida(const Grafo *grafo, int vertice, size_t *grado)
{
    if (!grafo || !grado) return GRAFO_ERROR_NULO;
    
    NodoVertice *nodo = grafo_buscar_vertice(grafo, vertice);
    if (!nodo) return GRAFO_ERROR_NO_EXISTE;
    
    *grado = 0;
    NodoArista *actual = nodo->aristas;
    while (actual) {
        (*grado)++;
        actual = actual->siguiente;
    }
    
    return GRAFO_OK;
}

GrafoEstado grafo_grado_entrada(const Grafo *grafo, int vertice, size_t *grado)
{
    if (!grafo || !grado) return GRAFO_ERROR_NULO;
    
    if (!grafo_existe_vertice(grafo, vertice)) {
        return GRAFO_ERROR_NO_EXISTE;
    }
    
    *grado = 0;
    NodoVertice *actual = grafo->cabeza;
    while (actual) {
        if (grafo_existe_arista(grafo, actual->vertice, vertice)) {
            (*grado)++;
        }
        actual = actual->siguiente;
    }
    
    return GRAFO_OK;
}

/* ============================================================================
 * Copias públicas de datos
 * ============================================================================ */

GrafoEstado grafo_obtener_vertices(const Grafo *grafo, int **vertices, size_t *cantidad)
{
    if (!grafo || !vertices || !cantidad) return GRAFO_ERROR_NULO;
    
    *cantidad = grafo_orden(grafo);
    if (*cantidad == 0) {
        *vertices = NULL;
        return GRAFO_OK;
    }
    
    *vertices = (int *)malloc((*cantidad) * sizeof(int));
    if (!*vertices) return GRAFO_ERROR_MEMORIA;
    
    size_t idx = 0;
    NodoVertice *actual = grafo->cabeza;
    while (actual) {
        (*vertices)[idx++] = actual->vertice;
        actual = actual->siguiente;
    }
    
    return GRAFO_OK;
}

GrafoEstado grafo_obtener_aristas(const Grafo *grafo, GrafoArista **aristas, size_t *cantidad)
{
    if (!grafo || !aristas || !cantidad) return GRAFO_ERROR_NULO;
    
    *cantidad = grafo_tamano(grafo);
    if (*cantidad == 0) {
        *aristas = NULL;
        return GRAFO_OK;
    }
    
    *aristas = (GrafoArista *)malloc((*cantidad) * sizeof(GrafoArista));
    if (!*aristas) return GRAFO_ERROR_MEMORIA;
    
    size_t idx = 0;
    NodoVertice *actual = grafo->cabeza;
    while (actual) {
        NodoArista *arista = actual->aristas;
        while (arista) {
            if (grafo->dirigido || actual->vertice < arista->destino) {
                (*aristas)[idx].origen = actual->vertice;
                (*aristas)[idx].destino = arista->destino;
                (*aristas)[idx].peso = arista->peso;
                idx++;
            }
            arista = arista->siguiente;
        }
        actual = actual->siguiente;
    }
    
    return GRAFO_OK;
}

GrafoEstado grafo_sucesores(const Grafo *grafo, int vertice, int **sucesores, size_t *cantidad)
{
    if (!grafo || !sucesores || !cantidad) return GRAFO_ERROR_NULO;
    
    NodoVertice *nodo = grafo_buscar_vertice(grafo, vertice);
    if (!nodo) return GRAFO_ERROR_NO_EXISTE;
    
    *cantidad = 0;
    NodoArista *arista = nodo->aristas;
    while (arista) {
        (*cantidad)++;
        arista = arista->siguiente;
    }
    
    if (*cantidad == 0) {
        *sucesores = NULL;
        return GRAFO_OK;
    }
    
    *sucesores = (int *)malloc((*cantidad) * sizeof(int));
    if (!*sucesores) return GRAFO_ERROR_MEMORIA;
    
    size_t idx = 0;
    arista = nodo->aristas;
    while (arista) {
        (*sucesores)[idx++] = arista->destino;
        arista = arista->siguiente;
    }
    
    return GRAFO_OK;
}

GrafoEstado grafo_predecesores(const Grafo *grafo, int vertice, int **predecesores, size_t *cantidad)
{
    if (!grafo || !predecesores || !cantidad) return GRAFO_ERROR_NULO;
    
    if (!grafo_existe_vertice(grafo, vertice)) {
        return GRAFO_ERROR_NO_EXISTE;
    }
    
    *cantidad = 0;
    NodoVertice *actual = grafo->cabeza;
    while (actual) {
        if (grafo_existe_arista(grafo, actual->vertice, vertice)) {
            (*cantidad)++;
        }
        actual = actual->siguiente;
    }
    
    if (*cantidad == 0) {
        *predecesores = NULL;
        return GRAFO_OK;
    }
    
    *predecesores = (int *)malloc((*cantidad) * sizeof(int));
    if (!*predecesores) return GRAFO_ERROR_MEMORIA;
    
    size_t idx = 0;
    actual = grafo->cabeza;
    while (actual) {
        if (grafo_existe_arista(grafo, actual->vertice, vertice)) {
            (*predecesores)[idx++] = actual->vertice;
        }
        actual = actual->siguiente;
    }
    
    return GRAFO_OK;
}

/* ============================================================================
 * Algoritmos
 * ============================================================================ */

GrafoRecorrido grafo_bfs(const Grafo *grafo, int inicio)
{
    GrafoRecorrido resultado = {NULL, 0, GRAFO_ERROR_NULO};
    
    if (!grafo) return resultado;
    
    if (!grafo_existe_vertice(grafo, inicio)) {
        resultado.estado = GRAFO_ERROR_NO_EXISTE;
        return resultado;
    }
    
    size_t orden = grafo_orden(grafo);
    if (orden == 0) {
        resultado.estado = GRAFO_OK;
        return resultado;
    }
    
    /* Arrays auxiliares */
    bool *visitado = (bool *)calloc(5000, sizeof(bool));
    int *cola = (int *)malloc(orden * sizeof(int));
    resultado.vertices = (int *)malloc(orden * sizeof(int));
    
    if (!visitado || !cola || !resultado.vertices) {
        free(visitado);
        free(cola);
        free(resultado.vertices);
        resultado.vertices = NULL;
        resultado.estado = GRAFO_ERROR_MEMORIA;
        return resultado;
    }
    
    /* BFS */
    int frente = 0, atras = 0;
    visitado[inicio] = true;
    cola[atras++] = inicio;
    
    while (frente < atras) {
        int actual = cola[frente++];
        resultado.vertices[resultado.cantidad++] = actual;
        
        int *sucesores = NULL;
        size_t cant_sucesores = 0;
        if (grafo_sucesores(grafo, actual, &sucesores, &cant_sucesores) == GRAFO_OK && sucesores) {
            for (size_t i = 0; i < cant_sucesores; i++) {
                if (!visitado[sucesores[i]]) {
                    visitado[sucesores[i]] = true;
                    cola[atras++] = sucesores[i];
                }
            }
            free(sucesores);
        }
    }
    
    free(visitado);
    free(cola);
    resultado.estado = GRAFO_OK;
    return resultado;
}

GrafoRecorrido grafo_dfs(const Grafo *grafo, int inicio)
{
    GrafoRecorrido resultado = {NULL, 0, GRAFO_ERROR_NULO};
    
    if (!grafo) return resultado;
    
    if (!grafo_existe_vertice(grafo, inicio)) {
        resultado.estado = GRAFO_ERROR_NO_EXISTE;
        return resultado;
    }
    
    size_t orden = grafo_orden(grafo);
    if (orden == 0) {
        resultado.estado = GRAFO_OK;
        return resultado;
    }
    
    /* Arrays auxiliares */
    bool *visitado = (bool *)calloc(5000, sizeof(bool));
    int *pila = (int *)malloc(orden * sizeof(int));
    resultado.vertices = (int *)malloc(orden * sizeof(int));
    
    if (!visitado || !pila || !resultado.vertices) {
        free(visitado);
        free(pila);
        free(resultado.vertices);
        resultado.vertices = NULL;
        resultado.estado = GRAFO_ERROR_MEMORIA;
        return resultado;
    }
    
    /* DFS iterativo */
    int tope = 0;
    visitado[inicio] = true;
    pila[tope++] = inicio;
    
    while (tope > 0) {
        int actual = pila[--tope];
        resultado.vertices[resultado.cantidad++] = actual;
        
        int *sucesores = NULL;
        size_t cant_sucesores = 0;
        if (grafo_sucesores(grafo, actual, &sucesores, &cant_sucesores) == GRAFO_OK && sucesores) {
            for (size_t i = 0; i < cant_sucesores; i++) {
                if (!visitado[sucesores[i]]) {
                    visitado[sucesores[i]] = true;
                    pila[tope++] = sucesores[i];
                }
            }
            free(sucesores);
        }
    }
    
    free(visitado);
    free(pila);
    resultado.estado = GRAFO_OK;
    return resultado;
}

GrafoCamino grafo_dijkstra(const Grafo *grafo, int origen, int destino)
{
    GrafoCamino resultado = {NULL, 0, 0, false, GRAFO_ERROR_NULO};
    
    if (!grafo) return resultado;
    
    if (!grafo_existe_vertice(grafo, origen) || !grafo_existe_vertice(grafo, destino)) {
        resultado.estado = GRAFO_ERROR_NO_EXISTE;
        return resultado;
    }
    
    /* Verificar pesos negativos */
    int *vertices = NULL;
    size_t n_vertices = 0;
    if (grafo_obtener_vertices(grafo, &vertices, &n_vertices) != GRAFO_OK) {
        resultado.estado = GRAFO_ERROR_MEMORIA;
        return resultado;
    }
    
    for (size_t i = 0; i < n_vertices; i++) {
        int *sucesores = NULL;
        size_t cant = 0;
        if (grafo_sucesores(grafo, vertices[i], &sucesores, &cant) == GRAFO_OK && sucesores) {
            for (size_t j = 0; j < cant; j++) {
                int peso = 0;
                if (grafo_obtener_peso(grafo, vertices[i], sucesores[j], &peso) == GRAFO_OK && peso < 0) {
                    free(vertices);
                    free(sucesores);
                    resultado.estado = GRAFO_ERROR_PESO_NEGATIVO;
                    return resultado;
                }
            }
            free(sucesores);
        }
    }
    free(vertices);
    
    /* Arrays para Dijkstra */
    int dist[5000];
    int predecesor[5000];
    bool visitado[5000] = {false};
    
    for (int i = 0; i < 5000; i++) {
        dist[i] = INT_MAX;
        predecesor[i] = -1;
    }
    
    dist[origen] = 0;
    
    for (size_t i = 0; i < n_vertices; i++) {
        int min_dist = INT_MAX;
        int u = -1;
        
        if (grafo_obtener_vertices(grafo, &vertices, &n_vertices) == GRAFO_OK && vertices) {
            for (size_t j = 0; j < n_vertices; j++) {
                if (!visitado[vertices[j]] && dist[vertices[j]] < min_dist) {
                    min_dist = dist[vertices[j]];
                    u = vertices[j];
                }
            }
            free(vertices);
        }
        
        if (u == -1) break;
        visitado[u] = true;
        
        int *sucesores = NULL;
        size_t cant = 0;
        if (grafo_sucesores(grafo, u, &sucesores, &cant) == GRAFO_OK && sucesores) {
            for (size_t j = 0; j < cant; j++) {
                int v = sucesores[j];
                int peso = 0;
                grafo_obtener_peso(grafo, u, v, &peso);
                
                if (dist[u] != INT_MAX && dist[u] + peso < dist[v]) {
                    dist[v] = dist[u] + peso;
                    predecesor[v] = u;
                }
            }
            free(sucesores);
        }
    }
    
    if (dist[destino] == INT_MAX) {
        resultado.estado = GRAFO_OK;
        resultado.existe = false;
        return resultado;
    }
    
    /* Reconstruir camino */
    int camino_aux[5000];
    int tope = 0;
    int actual = destino;
    
    while (actual != -1) {
        camino_aux[tope++] = actual;
        actual = predecesor[actual];
    }
    
    resultado.cantidad = tope;
    resultado.aristas = (GrafoArista *)malloc(resultado.cantidad * sizeof(GrafoArista));
    if (!resultado.aristas) {
        resultado.estado = GRAFO_ERROR_MEMORIA;
        return resultado;
    }
    
    for (int i = tope - 1; i > 0; i--) {
        int u = camino_aux[i];
        int v = camino_aux[i - 1];
        int peso = 0;
        grafo_obtener_peso(grafo, u, v, &peso);
        
        resultado.aristas[tope - 1 - i].origen = u;
        resultado.aristas[tope - 1 - i].destino = v;
        resultado.aristas[tope - 1 - i].peso = peso;
    }
    
    resultado.costo_total = dist[destino];
    resultado.existe = true;
    resultado.estado = GRAFO_OK;
    return resultado;
}

GrafoCamino grafo_bellman_ford(const Grafo *grafo, int origen, int destino)
{
    GrafoCamino resultado = {NULL, 0, 0, false, GRAFO_ERROR_NULO};
    
    if (!grafo) return resultado;
    
    if (!grafo_existe_vertice(grafo, origen) || !grafo_existe_vertice(grafo, destino)) {
        resultado.estado = GRAFO_ERROR_NO_EXISTE;
        return resultado;
    }
    
    size_t n = grafo_orden(grafo);
    int dist[5000];
    int predecesor[5000] = {0};
    
    for (int i = 0; i < 5000; i++) {
        dist[i] = INT_MAX;
        predecesor[i] = -1;
    }
    
    dist[origen] = 0;
    
    /* Relajación V-1 veces */
    for (size_t i = 1; i < n; i++) {
        int *vertices = NULL;
        size_t cant_v = 0;
        if (grafo_obtener_vertices(grafo, &vertices, &cant_v) == GRAFO_OK && vertices) {
            for (size_t j = 0; j < cant_v; j++) {
                int u = vertices[j];
                int *sucesores = NULL;
                size_t cant = 0;
                if (grafo_sucesores(grafo, u, &sucesores, &cant) == GRAFO_OK && sucesores) {
                    for (size_t k = 0; k < cant; k++) {
                        int v = sucesores[k];
                        int peso = 0;
                        grafo_obtener_peso(grafo, u, v, &peso);
                        
                        if (dist[u] != INT_MAX && dist[u] + peso < dist[v]) {
                            dist[v] = dist[u] + peso;
                            predecesor[v] = u;
                        }
                    }
                    free(sucesores);
                }
            }
            free(vertices);
        }
    }
    
    /* Verificar ciclos negativos */
    int *vertices = NULL;
    size_t cant_v = 0;
    if (grafo_obtener_vertices(grafo, &vertices, &cant_v) == GRAFO_OK && vertices) {
        for (size_t j = 0; j < cant_v; j++) {
            int u = vertices[j];
            int *sucesores = NULL;
            size_t cant = 0;
            if (grafo_sucesores(grafo, u, &sucesores, &cant) == GRAFO_OK && sucesores) {
                for (size_t k = 0; k < cant; k++) {
                    int v = sucesores[k];
                    int peso = 0;
                    grafo_obtener_peso(grafo, u, v, &peso);
                    
                    if (dist[u] != INT_MAX && dist[u] + peso < dist[v]) {
                        free(vertices);
                        free(sucesores);
                        resultado.estado = GRAFO_ERROR_CICLO_NEGATIVO;
                        return resultado;
                    }
                }
                free(sucesores);
            }
        }
        free(vertices);
    }
    
    if (dist[destino] == INT_MAX) {
        resultado.estado = GRAFO_OK;
        resultado.existe = false;
        return resultado;
    }
    
    /* Reconstruir camino (igual que Dijkstra) */
    int camino_aux[5000];
    int tope = 0;
    int actual = destino;
    
    while (actual != -1) {
        camino_aux[tope++] = actual;
        actual = predecesor[actual];
    }
    
    resultado.cantidad = tope;
    resultado.aristas = (GrafoArista *)malloc(resultado.cantidad * sizeof(GrafoArista));
    if (!resultado.aristas) {
        resultado.estado = GRAFO_ERROR_MEMORIA;
        return resultado;
    }
    
    for (int i = tope - 1; i > 0; i--) {
        int u = camino_aux[i];
        int v = camino_aux[i - 1];
        int peso = 0;
        grafo_obtener_peso(grafo, u, v, &peso);
        
        resultado.aristas[tope - 1 - i].origen = u;
        resultado.aristas[tope - 1 - i].destino = v;
        resultado.aristas[tope - 1 - i].peso = peso;
    }
    
    resultado.costo_total = dist[destino];
    resultado.existe = true;
    resultado.estado = GRAFO_OK;
    return resultado;
}

GrafoCamino grafo_prim(const Grafo *grafo, int inicio)
{
    GrafoCamino resultado = {NULL, 0, 0, false, GRAFO_ERROR_NULO};
    
    if (!grafo) return resultado;
    if (grafo->dirigido) {
        resultado.estado = GRAFO_ERROR_YA_EXISTE;
        return resultado;
    }
    
    if (!grafo_existe_vertice(grafo, inicio)) {
        resultado.estado = GRAFO_ERROR_NO_EXISTE;
        return resultado;
    }
    
    size_t n = grafo_orden(grafo);
    if (n == 0) {
        resultado.estado = GRAFO_OK;
        return resultado;
    }
    
    bool en_mst[5000] = {false};
    int clave[5000];
    int padre[5000];
    
    for (int i = 0; i < 5000; i++) {
        clave[i] = INT_MAX;
        padre[i] = -1;
    }
    
    clave[inicio] = 0;
    
    for (size_t i = 0; i < n; i++) {
        int min_clave = INT_MAX;
        int u = -1;
        
        int *vertices = NULL;
        size_t cant_v = 0;
        if (grafo_obtener_vertices(grafo, &vertices, &cant_v) == GRAFO_OK && vertices) {
            for (size_t j = 0; j < cant_v; j++) {
                if (!en_mst[vertices[j]] && clave[vertices[j]] < min_clave) {
                    min_clave = clave[vertices[j]];
                    u = vertices[j];
                }
            }
            free(vertices);
        }
        
        if (u == -1 || min_clave == INT_MAX) break;
        
        en_mst[u] = true;
        
        int *sucesores = NULL;
        size_t cant = 0;
        if (grafo_sucesores(grafo, u, &sucesores, &cant) == GRAFO_OK && sucesores) {
            for (size_t j = 0; j < cant; j++) {
                int v = sucesores[j];
                int peso = 0;
                grafo_obtener_peso(grafo, u, v, &peso);
                
                if (!en_mst[v] && peso < clave[v]) {
                    clave[v] = peso;
                    padre[v] = u;
                }
            }
            free(sucesores);
        }
    }
    
    /* Contar aristas del MST */
    int aristas_count = 0;
    for (int i = 0; i < 5000; i++) {
        if (padre[i] != -1) aristas_count++;
    }
    
    resultado.cantidad = aristas_count;
    resultado.aristas = (GrafoArista *)malloc(resultado.cantidad * sizeof(GrafoArista));
    if (!resultado.aristas && resultado.cantidad > 0) {
        resultado.estado = GRAFO_ERROR_MEMORIA;
        return resultado;
    }
    
    int idx = 0;
    int costo = 0;
    for (int i = 0; i < 5000; i++) {
        if (padre[i] != -1) {
            resultado.aristas[idx].origen = padre[i];
            resultado.aristas[idx].destino = i;
            resultado.aristas[idx].peso = clave[i];
            costo += clave[i];
            idx++;
        }
    }
    
    resultado.costo_total = costo;
    resultado.existe = (aristas_count == (int)n - 1);
    resultado.estado = GRAFO_OK;
    return resultado;
}

GrafoCamino grafo_kruskal(const Grafo *grafo)
{
    GrafoCamino resultado = {NULL, 0, 0, false, GRAFO_ERROR_NULO};
    
    if (!grafo) return resultado;
    if (grafo->dirigido) {
        resultado.estado = GRAFO_ERROR_YA_EXISTE;
        return resultado;
    }
    
    size_t n = grafo_orden(grafo);
    if (n == 0) {
        resultado.estado = GRAFO_OK;
        return resultado;
    }
    
    /* Obtener aristas */
    GrafoArista *aristas = NULL;
    size_t m = 0;
    if (grafo_obtener_aristas(grafo, &aristas, &m) != GRAFO_OK) {
        resultado.estado = GRAFO_ERROR_MEMORIA;
        return resultado;
    }
    
    /* Ordenar aristas por peso (burbuja simple para grafos pequeños) */
    for (size_t i = 0; i < m; i++) {
        for (size_t j = i + 1; j < m; j++) {
            if (aristas[j].peso < aristas[i].peso) {
                GrafoArista temp = aristas[i];
                aristas[i] = aristas[j];
                aristas[j] = temp;
            }
        }
    }
    
    /* Union-Find */
    int padre_uf[5000];
    for (int i = 0; i < 5000; i++) padre_uf[i] = i;
    
    resultado.aristas = (GrafoArista *)malloc(n * sizeof(GrafoArista));
    if (!resultado.aristas && n > 0) {
        free(aristas);
        resultado.estado = GRAFO_ERROR_MEMORIA;
        return resultado;
    }
    
    int costo = 0;
    for (size_t i = 0; i < m && resultado.cantidad < n - 1; i++) {
        /* Encontrar raíz de origen */
        int u = aristas[i].origen;
        while (padre_uf[u] != u) u = padre_uf[u];
        
        /* Encontrar raíz de destino */
        int v = aristas[i].destino;
        while (padre_uf[v] != v) v = padre_uf[v];
        
        if (u != v) {
            padre_uf[u] = v;
            resultado.aristas[resultado.cantidad] = aristas[i];
            costo += aristas[i].peso;
            resultado.cantidad++;
        }
    }
    
    free(aristas);
    resultado.costo_total = costo;
    resultado.existe = (resultado.cantidad == n - 1);
    resultado.estado = GRAFO_OK;
    return resultado;
}

/* ============================================================================
 * Liberación de resultados
 * ============================================================================ */

void grafo_liberar_recorrido(GrafoRecorrido *recorrido)
{
    if (!recorrido) return;
    
    if (recorrido->vertices) {
        free(recorrido->vertices);
        recorrido->vertices = NULL;
    }
    recorrido->cantidad = 0;
    recorrido->estado = GRAFO_OK;
}

void grafo_liberar_camino(GrafoCamino *camino)
{
    if (!camino) return;
    
    if (camino->aristas) {
        free(camino->aristas);
        camino->aristas = NULL;
    }
    camino->cantidad = 0;
    camino->costo_total = 0;
    camino->existe = false;
    camino->estado = GRAFO_OK;
}

const char *grafo_estado_cadena(GrafoEstado estado)
{
    switch (estado) {
        case GRAFO_OK:
            return "GRAFO_OK";
        case GRAFO_ERROR_NULO:
            return "GRAFO_ERROR_NULO";
        case GRAFO_ERROR_MEMORIA:
            return "GRAFO_ERROR_MEMORIA";
        case GRAFO_ERROR_NO_EXISTE:
            return "GRAFO_ERROR_NO_EXISTE";
        case GRAFO_ERROR_YA_EXISTE:
            return "GRAFO_ERROR_YA_EXISTE";
        case GRAFO_ERROR_PESO_NEGATIVO:
            return "GRAFO_ERROR_PESO_NEGATIVO";
        case GRAFO_ERROR_CICLO_NEGATIVO:
            return "GRAFO_ERROR_CICLO_NEGATIVO";
        default:
            return "GRAFO_ERROR_DESCONOCIDO";
    }
}
