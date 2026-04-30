#include "sublista.h"

#include <stdio.h>
#include <stdlib.h>

static Nodo *crear_padre(int valor) {
    Nodo *nuevo = (Nodo *)malloc(sizeof(Nodo));
    if (nuevo == NULL) {
        return NULL;
    }
    nuevo->nro = valor;
    nuevo->sgte = NULL;
    nuevo->sub = NULL;
    return nuevo;
}

static Sublista *crear_hijo(int valor) {
    Sublista *nuevo = (Sublista *)malloc(sizeof(Sublista));
    if (nuevo == NULL) {
        return NULL;
    }
    nuevo->nro = valor;
    nuevo->sgte = NULL;
    return nuevo;
}

static void destruir_hijos(Sublista **lista_hijos) {
    Sublista *actual;
    Sublista *next;

    if (lista_hijos == NULL) {
        return;
    }

    actual = *lista_hijos;
    while (actual != NULL) {
        next = actual->sgte;
        free(actual);
        actual = next;
    }
    *lista_hijos = NULL;
}

void sublista_inicializar(Nodo **lista) {
    if (lista == NULL) {
        return;
    }
    *lista = NULL;
}

Nodo *sublista_insertar_padre_final(Nodo **lista, int valor_padre) {
    Nodo *nuevo;
    Nodo *actual;

    if (lista == NULL) {
        return NULL;
    }

    nuevo = crear_padre(valor_padre);
    if (nuevo == NULL) {
        return NULL;
    }

    if (*lista == NULL) {
        *lista = nuevo;
        return nuevo;
    }

    actual = *lista;
    while (actual->sgte != NULL) {
        actual = actual->sgte;
    }
    actual->sgte = nuevo;
    return nuevo;
}

Nodo *sublista_buscar_padre(Nodo *lista, int valor_padre) {
    Nodo *actual = lista;
    while (actual != NULL) {
        if (actual->nro == valor_padre) {
            return actual;
        }
        actual = actual->sgte;
    }
    return NULL;
}

bool sublista_eliminar_padre_primero(Nodo **lista, int valor_padre) {
    Nodo *actual;
    Nodo *anterior = NULL;

    if (lista == NULL || *lista == NULL) {
        return false;
    }

    actual = *lista;
    while (actual != NULL) {
        if (actual->nro == valor_padre) {
            if (anterior == NULL) {
                *lista = actual->sgte;
            } else {
                anterior->sgte = actual->sgte;
            }
            destruir_hijos(&actual->sub);
            free(actual);
            return true;
        }
        anterior = actual;
        actual = actual->sgte;
    }

    return false;
}

int sublista_contar_padres(const Nodo *lista) {
    int n = 0;
    const Nodo *actual = lista;
    while (actual != NULL) {
        n++;
        actual = actual->sgte;
    }
    return n;
}

bool sublista_insertar_hijo_final(Nodo *padre, int valor_hijo) {
    Sublista *nuevo;
    Sublista *actual;

    if (padre == NULL) {
        return false;
    }

    nuevo = crear_hijo(valor_hijo);
    if (nuevo == NULL) {
        return false;
    }

    if (padre->sub == NULL) {
        padre->sub = nuevo;
        return true;
    }

    actual = padre->sub;
    while (actual->sgte != NULL) {
        actual = actual->sgte;
    }
    actual->sgte = nuevo;
    return true;
}

Sublista *sublista_buscar_hijo(Sublista *lista_hijos, int valor_hijo) {
    Sublista *actual = lista_hijos;
    while (actual != NULL) {
        if (actual->nro == valor_hijo) {
            return actual;
        }
        actual = actual->sgte;
    }
    return NULL;
}

bool sublista_eliminar_hijo_primero(Nodo *padre, int valor_hijo) {
    Sublista *actual;
    Sublista *anterior = NULL;

    if (padre == NULL || padre->sub == NULL) {
        return false;
    }

    actual = padre->sub;
    while (actual != NULL) {
        if (actual->nro == valor_hijo) {
            if (anterior == NULL) {
                padre->sub = actual->sgte;
            } else {
                anterior->sgte = actual->sgte;
            }
            free(actual);
            return true;
        }
        anterior = actual;
        actual = actual->sgte;
    }

    return false;
}

int sublista_contar_hijos(const Nodo *padre) {
    int n = 0;
    const Sublista *actual;

    if (padre == NULL) {
        return 0;
    }

    actual = padre->sub;
    while (actual != NULL) {
        n++;
        actual = actual->sgte;
    }
    return n;
}

int sublista_copiar_hijos(const Nodo *padre, int *destino, int capacidad) {
    int usados = 0;
    const Sublista *actual;

    if (padre == NULL || destino == NULL || capacidad <= 0) {
        return 0;
    }

    actual = padre->sub;
    while (actual != NULL && usados < capacidad) {
        destino[usados] = actual->nro;
        usados++;
        actual = actual->sgte;
    }
    return usados;
}

void sublista_formatear(const Nodo *lista, char *destino, size_t capacidad) {
    const Nodo *padre;
    const Sublista *hijo;
    size_t usado = 0;
    int escritos;

    if (destino == NULL || capacidad == 0) {
        return;
    }

    destino[0] = '\0';
    if (lista == NULL) {
        snprintf(destino, capacidad, "Lista padre vacia");
        return;
    }

    padre = lista;
    while (padre != NULL && usado < capacidad) {
        escritos = snprintf(destino + usado, capacidad - usado, "P(%d): ", padre->nro);
        if (escritos < 0) {
            return;
        }
        usado += (size_t)escritos;

        hijo = padre->sub;
        if (hijo == NULL) {
            escritos = snprintf(destino + usado, capacidad - usado, "(sin hijos)");
            if (escritos < 0) {
                return;
            }
            usado += (size_t)escritos;
        } else {
            while (hijo != NULL && usado < capacidad) {
                escritos = snprintf(destino + usado, capacidad - usado, "[%d]", hijo->nro);
                if (escritos < 0) {
                    return;
                }
                usado += (size_t)escritos;
                hijo = hijo->sgte;
                if (hijo != NULL && usado < capacidad) {
                    escritos = snprintf(destino + usado, capacidad - usado, " -> ");
                    if (escritos < 0) {
                        return;
                    }
                    usado += (size_t)escritos;
                }
            }
        }

        padre = padre->sgte;
        if (padre != NULL && usado < capacidad) {
            escritos = snprintf(destino + usado, capacidad - usado, "\n");
            if (escritos < 0) {
                return;
            }
            usado += (size_t)escritos;
        }
    }
}

void sublista_destruir(Nodo **lista) {
    Nodo *actual;
    Nodo *next;

    if (lista == NULL) {
        return;
    }

    actual = *lista;
    while (actual != NULL) {
        next = actual->sgte;
        destruir_hijos(&actual->sub);
        free(actual);
        actual = next;
    }
    *lista = NULL;
}
