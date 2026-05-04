/**
 * @file grafo_code_viewer.c
 * @brief Implementación del visualizador de código
 */

#include "grafo_code_viewer.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * Pseudocódigos de Algoritmos
 * ============================================================================ */

GrafoCodigoAlgoritmo grafo_codigo_bfs(void) {
    GrafoCodigoAlgoritmo codigo;
    strcpy(codigo.nombre_algoritmo, "BFS - Búsqueda en Amplitud");
    codigo.linea_actual = -1;
    
    const char *lineas[] = {
        "BFS(grafo, inicio):",
        "  cola = crearCola()",
        "  marcado[inicio] = true",
        "  encolar(cola, inicio)",
        "  mientras cola no esté vacía:",
        "    v = desencolar(cola)",
        "    procesar(v)",
        "    para cada vecino u de v:",
        "      si no marcado[u]:",
        "        marcado[u] = true",
        "        encolar(cola, u)"
    };
    
    codigo.cantidad_lineas = sizeof(lineas) / sizeof(lineas[0]);
    for (int i = 0; i < codigo.cantidad_lineas; i++) {
        strcpy(codigo.lineas[i].contenido, lineas[i]);
        codigo.lineas[i].numero_linea = i + 1;
        codigo.lineas[i].es_linea_actual = false;
        codigo.lineas[i].esta_resaltada = false;
    }
    
    return codigo;
}

GrafoCodigoAlgoritmo grafo_codigo_dfs(void) {
    GrafoCodigoAlgoritmo codigo;
    strcpy(codigo.nombre_algoritmo, "DFS - Búsqueda en Profundidad");
    codigo.linea_actual = -1;
    
    const char *lineas[] = {
        "DFS(grafo, inicio):",
        "  pila = crearPila()",
        "  marcado[inicio] = true",
        "  apilar(pila, inicio)",
        "  mientras pila no esté vacía:",
        "    v = desapilar(pila)",
        "    procesar(v)",
        "    para cada vecino u de v:",
        "      si no marcado[u]:",
        "        marcado[u] = true",
        "        apilar(pila, u)"
    };
    
    codigo.cantidad_lineas = sizeof(lineas) / sizeof(lineas[0]);
    for (int i = 0; i < codigo.cantidad_lineas; i++) {
        strcpy(codigo.lineas[i].contenido, lineas[i]);
        codigo.lineas[i].numero_linea = i + 1;
        codigo.lineas[i].es_linea_actual = false;
        codigo.lineas[i].esta_resaltada = false;
    }
    
    return codigo;
}

GrafoCodigoAlgoritmo grafo_codigo_dijkstra(void) {
    GrafoCodigoAlgoritmo codigo;
    strcpy(codigo.nombre_algoritmo, "Dijkstra - Camino Mínimo");
    codigo.linea_actual = -1;
    
    const char *lineas[] = {
        "Dijkstra(grafo, inicio):",
        "  distancia[inicio] = 0",
        "  para cada vértice v != inicio:",
        "    distancia[v] = INFINITO",
        "  visitado = conjunto vacío",
        "  mientras hay vértices no visitados:",
        "    u = vértice no visitado con min distancia",
        "    visitado.agregar(u)",
        "    para cada vecino v de u:",
        "      si distancia[u] + peso(u,v) < distancia[v]:",
        "        distancia[v] = distancia[u] + peso(u,v)"
    };
    
    codigo.cantidad_lineas = sizeof(lineas) / sizeof(lineas[0]);
    for (int i = 0; i < codigo.cantidad_lineas; i++) {
        strcpy(codigo.lineas[i].contenido, lineas[i]);
        codigo.lineas[i].numero_linea = i + 1;
        codigo.lineas[i].es_linea_actual = false;
        codigo.lineas[i].esta_resaltada = false;
    }
    
    return codigo;
}

GrafoCodigoAlgoritmo grafo_codigo_bellman_ford(void) {
    GrafoCodigoAlgoritmo codigo;
    strcpy(codigo.nombre_algoritmo, "Bellman-Ford - Camino con Pesos Negativos");
    codigo.linea_actual = -1;
    
    const char *lineas[] = {
        "BellmanFord(grafo, inicio):",
        "  distancia[inicio] = 0",
        "  para cada vértice v != inicio:",
        "    distancia[v] = INFINITO",
        "  para i = 1 hasta V-1:",
        "    para cada arista (u,v,w) en grafo:",
        "      si distancia[u] + w < distancia[v]:",
        "        distancia[v] = distancia[u] + w",
        "  para cada arista (u,v,w) en grafo:",
        "    si distancia[u] + w < distancia[v]:",
        "      reportar ciclo negativo"
    };
    
    codigo.cantidad_lineas = sizeof(lineas) / sizeof(lineas[0]);
    for (int i = 0; i < codigo.cantidad_lineas; i++) {
        strcpy(codigo.lineas[i].contenido, lineas[i]);
        codigo.lineas[i].numero_linea = i + 1;
        codigo.lineas[i].es_linea_actual = false;
        codigo.lineas[i].esta_resaltada = false;
    }
    
    return codigo;
}

