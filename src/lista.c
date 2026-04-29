#include "lista.h"

#include <stdio.h>
#include <stdlib.h>

struct nodo {
    int nro;
    struct nodo *sgte;
};

static Nodo *crear_nodo(int valor) {
    Nodo *q = (Nodo *)malloc(sizeof(Nodo));
    if (q == NULL) {
        return NULL;
    }
    q->nro = valor;
    q->sgte = NULL;
    return q;
}

/**
 * @brief Inicializa una Lista poniéndola en estado vacío.
 *
 * Establece @c cabeza a NULL. Debe ser la primera llamada antes de
 * cualquier otra operación sobre la Lista. No reserva memoria dinámica.
 *
 * @param[out] lista Puntero a la Lista que se va a inicializar.
 *                   Si es NULL la función no hace nada.
 *
 * @post La lista queda vacía y lista para usarse.
 */
void lista_inicializar(Lista *lista) {
    if (lista == NULL) {
        return;
    }
    lista->cabeza = NULL;
}

/**
 * @brief Inserta un valor al inicio de la lista.
 *
 * @details
 * Reserva un nuevo @c Nodo y lo convierte en la nueva @c cabeza de la
 * lista. El antiguo primer nodo queda enlazado como segundo elemento.
 *
 * @param[in,out] lista Puntero a la Lista destino.
 * @param[in]     valor Valor entero a insertar.
 *
 * @return @c true  si la inserción fue exitosa.
 * @return @c false si @p lista es NULL o @c malloc() falla.
 *
 * @pre  La Lista debe haber sido inicializada con lista_inicializar().
 * @post El tamaño de la lista aumenta en 1 y el nuevo nodo es la @c cabeza.
 * @note Complejidad temporal: O(1).
 */
bool lista_insertar_inicio(Lista *lista, int valor) {
    Nodo *q;
    if (lista == NULL) {
        return false;
    }

    q = crear_nodo(valor);
    if (q == NULL) {
        return false;
    }

    q->sgte = lista->cabeza;
    lista->cabeza = q;
    return true;
}

/**
 * @brief Inserta un valor al final de la lista.
 *
 * @details
 * Recorre la lista hasta encontrar el último nodo (aquel cuyo @c sgte
 * es NULL) y enlaza el nuevo @c Nodo a continuación. Si la lista está
 * vacía, el nuevo nodo pasa a ser la @c cabeza.
 *
 * @param[in,out] lista Puntero a la Lista destino.
 * @param[in]     valor Valor entero a insertar.
 *
 * @return @c true  si la inserción fue exitosa.
 * @return @c false si @p lista es NULL o @c malloc() falla.
 *
 * @post El tamaño de la lista aumenta en 1 y el nuevo nodo es el último.
 * @note Complejidad temporal: O(n).
 */
bool lista_insertar_final(Lista *lista, int valor) {
    Nodo *q;
    Nodo *t;

    if (lista == NULL) {
        return false;
    }

    q = crear_nodo(valor);
    if (q == NULL) {
        return false;
    }

    if (lista->cabeza == NULL) {
        lista->cabeza = q;
        return true;
    }

    t = lista->cabeza;
    while (t->sgte != NULL) {
        t = t->sgte;
    }
    t->sgte = q;
    return true;
}

/**
 * @brief Inserta un valor antes de la posición indicada (1-based).
 *
 * @details
 * Si @p pos == 1 delega en lista_insertar_inicio(). Para @p pos > 1,
 * recorre la lista hasta el nodo en la posición @p pos-1 y enlaza
 * el nuevo @c Nodo entre éste y el que estaba en @p pos.
 *
 * @param[in,out] lista Puntero a la Lista destino.
 * @param[in]     valor Valor entero a insertar.
 * @param[in]     pos   Posición (1-based) delante de la cual insertar.
 *
 * @return @c true  si se insertó correctamente.
 * @return @c false si @p lista es NULL, @p pos < 1, la posición no
 *                  existe en la lista, o @c malloc() falla.
 *
 * @note Complejidad temporal: O(pos).
 */
