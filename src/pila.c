#include "pila.h"

#include <stdio.h>
#include <stdlib.h>

struct nodo {
    int nro;
    struct nodo *sgte;
};

/**
 * @brief Inicializa una Pila poniéndola en estado vacío.
 *
 * Establece @c tope a NULL. Debe ser la primera llamada antes de operar
 * sobre la Pila. No reserva memoria dinámica.
 *
 * @param[out] pila Puntero a la Pila que se va a inicializar.
 *                  Si es NULL la función no hace nada.
 *
 * @pre  @p pila apunta a un bloque de memoria válido.
 * @post La pila queda vacía y lista para usarse.
 */
void pila_inicializar(Pila *pila) {
    if (pila == NULL) {
        return;
    }
    pila->tope = NULL;
}

/**
 * @brief Inserta un valor entero en el tope de la pila (push).
 *
 * @details
 * Reserva dinámicamente un nuevo @c Nodo, almacena @p valor en él y
 * lo coloca encima del nodo actual apuntado por @c tope. El nuevo nodo
 * enlaza al antiguo tope a través de su puntero @c sgte.
 *
 * @param[in,out] pila  Puntero a la Pila destino.
 * @param[in]     valor Valor entero a insertar.
 *
 * @return @c true  si el nodo fue creado e insertado correctamente.
 * @return @c false si @p pila es NULL o @c malloc() falla.
 *
 * @pre  La Pila debe haber sido inicializada con pila_inicializar().
 * @post El tamaño de la pila aumenta en 1.
 * @note Complejidad temporal: O(1).
 */
bool pila_push(Pila *pila, int valor) {
    Nodo *aux;
    if (pila == NULL) {
        return false;
    }

    aux = (Nodo *)malloc(sizeof(Nodo));
    if (aux == NULL) {
        return false;
    }

    aux->nro = valor;
    aux->sgte = pila->tope;
    pila->tope = aux;
    return true;
}

/**
 * @brief Extrae el valor del tope de la pila (pop).
 *
 * @details
 * Elimina el nodo apuntado por @c tope, escribe su valor en @p *valor
 * y libera la memoria del nodo. El nuevo @c tope pasa a ser el nodo
 * que estaba inmediatamente debajo.
 *
 * @param[in,out] pila  Puntero a la Pila de origen.
 * @param[out]    valor Puntero donde se almacenará el valor extraído.
 *
 * @return @c true  si se extrajo un elemento correctamente.
 * @return @c false si @p pila o @p valor son NULL, o la pila está vacía.
 *
 * @pre  La Pila debe contener al menos un elemento.
 * @post El tamaño de la pila disminuye en 1.
 * @note Complejidad temporal: O(1).
 */
bool pila_pop(Pila *pila, int *valor) {
    Nodo *aux;
    if (pila == NULL || pila->tope == NULL || valor == NULL) {
        return false;
    }

    aux = pila->tope;
    *valor = aux->nro;
    pila->tope = aux->sgte;
    free(aux);
    return true;
}

/**
 * @brief Libera todos los nodos de la pila y deja @c tope en NULL.
 *
 * @details
 * Recorre la pila desde el tope hasta el fondo llamando a @c free()
 * en cada nodo. Al finalizar, @c tope queda en NULL. Equivalente a
 * llamar pila_pop() repetidamente hasta vaciar, pero más eficiente.
 *
 * @param[in,out] pila Puntero a la Pila a destruir.
 *                     Si es NULL la función no hace nada.
 *
 * @post La pila queda en el mismo estado que tras pila_inicializar().
 * @note Complejidad temporal: O(n), donde n es el número de elementos.
 */
void pila_destruir(Pila *pila) {
    Nodo *aux;
    if (pila == NULL) {
        return;
    }

    while (pila->tope != NULL) {
        aux = pila->tope;
        pila->tope = aux->sgte;
        free(aux);
    }
}

/**
 * @brief Cuenta el número de elementos presentes en la pila.
 *
 * @details
 * Recorre la lista enlazada desde @c tope hasta el último nodo,
 * incrementando un contador en cada paso.
 *
 * @param[in] pila Puntero constante a la Pila a consultar.
 *
 * @return Número de nodos en la pila (>= 0).
 *         Retorna 0 si @p pila es NULL.
 *
 * @note Complejidad temporal: O(n), donde n es el número de elementos.
 */