GrafoCodigoAlgoritmo grafo_codigo_prim(void) {
    GrafoCodigoAlgoritmo codigo;
    strcpy(codigo.nombre_algoritmo, "Prim - Árbol Expandido Mínimo");
    codigo.linea_actual = -1;
    
    const char *lineas[] = {
        "Prim(grafo, inicio):",
        "  visitado = {inicio}",
        "  aristas_mst = []",
        "  mientras |visitado| < V:",
        "    (u,v,w) = arista mínima con u en visitado, v no en visitado",
        "    visitado.agregar(v)",
        "    aristas_mst.agregar((u,v,w))",
        "  retornar aristas_mst"
    };
    
    codigo.cantidad_lineas = sizeof(lineas) / sizeof(lineas[0]);
    for (int i = 0; i < codigo.cantidad_lineas; i++) {
        strcpy(codigo.lineas[i].contenido, lineas[i]);
        codigo.lineas[i].numero_linea = i + 1;
        codigo.lineas[i].es_linea_actual = false;
        codigo.lineas[i].esta_resaltada = false;
    }
    
    return codigo;
}

GrafoCodigoAlgoritmo grafo_codigo_kruskal(void) {
    GrafoCodigoAlgoritmo codigo;
    strcpy(codigo.nombre_algoritmo, "Kruskal - Árbol Expandido Mínimo");
    codigo.linea_actual = -1;
    
    const char *lineas[] = {
        "Kruskal(grafo):",
        "  aristas = ordenar(todas las aristas) por peso",
        "  uf = crearUnionFind(V)",
        "  aristas_mst = []",
        "  para cada arista (u,v,w) en aristas:",
        "    si uf.find(u) != uf.find(v):",
        "      uf.union(u,v)",
        "      aristas_mst.agregar((u,v,w))",
        "  retornar aristas_mst"
    };
    
    codigo.cantidad_lineas = sizeof(lineas) / sizeof(lineas[0]);
    for (int i = 0; i < codigo.cantidad_lineas; i++) {
        strcpy(codigo.lineas[i].contenido, lineas[i]);
        codigo.lineas[i].numero_linea = i + 1;
        codigo.lineas[i].es_linea_actual = false;
        codigo.lineas[i].esta_resaltada = false;
    }
    
    return codigo;
}

/* ============================================================================
 * Renderizado
 * ============================================================================ */

void grafo_codigo_dibujar_con_scroll(const GrafoCodigoAlgoritmo *codigo, Rectangle area_destino,
                                     float scroll_y) {
    if (!codigo) return;
    
    /* Titulo */
    DrawRectangleRec(area_destino, (Color){240, 240, 240, 255});
    DrawRectangleLinesEx(area_destino, 2.0f, (Color){100, 100, 100, 255});
    
    DrawText(codigo->nombre_algoritmo, (int)area_destino.x + 10, 
            (int)area_destino.y + 5, 14, BLACK);
    
    /* Lineas de codigo */
    int y_offset = (int)area_destino.y + 30 - (int)scroll_y;
    BeginScissorMode((int)area_destino.x + 2, (int)area_destino.y + 30,
                     (int)area_destino.width - 4, (int)area_destino.height - 32);
    for (int i = 0; i < codigo->cantidad_lineas; i++) {
        const GrafoCodigoLinea *linea = &codigo->lineas[i];
        
        Color color_fondo = (Color){255, 255, 255, 255};
        Color color_texto = BLACK;
        
        if (linea->es_linea_actual) {
            color_fondo = YELLOW;
            color_texto = BLACK;
        } else if (linea->esta_resaltada) {
            color_fondo = (Color){200, 220, 255, 255};
            color_texto = BLUE;
        }
        
        if (y_offset + 18 >= area_destino.y + 30 &&
            y_offset <= area_destino.y + area_destino.height) {
            DrawRectangle((int)area_destino.x + 2, y_offset, 
                         (int)area_destino.width - 4, 18, color_fondo);
            
            char buffer[280];
            snprintf(buffer, sizeof(buffer), "%2d: %s", linea->numero_linea, linea->contenido);
            DrawText(buffer, (int)area_destino.x + 8, y_offset + 2, 12, color_texto);
        }
        
        y_offset += 20;
    }
    EndScissorMode();
}

void grafo_codigo_dibujar(const GrafoCodigoAlgoritmo *codigo, Rectangle area_destino) {
    grafo_codigo_dibujar_con_scroll(codigo, area_destino, 0.0f);
}

void grafo_codigo_establecer_linea_actual(GrafoCodigoAlgoritmo *codigo, int numero_linea) {
    if (!codigo) return;
    
    for (int i = 0; i < codigo->cantidad_lineas; i++) {
        codigo->lineas[i].es_linea_actual = (codigo->lineas[i].numero_linea == numero_linea);
    }
}

void grafo_codigo_resaltar_linea(GrafoCodigoAlgoritmo *codigo, int numero_linea, bool resaltar) {
    if (!codigo) return;
    
    for (int i = 0; i < codigo->cantidad_lineas; i++) {
        if (codigo->lineas[i].numero_linea == numero_linea) {
            codigo->lineas[i].esta_resaltada = resaltar;
            break;
        }
    }
}