bool lista_insertar_antes(Lista *lista, int valor, int pos) {
    Nodo *q;
    Nodo *actual;
    int i;

    if (lista == NULL || pos < 1) {
        return false;
    }

    if (pos == 1) {
        return lista_insertar_inicio(lista, valor);
    }

    q = crear_nodo(valor);
    if (q == NULL) {
        return false;
    }

    actual = lista->cabeza;
    for (i = 1; actual != NULL; i++) {
        if (i == pos - 1) {
            q->sgte = actual->sgte;
            actual->sgte = q;
            return true;
        }
        actual = actual->sgte;
    }

    free(q);
    return false;
}

/**
 * @brief Inserta un valor después de la posición indicada (1-based).
 *
 * @details
 * Recorre la lista hasta el nodo en la posición @p pos y enlaza
 * el nuevo @c Nodo entre éste y el que estaba en @p pos+1.
 *
 * @param[in,out] lista Puntero a la Lista destino.
 * @param[in]     valor Valor entero a insertar.
 * @param[in]     pos   Posición (1-based) después de la cual insertar.
 *
 * @return @c true  si se insertó correctamente.
 * @return @c false si @p lista es NULL, @p pos < 1, la posición no
 *                  existe en la lista, o @c malloc() falla.
 *
 * @note Complejidad temporal: O(pos).
 */
bool lista_insertar_despues(Lista *lista, int valor, int pos) {
    Nodo *q;
    Nodo *actual;
    int i;

    if (lista == NULL || pos < 1) {
        return false;
    }

    q = crear_nodo(valor);
    if (q == NULL) {
        return false;
    }

    actual = lista->cabeza;
    for (i = 1; actual != NULL; i++) {
        if (i == pos) {
            q->sgte = actual->sgte;
            actual->sgte = q;
            return true;
        }
        actual = actual->sgte;
    }

    free(q);
    return false;
}

/**
 * @brief Busca un valor y devuelve las posiciones (1-based) donde aparece.
 *
 * @details
 * Recorre toda la lista comparando cada nodo con @p valor. Cada vez que
 * encuentra una coincidencia, si @p destino no es NULL y queda capacidad,
 * almacena la posición del nodo. El conteo siempre se realiza aunque
 * @p destino sea NULL o se haya llenado el arreglo.
 *
 * @param[in]  lista     Puntero constante a la Lista.
 * @param[in]  valor     Valor entero a buscar.
 * @param[out] destino   Arreglo donde se almacenan las posiciones encontradas.
 *                       Puede ser NULL si solo se desea contar.
 * @param[in]  capacidad Tamaño máximo del arreglo @p destino.
 *
 * @return Número total de coincidencias encontradas en la lista.
 *         Retorna 0 si @p lista es NULL.
 *
 * @note Complejidad temporal: O(n).
 */
int lista_buscar_posiciones(const Lista *lista, int valor, int *destino, int capacidad) {
    Nodo *q;
    int i = 1;
    int encontrados = 0;

    if (lista == NULL) {
        return 0;
    }

    q = lista->cabeza;
    while (q != NULL) {
        if (q->nro == valor) {
            if (destino != NULL && encontrados < capacidad) {
                destino[encontrados] = i;
            }
            encontrados++;
        }
        q = q->sgte;
        i++;
    }

    return encontrados;
}

/**
 * @brief Elimina la primera ocurrencia de un valor en la lista.
 *
 * @details
 * Recorre la lista con un puntero @c p al nodo actual y @c ant al
 * anterior. Cuando encuentra un nodo con @c nro == @p valor, actualiza
 * el enlace de @c ant (o de @c cabeza si es el primero) y libera el nodo.
 * Se detiene tras la primera coincidencia.
 *
 * @param[in,out] lista Puntero a la Lista.
 * @param[in]     valor Valor entero a eliminar.
 *
 * @return @c true  si se encontró y eliminó el nodo.
 * @return @c false si @p lista es NULL o el valor no existe.
 *
 * @post El tamaño de la lista disminuye en 1 si retorna @c true.
 * @note Complejidad temporal: O(n).
 */
