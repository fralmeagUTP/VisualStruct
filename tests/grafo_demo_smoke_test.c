#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "app_state.h"
#include "grafo.h"

static unsigned long hash_aristas(const Grafo *g) {
    GrafoArista *ar = NULL;
    size_t n = 0;
    size_t i;
    unsigned long h = 1469598103u;

    if (grafo_obtener_aristas(g, &ar, &n) != GRAFO_OK || ar == NULL) {
        return 0u;
    }

    for (i = 0; i < n; i++) {
        h ^= (unsigned long)(ar[i].origen * 73856093);
        h *= 16777619u;
        h ^= (unsigned long)(ar[i].destino * 19349663);
        h *= 16777619u;
        h ^= (unsigned long)(ar[i].peso * 83492791);
        h *= 16777619u;
    }

    free(ar);
    return h ^ (unsigned long)n;
}

int main(void) {
    AppState app;
    size_t n_vertices;
    size_t n_aristas;
    int objetivo = 12;
    unsigned long h1;
    unsigned long h2;

    app_state_init(&app);
    app_state_set_estructura(&app, ESTRUCTURA_GRAFO);
    app_state_set_valor(&app, objetivo);

    app_state_grafo_cargar_demo(&app);
    n_vertices = grafo_orden(app.grafo);
    n_aristas = grafo_tamano(app.grafo);
    h1 = hash_aristas(app.grafo);

    if (n_vertices != (size_t)objetivo) {
        printf("[FAIL] demo no respeta cantidad de nodos: esperado=%d real=%zu\n", objetivo,
               n_vertices);
        app_state_shutdown(&app);
        return 1;
    }

    if (n_aristas < (size_t)(objetivo - 1)) {
        printf("[FAIL] demo no garantiza conectividad base: aristas=%zu\n", n_aristas);
        app_state_shutdown(&app);
        return 1;
    }

    app_state_set_valor(&app, objetivo);
    app_state_grafo_cargar_demo(&app);
    h2 = hash_aristas(app.grafo);

    if (h1 == h2) {
        /* Un tercer intento para reducir falsa alarma por coincidencia aleatoria */
        unsigned long h3;
        app_state_set_valor(&app, objetivo);
        app_state_grafo_cargar_demo(&app);
        h3 = hash_aristas(app.grafo);
        if (h3 == h2) {
            printf("[FAIL] demo no evidencia variacion de aristas en 3 cargas consecutivas\n");
            app_state_shutdown(&app);
            return 1;
        }
    }

    printf("[PASS] demo respeta nodos=%d\n", objetivo);
    printf("[PASS] demo garantiza aristas base (>= n-1)\n");
    printf("[PASS] demo evidencia variacion de aristas entre cargas\n");

    app_state_shutdown(&app);
    return 0;
}
