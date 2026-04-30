# Analisis y Diseno: Modulo Sublistas

## 1. Objetivo de la funcionalidad
Incorporar una estructura jerarquica visualizable llamada `Sublistas` dentro del modulo de estructuras secuenciales, con modelo padre-hijo y comportamiento pedagogico coherente con el resto de la app.

La funcionalidad se disena como una estructura independiente de `Lista Circular`.

## 2. Alcance funcional
La estructura `Sublistas` debe permitir:

1. Inicializar
2. Insertar nodo padre
3. Seleccionar padre activo
4. Eliminar nodo padre
5. Insertar hijo en el padre activo
6. Eliminar hijo en el padre activo
7. Vaciar

Adicionalmente:

- visualizacion clara de agrupacion padre-hijos,
- resaltado del padre activo,
- integracion completa con botones, atajos, snippets, traza y ayuda interna.

## 3. Modelo de datos exigido
El diseno adopta exactamente el modelo solicitado:

```c
typedef struct sublista {
    int nro;
    struct sublista *sgte;
} Sublista;

typedef struct nodo {
    int nro;
    struct nodo *sgte;
    Sublista *sub;
} Nodo;
```

### Decisiones clave
- Cada `Nodo` padre mantiene su propia sublista a traves de `sub`.
- Las sublistas no comparten nodos entre padres.
- La seleccion de contexto recae en el `padre activo` almacenado en `AppState`.

## 4. Requerimientos funcionales
### RF-01: TAD desacoplado de Raylib
- API publica en `include/sublista.h`.
- Implementacion en `src/sublista.c` sin dependencias de UI.

### RF-02: Integracion de estado global
- Nuevo tipo `ESTRUCTURA_SUBLISTA`.
- Estado adicional para control de padre activo.
- Operaciones de hijo separadas de operaciones genericas.

### RF-03: Vista dedicada
- `src/sublista_view.c` renderiza padres, hijos, enlaces y seleccion activa.
- Soporta escenarios con overflow vertical.

### RF-04: Integracion pedagogica
- `src/code_viewer.c` incluye snippets por operacion de padres e hijos.
- `src/algorithm_trace.c` incorpora trazas y complejidades asociadas.

## 5. Complejidad esperada
- Inicializar: `O(1)`
- Insertar padre: `O(1)` o `O(n)` segun estrategia de insercion
- Buscar/seleccionar padre: `O(n)`
- Eliminar padre: `O(n)` (mas liberacion de su sublista)
- Insertar hijo: `O(1)` o `O(m)` segun insercion en sublista activa
- Eliminar hijo: `O(m)`
- Vaciar: `O(n + total_hijos)`

Donde:
- `n` = cantidad de padres
- `m` = cantidad de hijos de un padre concreto

## 6. Integracion de interfaz
- Home: sexta tarjeta `Sublistas`.
- Sidebar: boton dedicado `Sublistas`.
- Atajo directo: tecla `6`.
- `TAB`: en submenu secuencial rota entre seis estructuras; en visualizador global rota entre siete (incluyendo Grafo).
- Controles:
  - `I` Inicializar
  - `A` Padre +
  - `B` Seleccionar padre
  - `D` Padre -
  - `Z` Hijo +
  - `R` Hijo -
  - `V` Vaciar

## 7. Criterios de aceptacion
1. Compila con flags estrictos (`-Wall -Wextra -pedantic`).
2. Sublistas aparece en Home, Sidebar y atajo `6`.
3. Operaciones de padres e hijos actualizan estado, mensaje y paneles pedagogicos.
4. El padre activo cambia por busqueda y se refleja visualmente.
5. Eliminar padre libera correctamente sus hijos.
6. Ayuda interna y documentos QA quedan sincronizados.

## 8. Riesgos y mitigaciones
- Riesgo: confundir `Sublistas` con `Lista Circular`.
  - Mitigacion: mantener modulos separados, nombres explicitos y documentacion diferencial.
- Riesgo: fugas de memoria al eliminar padres con hijos.
  - Mitigacion: liberar sublista completa antes de destruir nodo padre.
- Riesgo: inconsistencias de controles por agregar operaciones de hijo.
  - Mitigacion: centralizar mapeo de atajos y etiquetas en `src/main.c`.

## 9. Plan minimo de pruebas
1. Insertar y eliminar padres sin hijos.
2. Insertar hijos en padres distintos y validar independencia.
3. Cambiar padre activo repetidamente y verificar que las operaciones de hijo actuen sobre el contexto correcto.
4. Eliminar padre con hijos y comprobar que no quedan referencias colgantes.
5. Verificar snippet, traza y complejidad para cada operacion.