int pila_contar(const Pila *pila) {
    int cantidad = 0;
    Nodo *actual;
    if (pila == NULL) {
        return 0;
    }

    actual = pila->tope;
    while (actual != NULL) {
        cantidad++;
        actual = actual->sgte;
    }
    return cantidad;
}

/**
 * @brief Indica si la pila no contiene ningún elemento.
 *
 * @param[in] pila Puntero constante a la Pila a consultar.
 *
 * @return @c true  si @p pila es NULL o su puntero @c tope es NULL.
 * @return @c false si la pila tiene al menos un elemento.
 *
 * @note Complejidad temporal: O(1).
 */
bool pila_vacia(const Pila *pila) {
    return pila == NULL || pila->tope == NULL;
}

/**
 * @brief Copia los valores de la pila en un arreglo externo.
 *
 * @details
 * Recorre la pila desde el tope copiando cada valor entero en @p destino.
 * El índice 0 del arreglo corresponde al elemento del tope (el último
 * insertado). La copia se detiene cuando se agota la pila o se alcanza
 * @p capacidad. La pila no se modifica.
 *
 * @param[in]  pila      Puntero constante a la Pila de origen.
 * @param[out] destino   Arreglo donde se almacenarán los valores copiados.
 * @param[in]  capacidad Número máximo de elementos que caben en @p destino.
 *
 * @return Número de elementos efectivamente copiados (0 a @p capacidad).
 *         Retorna 0 si @p pila, @p destino son NULL o @p capacidad <= 0.
 *
 * @pre  @p destino debe apuntar a un bloque con espacio para al menos
 *       @p capacidad enteros.
 * @note Complejidad temporal: O(min(n, capacidad)).
 */
int pila_copiar_valores(const Pila *pila, int *destino, int capacidad) {
    int usados = 0;
    Nodo *actual;

    if (pila == NULL || destino == NULL || capacidad <= 0) {
        return 0;
    }

    actual = pila->tope;
    while (actual != NULL && usados < capacidad) {
        destino[usados] = actual->nro;
        usados++;
        actual = actual->sgte;
    }

    return usados;
}

/**
 * @brief Genera una representación textual de la pila.
 *
 * @details
 * Escribe en @p destino una cadena con el formato:
 * @code
 * Tope -> v1 | v2 | ... | vN
 * @endcode
 * donde @c v1 es el tope y @c vN el elemento del fondo.
 * Si la pila está vacía escribe @c "Pila vacia".
 * La cadena siempre queda terminada en @c '\0' si @p capacidad >= 1.
 *
 * @param[in]  pila      Puntero constante a la Pila a representar.
 * @param[out] destino   Buffer de caracteres donde se escribirá el resultado.
 * @param[in]  capacidad Tamaño en bytes del buffer @p destino.
 *
 * @pre  @p destino debe apuntar a un buffer de al menos @p capacidad bytes.
 * @post @p destino contiene una cadena terminada en @c '\0'.
 * @note Si el contenido es más largo que @p capacidad, se trunca de
 *       forma segura (usa @c snprintf internamente).
 * @note Si @p destino es NULL o @p capacidad es 0, la función no hace nada.
 * @note Complejidad temporal: O(n), donde n es el número de elementos.
 */
void pila_formatear(const Pila *pila, char *destino, size_t capacidad) {
    Nodo *actual;
    size_t usado = 0;
    int escritos;

    if (destino == NULL || capacidad == 0) {
        return;
    }

    destino[0] = '\0';

    if (pila == NULL || pila->tope == NULL) {
        snprintf(destino, capacidad, "Pila vacia");
        return;
    }

    actual = pila->tope;
    escritos = snprintf(destino, capacidad, "Tope -> ");
    if (escritos < 0) {
        return;
    }
    usado = (size_t)escritos;

    while (actual != NULL && usado < capacidad) {
        escritos = snprintf(destino + usado, capacidad - usado, "%d", actual->nro);
        if (escritos < 0) {
            return;
        }
        usado += (size_t)escritos;

        actual = actual->sgte;
        if (actual != NULL && usado < capacidad) {
            escritos = snprintf(destino + usado, capacidad - usado, " | ");
            if (escritos < 0) {
                return;
            }
            usado += (size_t)escritos;
        }
    }
}