bool lista_eliminar_primero(Lista *lista, int valor) {
    Nodo *p;
    Nodo *ant;

    if (lista == NULL) {
        return false;
    }

    p = lista->cabeza;
    ant = NULL;
    while (p != NULL) {
        if (p->nro == valor) {
            if (ant == NULL) {
                lista->cabeza = p->sgte;
            } else {
                ant->sgte = p->sgte;
            }
            free(p);
            return true;
        }
        ant = p;
        p = p->sgte;
    }

    return false;
}

/**
 * @brief Elimina todas las ocurrencias de un valor en la lista.
 *
 * @details
 * Recorre la lista completa con punteros @c q (actual) y @c ant
 * (anterior). Cada vez que encuentra un nodo con @c nro == @p valor,
 * lo desenlaza y libera su memoria. Continúa hasta el final de la lista.
 *
 * @param[in,out] lista Puntero a la Lista.
 * @param[in]     valor Valor entero a eliminar.
 *
 * @return Número de nodos eliminados.
 *         Retorna 0 si @p lista es NULL o el valor no existe.
 *
 * @note Complejidad temporal: O(n).
 */
int lista_eliminar_todos(Lista *lista, int valor) {
    Nodo *q;
    Nodo *ant;
    Nodo *tmp;
    int eliminados = 0;

    if (lista == NULL) {
        return 0;
    }

    q = lista->cabeza;
    ant = NULL;
    while (q != NULL) {
        if (q->nro == valor) {
            tmp = q;
            if (ant == NULL) {
                lista->cabeza = q->sgte;
                q = lista->cabeza;
            } else {
                ant->sgte = q->sgte;
                q = ant->sgte;
            }
            free(tmp);
            eliminados++;
        } else {
            ant = q;
            q = q->sgte;
        }
    }

    return eliminados;
}

/**
 * @brief Invierte el orden de todos los nodos de la lista.
 *
 * @details
 * Aplica el algoritmo clásico de tres punteros (@c prev, @c curr, @c next)
 * para invertir los enlaces in-place sin reservar memoria adicional.
 * Al terminar, la antigua @c cabeza pasa a ser el último nodo y el
 * antiguo último nodo pasa a ser la nueva @c cabeza.
 *
 * @param[in,out] lista Puntero a la Lista a invertir.
 *                      Si es NULL la función no hace nada.
 *
 * @note Complejidad temporal: O(n).
 * @note Complejidad espacial: O(1) (no reserva memoria).
 */
void lista_invertir(Lista *lista) {
    Nodo *prev = NULL;
    Nodo *curr;
    Nodo *next;

    if (lista == NULL) {
        return;
    }

    curr = lista->cabeza;
    while (curr != NULL) {
        next = curr->sgte;
        curr->sgte = prev;
        prev = curr;
        curr = next;
    }
    lista->cabeza = prev;
}

/**
 * @brief Calcula el promedio aritmético de los valores de la lista.
 *
 * @details
 * Recorre la lista acumulando la suma en un @c long @c long para evitar
 * desbordamiento con listas largas. Al final divide por el conteo y
 * escribe el resultado como @c float.
 *
 * @param[in]  lista     Puntero constante a la Lista.
 * @param[out] resultado Puntero donde se escribe el promedio calculado.
 *
 * @return @c true  si la lista tiene al menos un elemento y se calculó.
 * @return @c false si @p lista o @p resultado son NULL, o la lista está vacía.
 *
 * @note Complejidad temporal: O(n).
 */
