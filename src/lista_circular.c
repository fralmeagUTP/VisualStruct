#include "lista_circular.h"

#include <stdio.h>
#include <stdlib.h>

struct lcir_nodo {
    int valor;
    struct lcir_nodo *sgte;
};

static LCirNodo *lcir_crear_nodo(int valor) {
    LCirNodo *nodo = (LCirNodo *)malloc(sizeof(LCirNodo));
    if (nodo == NULL) {
        return NULL;
    }
    nodo->valor = valor;
    nodo->sgte = NULL;
    return nodo;
}

void lcir_inicializar(ListaCircular *lista) {
    if (lista == NULL) {
        return;
    }
    lista->cabeza = NULL;
    lista->cola = NULL;
}

bool lcir_insertar_inicio(ListaCircular *lista, int valor) {
    LCirNodo *nuevo;

    if (lista == NULL) {
        return false;
    }

    nuevo = lcir_crear_nodo(valor);
    if (nuevo == NULL) {
        return false;
    }

    if (lista->cabeza == NULL) {
        nuevo->sgte = nuevo;
        lista->cabeza = nuevo;
        lista->cola = nuevo;
        return true;
    }

    nuevo->sgte = lista->cabeza;
    lista->cola->sgte = nuevo;
    lista->cabeza = nuevo;
    return true;
}

bool lcir_insertar_final(ListaCircular *lista, int valor) {
    LCirNodo *nuevo;

    if (lista == NULL) {
        return false;
    }

    nuevo = lcir_crear_nodo(valor);
    if (nuevo == NULL) {
        return false;
    }

    if (lista->cabeza == NULL) {
        nuevo->sgte = nuevo;
        lista->cabeza = nuevo;
        lista->cola = nuevo;
        return true;
    }

    nuevo->sgte = lista->cabeza;
    lista->cola->sgte = nuevo;
    lista->cola = nuevo;
    return true;
}

int lcir_buscar_posiciones(const ListaCircular *lista, int valor, int *destino, int capacidad) {
    LCirNodo *actual;
    int encontrados = 0;
    int pos = 1;

    if (lista == NULL || lista->cabeza == NULL) {
        return 0;
    }

    actual = lista->cabeza;
    do {
        if (actual->valor == valor) {
            if (destino != NULL && encontrados < capacidad) {
                destino[encontrados] = pos;
            }
            encontrados++;
        }
        actual = actual->sgte;
        pos++;
    } while (actual != lista->cabeza);

    return encontrados;
}

bool lcir_eliminar_primero(ListaCircular *lista, int valor) {
    LCirNodo *actual;
    LCirNodo *anterior;

    if (lista == NULL || lista->cabeza == NULL) {
        return false;
    }

    actual = lista->cabeza;
    anterior = lista->cola;
    do {
        if (actual->valor == valor) {
            if (actual == lista->cabeza && actual == lista->cola) {
                free(actual);
                lista->cabeza = NULL;
                lista->cola = NULL;
                return true;
            }

            anterior->sgte = actual->sgte;
            if (actual == lista->cabeza) {
                lista->cabeza = actual->sgte;
            }
            if (actual == lista->cola) {
                lista->cola = anterior;
            }
            free(actual);
            return true;
        }
        anterior = actual;
        actual = actual->sgte;
    } while (actual != lista->cabeza);

    return false;
}

void lcir_invertir(ListaCircular *lista) {
    LCirNodo *prev;
    LCirNodo *curr;
    LCirNodo *next;
    LCirNodo *old_head;

    if (lista == NULL || lista->cabeza == NULL || lista->cabeza == lista->cola) {
        return;
    }

    prev = lista->cola;
    curr = lista->cabeza;
    do {
        next = curr->sgte;
        curr->sgte = prev;
        prev = curr;
        curr = next;
    } while (curr != lista->cabeza);

    old_head = lista->cabeza;
    lista->cabeza = lista->cola;
    lista->cola = old_head;
}

bool lcir_vacia(const ListaCircular *lista) {
    return lista == NULL || lista->cabeza == NULL;
}

int lcir_contar(const ListaCircular *lista) {
    LCirNodo *actual;
    int cantidad = 0;

    if (lista == NULL || lista->cabeza == NULL) {
        return 0;
    }

    actual = lista->cabeza;
    do {
        cantidad++;
        actual = actual->sgte;
    } while (actual != lista->cabeza);

    return cantidad;
}

int lcir_copiar_valores(const ListaCircular *lista, int *destino, int capacidad) {
    LCirNodo *actual;
    int usados = 0;

    if (lista == NULL || lista->cabeza == NULL || destino == NULL || capacidad <= 0) {
        return 0;
    }

    actual = lista->cabeza;
    do {
        if (usados >= capacidad) {
            break;
        }
        destino[usados] = actual->valor;
        usados++;
        actual = actual->sgte;
    } while (actual != lista->cabeza);

    return usados;
}

void lcir_formatear(const ListaCircular *lista, char *destino, size_t capacidad) {
    LCirNodo *actual;
    size_t usado = 0;
    int escritos;
    int pos = 1;

    if (destino == NULL || capacidad == 0) {
        return;
    }

    destino[0] = '\0';
    if (lista == NULL || lista->cabeza == NULL) {
        snprintf(destino, capacidad, "Lista circular vacia");
        return;
    }

    actual = lista->cabeza;
    escritos = snprintf(destino, capacidad, "HEAD -> ");
    if (escritos < 0) {
        return;
    }
    usado = (size_t)escritos;

    do {
        escritos = snprintf(destino + usado, capacidad - usado, "[%d]=%d", pos, actual->valor);
        if (escritos < 0) {
            return;
        }
        usado += (size_t)escritos;
        actual = actual->sgte;
        pos++;

        if (actual != lista->cabeza && usado < capacidad) {
            escritos = snprintf(destino + usado, capacidad - usado, " -> ");
            if (escritos < 0) {
                return;
            }
            usado += (size_t)escritos;
        }
    } while (actual != lista->cabeza && usado < capacidad);

    if (usado < capacidad) {
        snprintf(destino + usado, capacidad - usado, " -> (vuelve a HEAD)");
    }
}

void lcir_destruir(ListaCircular *lista) {
    LCirNodo *actual;
    LCirNodo *next;

    if (lista == NULL || lista->cabeza == NULL) {
        return;
    }

    actual = lista->cabeza->sgte;
    while (actual != NULL && actual != lista->cabeza) {
        next = actual->sgte;
        free(actual);
        actual = next;
    }
    free(lista->cabeza);
    lista->cabeza = NULL;
    lista->cola = NULL;
}
