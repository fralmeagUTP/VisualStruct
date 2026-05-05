#include <stdbool.h>
#include <stdio.h>

#include "pila.h"
#include "cola.h"
#include "cola_prioridad.h"
#include "lista.h"
#include "lista_circular.h"
#include "sublista.h"

static int g_failures = 0;

#define CHECK(cond, msg) \
    do { \
        if (cond) { printf("[PASS] %s\n", msg); } \
        else { printf("[FAIL] %s\n", msg); g_failures++; } \
    } while (0)

static bool arr_eq(const int *a, const int *b, int n) {
    int i;
    for (i = 0; i < n; i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

int main(void) {
    int out[16];
    int v;
    int p;

    /* Pila */
    Pila pila;
    pila_inicializar(&pila);
    CHECK(pila_vacia(&pila), "pila inicial vacia");
    CHECK(pila_push(&pila, 10), "pila push 10");
    CHECK(pila_push(&pila, 20), "pila push 20");
    CHECK(pila_contar(&pila) == 2, "pila contar = 2");
    CHECK(pila_pop(&pila, &v) && v == 20, "pila pop devuelve 20");
    CHECK(pila_pop(&pila, &v) && v == 10, "pila pop devuelve 10");
    CHECK(!pila_pop(&pila, &v), "pila pop falla en vacia");
    pila_destruir(&pila);

    /* Cola */
    Cola cola;
    cola_inicializar(&cola);
    CHECK(cola_encolar(&cola, 1), "cola encolar 1");
    CHECK(cola_encolar(&cola, 2), "cola encolar 2");
    CHECK(cola_encolar(&cola, 3), "cola encolar 3");
    CHECK(cola_contar(&cola) == 3, "cola contar = 3");
    CHECK(cola_desencolar(&cola, &v) && v == 1, "cola desencolar 1");
    CHECK(cola_desencolar(&cola, &v) && v == 2, "cola desencolar 2");
    CHECK(cola_desencolar(&cola, &v) && v == 3, "cola desencolar 3");
    CHECK(!cola_desencolar(&cola, &v), "cola desencolar falla en vacia");
    cola_vaciar(&cola);

    /* Cola de prioridad */
    ColaPrioridad cp;
    cp_inicializar(&cp);
    CHECK(cp_encolar(&cp, 100, 5), "cp encolar (100,5)");
    CHECK(cp_encolar(&cp, 200, 1), "cp encolar (200,1)");
    CHECK(cp_encolar(&cp, 300, 3), "cp encolar (300,3)");
    CHECK(cp_desencolar(&cp, &v, &p) && v == 200 && p == 1, "cp desencolar prioridad minima");
    CHECK(cp_desencolar(&cp, &v, &p) && v == 300 && p == 3, "cp desencolar segundo");
    CHECK(cp_desencolar(&cp, &v, &p) && v == 100 && p == 5, "cp desencolar tercero");
    CHECK(!cp_desencolar(&cp, &v, &p), "cp desencolar falla en vacia");
    cp_vaciar(&cp);

    /* Lista */
    Lista lista;
    lista_inicializar(&lista);
    CHECK(lista_insertar_inicio(&lista, 2), "lista insertar inicio 2");
    CHECK(lista_insertar_inicio(&lista, 1), "lista insertar inicio 1");
    CHECK(lista_insertar_final(&lista, 3), "lista insertar final 3");
    CHECK(lista_contar(&lista) == 3, "lista contar = 3");
    CHECK(lista_copiar_valores(&lista, out, 16) == 3, "lista copiar valores 3");
    {
        int exp[] = {1,2,3};
        CHECK(arr_eq(out, exp, 3), "lista orden esperado 1,2,3");
    }
    lista_invertir(&lista);
    CHECK(lista_copiar_valores(&lista, out, 16) == 3, "lista copiar invertida");
    {
        int exp[] = {3,2,1};
        CHECK(arr_eq(out, exp, 3), "lista invertida 3,2,1");
    }
    CHECK(lista_eliminar_primero(&lista, 2), "lista eliminar primero 2");
    lista_destruir(&lista);

    /* Lista circular */
    ListaCircular lcir;
    lcir_inicializar(&lcir);
    CHECK(lcir_insertar_final(&lcir, 1), "lcir insertar final 1");
    CHECK(lcir_insertar_final(&lcir, 2), "lcir insertar final 2");
    CHECK(lcir_insertar_inicio(&lcir, 0), "lcir insertar inicio 0");
    CHECK(lcir_contar(&lcir) == 3, "lcir contar = 3");
    CHECK(lcir_copiar_valores(&lcir, out, 16) == 3, "lcir copiar 3");
    {
        int exp[] = {0,1,2};
        CHECK(arr_eq(out, exp, 3), "lcir orden esperado 0,1,2");
    }
    lcir_invertir(&lcir);
    CHECK(lcir_copiar_valores(&lcir, out, 16) == 3, "lcir copiar invertida");
    {
        int exp[] = {2,1,0};
        CHECK(arr_eq(out, exp, 3), "lcir invertida 2,1,0");
    }
    CHECK(lcir_eliminar_primero(&lcir, 1), "lcir eliminar 1");
    lcir_destruir(&lcir);

    /* Sublistas */
    {
        Nodo *sub = NULL;
        Nodo *p10;
        sublista_inicializar(&sub);
        p10 = sublista_insertar_padre_final(&sub, 10);
        CHECK(p10 != NULL, "sublista insertar padre 10");
        CHECK(sublista_insertar_padre_final(&sub, 20) != NULL, "sublista insertar padre 20");
        CHECK(sublista_contar_padres(sub) == 2, "sublista contar padres = 2");
        CHECK(sublista_insertar_hijo_final(p10, 101), "sublista insertar hijo 101 en 10");
        CHECK(sublista_insertar_hijo_final(p10, 102), "sublista insertar hijo 102 en 10");
        CHECK(sublista_contar_hijos(p10) == 2, "sublista contar hijos padre 10 = 2");
        CHECK(sublista_copiar_hijos(p10, out, 16) == 2, "sublista copiar hijos");
        {
            int exp[] = {101,102};
            CHECK(arr_eq(out, exp, 2), "sublista hijos esperados 101,102");
        }
        CHECK(sublista_eliminar_hijo_primero(p10, 101), "sublista eliminar hijo 101");
        CHECK(sublista_eliminar_padre_primero(&sub, 20), "sublista eliminar padre 20");
        sublista_destruir(&sub);
        CHECK(sub == NULL, "sublista destruir deja NULL");
    }

    if (g_failures == 0) {
        printf("\nRESULTADO: OK (secuenciales smoke pass).\n");
        return 0;
    }

    printf("\nRESULTADO: FALLA (%d pruebas).\n", g_failures);
    return 1;
}