bool lista_promedio(const Lista *lista, float *resultado) {
    long long suma = 0;
    int count = 0;
    Nodo *aux;

    if (lista == NULL || resultado == NULL) {
        return false;
    }

    aux = lista->cabeza;
    while (aux != NULL) {
        suma += aux->nro;
        count++;
        aux = aux->sgte;
    }

    if (count == 0) {
        return false;
    }

    *resultado = (float)suma / (float)count;
    return true;
}

/**
 * @brief Obtiene el valor mayor de la lista.
 *
 * @details
 * Inicializa el máximo con el valor de @c cabeza y recorre el resto de
 * los nodos comparando con el máximo actual. Actualiza @p *resultado
 * solo al finalizar el recorrido completo.
 *
 * @param[in]  lista     Puntero constante a la Lista.
 * @param[out] resultado Puntero donde se escribe el mayor valor encontrado.
 *
 * @return @c true  si la lista tiene al menos un elemento.
 * @return @c false si @p lista, @p resultado son NULL, o la lista está vacía.
 *
 * @note Complejidad temporal: O(n).
 */
bool lista_mayor(const Lista *lista, int *resultado) {
    int max;
    Nodo *aux;

    if (lista == NULL || resultado == NULL || lista->cabeza == NULL) {
        return false;
    }

    max = lista->cabeza->nro;
    aux = lista->cabeza->sgte;
    while (aux != NULL) {
        if (aux->nro > max) {
            max = aux->nro;
        }
        aux = aux->sgte;
    }

    *resultado = max;
    return true;
}

/**
 * @brief Verifica si la lista está ordenada de forma ascendente.
 *
 * @details
 * Recorre la lista comparando cada nodo con su sucesor. Si en algún
 * punto el valor de un nodo es mayor que el del siguiente, retorna
 * @c false inmediatamente. Una lista vacía o de un solo elemento se
 * considera ordenada (retorna @c true).
 *
 * @param[in] lista Puntero constante a la Lista.
 *
 * @return @c true  si todos los elementos están en orden no decreciente,
 *                  o si la lista tiene 0 ó 1 elementos.
 * @return @c false si hay al menos un par fuera de orden o @p lista es NULL.
 *
 * @note Complejidad temporal: O(n).
 */
bool lista_orden_asc(const Lista *lista) {
    Nodo *aux;
    if (lista == NULL || lista->cabeza == NULL || lista->cabeza->sgte == NULL) {
        return true;
    }

    aux = lista->cabeza;
    while (aux->sgte != NULL) {
        if (aux->nro > aux->sgte->nro) {
            return false;
        }
        aux = aux->sgte;
    }
    return true;
}

/**
 * @brief Indica si la lista no contiene ningún elemento.
 *
 * @param[in] lista Puntero constante a la Lista a consultar.
 *
 * @return @c true  si @p lista es NULL o @c cabeza es NULL.
 * @return @c false si la lista tiene al menos un elemento.
 *
 * @note Complejidad temporal: O(1).
 */
bool lista_vacia(const Lista *lista) {
    return lista == NULL || lista->cabeza == NULL;
}

/**
 * @brief Cuenta el número de nodos de la lista.
 *
 * @details
 * Recorre la lista desde @c cabeza hasta el último nodo, incrementando
 * un contador en cada paso.
 *
 * @param[in] lista Puntero constante a la Lista.
 *
 * @return Número de nodos (>= 0). Retorna 0 si @p lista es NULL.
 *
 * @note Complejidad temporal: O(n).
 */
int lista_contar(const Lista *lista) {
    int cantidad = 0;
    Nodo *aux;
    if (lista == NULL) {
        return 0;
    }

    aux = lista->cabeza;
    while (aux != NULL) {
        cantidad++;
        aux = aux->sgte;
    }
    return cantidad;
}

