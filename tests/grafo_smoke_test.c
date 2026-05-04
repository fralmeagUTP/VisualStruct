#include <stdbool.h>
#include <stdio.h>

#include "grafo.h"

static int g_failures = 0;

#define CHECK(cond, msg)            \
    do {                            \
        if (cond) {                 \
            printf("[PASS] %s\n", msg); \
        } else {                    \
            printf("[FAIL] %s\n", msg); \
            g_failures++;           \
        }                           \
    } while (0)

static bool contains_all_vertices(const int *vals, size_t n, const int *expected, size_t m) {
    if (n != m) return false;
    for (size_t i = 0; i < m; i++) {
        bool found = false;
        for (size_t j = 0; j < n; j++) {
            if (vals[j] == expected[i]) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

int main(void) {
    Grafo *g = grafo_crear(false);
    CHECK(g != NULL, "crear grafo no dirigido");
    if (!g) return 1;

    CHECK(grafo_insertar_vertice(g, 1) == GRAFO_OK, "insertar vertice 1");
    CHECK(grafo_insertar_vertice(g, 2) == GRAFO_OK, "insertar vertice 2");
    CHECK(grafo_insertar_vertice(g, 3) == GRAFO_OK, "insertar vertice 3");
    CHECK(grafo_insertar_vertice(g, 4) == GRAFO_OK, "insertar vertice 4");
    CHECK(grafo_insertar_vertice(g, 5) == GRAFO_OK, "insertar vertice 5");
    CHECK(grafo_insertar_vertice(g, 1) == GRAFO_ERROR_YA_EXISTE, "vertice duplicado reporta YA_EXISTE");

    CHECK(grafo_insertar_arista(g, 1, 2, 2) == GRAFO_OK, "insertar arista 1-2");
    CHECK(grafo_insertar_arista(g, 1, 3, 5) == GRAFO_OK, "insertar arista 1-3");
    CHECK(grafo_insertar_arista(g, 2, 3, 1) == GRAFO_OK, "insertar arista 2-3");
    CHECK(grafo_insertar_arista(g, 2, 4, 2) == GRAFO_OK, "insertar arista 2-4");
    CHECK(grafo_insertar_arista(g, 3, 4, 3) == GRAFO_OK, "insertar arista 3-4");
    CHECK(grafo_insertar_arista(g, 4, 5, 1) == GRAFO_OK, "insertar arista 4-5");

    CHECK(grafo_orden(g) == 5, "orden del grafo = 5");
    CHECK(grafo_tamano(g) == 6, "tamano del grafo = 6 aristas");

    GrafoRecorrido bfs = grafo_bfs(g, 1);
    int expected_vertices[] = {1, 2, 3, 4, 5};
    CHECK(bfs.estado == GRAFO_OK, "BFS retorna estado OK");
    CHECK(contains_all_vertices(bfs.vertices, bfs.cantidad, expected_vertices, 5), "BFS visita los 5 vertices");
    grafo_liberar_recorrido(&bfs);

    GrafoRecorrido dfs = grafo_dfs(g, 1);
    CHECK(dfs.estado == GRAFO_OK, "DFS retorna estado OK");
    CHECK(contains_all_vertices(dfs.vertices, dfs.cantidad, expected_vertices, 5), "DFS visita los 5 vertices");
    grafo_liberar_recorrido(&dfs);

    GrafoCamino d = grafo_dijkstra(g, 1, 5);
    CHECK(d.estado == GRAFO_OK, "Dijkstra estado OK");
    CHECK(d.existe, "Dijkstra encuentra camino 1->5");
    CHECK(d.costo_total == 5, "Dijkstra costo total esperado = 5");
    CHECK(d.cantidad == 3, "Dijkstra cantidad de aristas esperada = 3");
    grafo_liberar_camino(&d);

    GrafoCamino bf = grafo_bellman_ford(g, 1, 5);
    CHECK(bf.estado == GRAFO_OK, "Bellman-Ford estado OK");
    CHECK(bf.existe, "Bellman-Ford encuentra camino 1->5");
    CHECK(bf.costo_total == 5, "Bellman-Ford costo total esperado = 5");
    CHECK(bf.cantidad == 3, "Bellman-Ford cantidad de aristas esperada = 3");
    grafo_liberar_camino(&bf);

    GrafoCamino prim = grafo_prim(g, 1);
    CHECK(prim.estado == GRAFO_OK, "Prim estado OK");
    CHECK(prim.existe, "Prim construye MST");
    CHECK(prim.cantidad == 4, "Prim devuelve 4 aristas (n-1)");
    CHECK(prim.costo_total == 6, "Prim costo MST esperado = 6");
    grafo_liberar_camino(&prim);

    GrafoCamino kr = grafo_kruskal(g);
    CHECK(kr.estado == GRAFO_OK, "Kruskal estado OK");
    CHECK(kr.existe, "Kruskal construye MST");
    CHECK(kr.cantidad == 4, "Kruskal devuelve 4 aristas (n-1)");
    CHECK(kr.costo_total == 6, "Kruskal costo MST esperado = 6");
    grafo_liberar_camino(&kr);

    grafo_destruir(&g);
    CHECK(g == NULL, "grafo_destruir deja puntero en NULL");

    Grafo *gd = grafo_crear(true);
    CHECK(gd != NULL, "crear grafo dirigido");
    if (gd) {
        CHECK(grafo_insertar_vertice(gd, 0) == GRAFO_OK, "insertar vertice 0 dirigido");
        CHECK(grafo_insertar_vertice(gd, 1) == GRAFO_OK, "insertar vertice 1 dirigido");
        CHECK(grafo_insertar_vertice(gd, 2) == GRAFO_OK, "insertar vertice 2 dirigido");
        CHECK(grafo_insertar_arista(gd, 0, 1, 1) == GRAFO_OK, "insertar arista 0->1");
        CHECK(grafo_insertar_arista(gd, 1, 2, -1) == GRAFO_OK, "insertar arista 1->2 negativa");
        CHECK(grafo_insertar_arista(gd, 2, 1, -1) == GRAFO_OK, "insertar arista 2->1 negativa");

        GrafoCamino dneg = grafo_dijkstra(gd, 0, 2);
        CHECK(dneg.estado == GRAFO_ERROR_PESO_NEGATIVO, "Dijkstra rechaza pesos negativos");
        grafo_liberar_camino(&dneg);

        GrafoCamino bfneg = grafo_bellman_ford(gd, 0, 2);
        CHECK(bfneg.estado == GRAFO_ERROR_CICLO_NEGATIVO, "Bellman-Ford detecta ciclo negativo");
        grafo_liberar_camino(&bfneg);

        GrafoCamino pdir = grafo_prim(gd, 0);
        CHECK(pdir.estado == GRAFO_ERROR_YA_EXISTE, "Prim en grafo dirigido reporta error esperado");
        grafo_liberar_camino(&pdir);

        GrafoCamino kdir = grafo_kruskal(gd);
        CHECK(kdir.estado == GRAFO_ERROR_YA_EXISTE, "Kruskal en grafo dirigido reporta error esperado");
        grafo_liberar_camino(&kdir);

        grafo_destruir(&gd);
    }

    if (g_failures == 0) {
        printf("\nRESULTADO: OK (todas las pruebas smoke pasaron).\n");
        return 0;
    }

    printf("\nRESULTADO: FALLA (%d pruebas fallaron).\n", g_failures);
    return 1;
}