/**
 * @brief Copia los valores de la lista en un arreglo externo.
 *
 * @details
 * Recorre la lista desde @c cabeza copiando el valor de cada nodo en
 * @p destino. El índice 0 corresponde al primer nodo (cabeza). La copia
 * se detiene cuando se agota la lista o se alcanza @p capacidad.
 * La lista no se modifica.
 *
 * @param[in]  lista     Puntero constante a la Lista de origen.
 * @param[out] destino   Arreglo donde se almacenarán los valores copiados.
 * @param[in]  capacidad Número máximo de elementos que caben en @p destino.
 *
 * @return Número de elementos efectivamente copiados.
 *         Retorna 0 si @p lista, @p destino son NULL o @p capacidad <= 0.
 *
 * @note Complejidad temporal: O(min(n, capacidad)).
 */
int lista_copiar_valores(const Lista *lista, int *destino, int capacidad) {
    int usados = 0;
    Nodo *aux;

    if (lista == NULL || destino == NULL || capacidad <= 0) {
        return 0;
    }

    aux = lista->cabeza;
    while (aux != NULL && usados < capacidad) {
        destino[usados] = aux->nro;
        usados++;
        aux = aux->sgte;
    }
    return usados;
}

/**
 * @brief Genera una representación textual de la lista.
 *
 * @details
 * Escribe en @p destino una cadena donde cada elemento aparece como
 * @c "[pos]=valor" separados por @c " -> ". Ejemplo:
 * @code
 * [1]=10 -> [2]=20 -> [3]=30
 * @endcode
 * Si la lista está vacía escribe @c "Lista vacia".
 * La cadena siempre queda terminada en @c '\0' si @p capacidad >= 1.
 *
 * @param[in]  lista     Puntero constante a la Lista a representar.
 * @param[out] destino   Buffer de caracteres donde se escribirá el resultado.
 * @param[in]  capacidad Tamaño en bytes del buffer @p destino.
 *
 * @pre  @p destino debe apuntar a un buffer de al menos @p capacidad bytes.
 * @post @p destino contiene una cadena terminada en @c '\0'.
 * @note Si el contenido es más largo que @p capacidad, se trunca de
 *       forma segura (usa @c snprintf internamente).
 * @note Si @p destino es NULL o @p capacidad es 0, la función no hace nada.
 * @note Complejidad temporal: O(n).
 */
void lista_formatear(const Lista *lista, char *destino, size_t capacidad) {
    Nodo *aux;
    size_t usado = 0;
    int escritos;
    int pos = 1;

    if (destino == NULL || capacidad == 0) {
        return;
    }

    destino[0] = '\0';
    if (lista == NULL || lista->cabeza == NULL) {
        snprintf(destino, capacidad, "Lista vacia");
        return;
    }

    aux = lista->cabeza;
    while (aux != NULL && usado < capacidad) {
        escritos = snprintf(destino + usado, capacidad - usado, "[%d]=%d", pos, aux->nro);
        if (escritos < 0) {
            return;
        }
        usado += (size_t)escritos;

        aux = aux->sgte;
        pos++;
        if (aux != NULL && usado < capacidad) {
            escritos = snprintf(destino + usado, capacidad - usado, " -> ");
            if (escritos < 0) {
                return;
            }
            usado += (size_t)escritos;
        }
    }
}

/**
 * @brief Libera toda la memoria de los nodos de la lista.
 *
 * @details
 * Recorre la lista desde @c cabeza hasta el final, liberando cada nodo
 * con @c free(). Usa un puntero @c next para guardar el enlace antes de
 * liberar. Al finalizar, @c cabeza queda en NULL.
 *
 * @param[in,out] lista Puntero a la Lista a destruir.
 *                      Si es NULL la función no hace nada.
 *
 * @post La lista queda en el mismo estado que tras lista_inicializar().
 * @note Complejidad temporal: O(n).
 */
void lista_destruir(Lista *lista) {
    Nodo *aux;
    Nodo *next;
    if (lista == NULL) {
        return;
    }

    aux = lista->cabeza;
    while (aux != NULL) {
        next = aux->sgte;
        free(aux);
        aux = next;
    }
    lista->cabeza = NULL;
}
