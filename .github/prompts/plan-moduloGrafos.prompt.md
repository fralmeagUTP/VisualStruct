# Plan: Módulo de Grafos + Reestructuración de Navegación

## Contexto
Proyecto: `C:\Users\fralm\Desktop\VisualStruct  Version 2`  
Lenguaje: C estándar — Interfaz gráfica: raylib.h  
Última compilación exitosa: visualstruct_buildcheck42  
Tipografía unificada: Tahoma (validada)

## Decisiones confirmadas

- **Menú principal**: 2 opciones — "Estructuras Secuenciales" y "Grafos"
  - Submenú Secuenciales: pila, cola, cola_prioridad, lista, lista_circular, sublista (existente, sin cambios)
  - Submenú Grafos: construcción del grafo + BFS, DFS, Dijkstra, Bellman-Ford, Prim, Kruskal
- **Input grafos**: 3 campos separados (origen, destino, peso) para aristas
- **Enum**: `ESTRUCTURA_GRAFO = 6`, sin renumerar los existentes (0–5)

---

## Navegación rediseñada

```
PANTALLA_HOME  (menú raíz — 2 cards)
├── PANTALLA_MENU_SECUENCIALES   ← lo que hoy es HOME con las 6 estructuras
│   └── → PANTALLA_VISUALIZER   (sin cambios internos)
└── PANTALLA_MENU_GRAFOS         ← nuevo
    ├── PANTALLA_GRAFO_CONSTRUCCION
    ├── PANTALLA_GRAFO_BFS
    ├── PANTALLA_GRAFO_DFS
    ├── PANTALLA_GRAFO_DIJKSTRA
    ├── PANTALLA_GRAFO_BELLMAN_FORD
    ├── PANTALLA_GRAFO_PRIM
    └── PANTALLA_GRAFO_KRUSKAL
```

---

## Fases de implementación

### Fase 1 — TAD Grafo puro (sin raylib)

**Crear `include/grafo.h`**
- Tipo opaco: `typedef struct Grafo Grafo;`
- `GrafoEstado` enum (7 valores: OK, NULO, MEMORIA, NO_EXISTE, YA_EXISTE, PESO_NEGATIVO, CICLO_NEGATIVO)
- Structs: `GrafoArista { int origen; int destino; int peso; }`
- `GrafoRecorrido { int *vertices; size_t cantidad; GrafoEstado estado; }`
- `GrafoCamino { GrafoArista *aristas; size_t cantidad; int costo_total; bool existe; GrafoEstado estado; }`
- API completa (ver §11 del SDD):
  - Creación/destrucción: `grafo_crear`, `grafo_destruir`, `grafo_es_dirigido`
  - Estructurales: `grafo_insertar_vertice`, `grafo_insertar_arista`, `grafo_eliminar_vertice`, `grafo_eliminar_arista`
  - Consultas: `grafo_existe_vertice`, `grafo_existe_arista`, `grafo_obtener_peso`, `grafo_orden`, `grafo_tamano`, `grafo_grado_salida`, `grafo_grado_entrada`
  - Copias públicas: `grafo_obtener_vertices`, `grafo_obtener_aristas`, `grafo_sucesores`, `grafo_predecesores`
  - Algoritmos: `grafo_bfs`, `grafo_dfs`, `grafo_dijkstra`, `grafo_bellman_ford`, `grafo_prim`, `grafo_kruskal`
  - Liberación: `grafo_liberar_recorrido`, `grafo_liberar_camino`, `grafo_estado_cadena`

**Crear `src/grafo.c`**
- Representación interna: lista enlazada de `NodoVertice`, cada uno con lista enlazada de `NodoArista`
- Grafos no dirigidos: cada arista se almacena en ambas listas de adyacencia
- Validar RT-08 del SDD: punteros nulos, vértices inexistentes, aristas inexistentes, errores de memoria
- Dijkstra: selección lineal del mínimo — O(V² + E·V) — sin heap (suficiente para grafos académicos pequeños)
- Kruskal: ordenamiento por peso + Union-Find (find con path compression, union por rango)
- Prim: selección lineal de clave mínima — O(V² + E·V)
- Todo `malloc` con su `free` correspondiente; liberar con `grafo_liberar_recorrido` / `grafo_liberar_camino`

---

### Fase 2 — Estado visual y Layout (pasos paralelos)

**Crear `include/grafo_state.h` y `src/grafo_state.c`**
```c
typedef enum {
    GRAFO_ALG_NINGUNO = 0,
    GRAFO_ALG_BFS, GRAFO_ALG_DFS,
    GRAFO_ALG_DIJKSTRA, GRAFO_ALG_BELLMAN_FORD,
    GRAFO_ALG_PRIM, GRAFO_ALG_KRUSKAL
} GrafoAlgoritmoActivo;

typedef struct {
    int vertice;
    Vector2 posicion;
    bool resaltado;
    bool visitado;
} GrafoVerticeVisual;

typedef struct {
    int origen; int destino; int peso;
    bool resaltada; bool relajada;
} GrafoAristaVisual;

typedef struct {
    Grafo *grafo;
    bool dirigido;
    GrafoAlgoritmoActivo algoritmo_activo;
    int origen_actual;
    int destino_actual;
    int peso_actual;
    char mensaje[256];
    char complejidad[256];
    char codigo_actual[4096];
    // Arrays de estado visual (max 64 vértices)
    GrafoVerticeVisual vertices_vis[64];
    GrafoAristaVisual aristas_vis[256];
    int n_vertices_vis;
    int n_aristas_vis;
} GrafoState;
```
- `grafo_state_init(GrafoState *state)`
- `grafo_state_destruir(GrafoState *state)` — llama `grafo_destruir`
- `grafo_state_actualizar_visual(GrafoState *state)` — sincroniza arrays vis con el TAD

**Crear `include/grafo_layout.h` y `src/grafo_layout.c`**
- `grafo_layout_calcular_circular(GrafoVerticeVisual *vis, int n, Rectangle panel)`
- Distribuye N vértices en arco alrededor del centro del panel
- Radio adaptado al tamaño del panel y cantidad de vértices

---

### Fase 3 — Vista gráfica raylib

**Crear `include/grafo_view.h` y `src/grafo_view.c`**
- `grafo_view_draw(const GrafoState *state, Rectangle bounds)`
- Vértices como círculos rellenos con etiqueta del entero; 5 estados visuales:
  - Normal → azul/gris
  - Visitado → verde resaltado
  - Inicial → borde grueso verde
  - Destino → borde doble amarillo
  - Actual → color naranja temporal
- Aristas como líneas (no dirigido) o flechas (dirigido); 5 estados:
  - Normal → gris
  - Relajada → amarillo temporal
  - Camino mínimo → verde destacado
  - MST → azul grueso
  - Error/ciclo negativo → rojo
- Peso de la arista centrado sobre la línea
- No incluir raylib.h en grafo_state.h ni grafo.h

---

### Fase 4 — Controlador

**Crear `include/grafo_controller.h` y `src/grafo_controller.c`**
- `grafo_controller_init(GrafoState *state)`
- `grafo_controller_crear(GrafoState *state, bool dirigido)` → destruye grafo anterior si existe
- `grafo_controller_insertar_vertice(GrafoState *state, int v)`
- `grafo_controller_insertar_arista(GrafoState *state, int origen, int destino, int peso)`
- `grafo_controller_eliminar_vertice(GrafoState *state, int v)`
- `grafo_controller_eliminar_arista(GrafoState *state, int origen, int destino)`
- `grafo_controller_ejecutar_bfs(GrafoState *state, int inicio)`
- `grafo_controller_ejecutar_dfs(GrafoState *state, int inicio)`
- `grafo_controller_ejecutar_dijkstra(GrafoState *state, int origen, int destino)`
- `grafo_controller_ejecutar_bellman_ford(GrafoState *state, int origen, int destino)`
- `grafo_controller_ejecutar_prim(GrafoState *state, int inicio)`
- `grafo_controller_ejecutar_kruskal(GrafoState *state)`
- `grafo_controller_limpiar(GrafoState *state)` → destruye y recrea grafo vacío
- Cada función que retorna resultados dinámicos (recorridos, caminos) los libera con `grafo_liberar_recorrido` / `grafo_liberar_camino` antes de retornar

---

### Fase 5 — Capa didáctica (paralelo con Fase 4)

**Crear `include/grafo_code_viewer.h` y `src/grafo_code_viewer.c`**
```c
typedef struct {
    GrafoAlgoritmoActivo algoritmo;
    const char *titulo;
    const char *codigo;
    const char *explicacion;
    const char *complejidad;
} GrafoSnippet;
```
- `grafo_code_viewer_get(GrafoAlgoritmoActivo alg)` → retorna puntero a `GrafoSnippet`
- Fragmentos C para: NINGUNO/construcción, BFS, DFS, Dijkstra, Bellman-Ford, Prim, Kruskal (§16 SDD)

**Crear `include/grafo_trace.h` y `src/grafo_trace.c`**
- `grafo_trace_get_pasos(GrafoAlgoritmoActivo alg)` → retorna array de strings con 7 pasos
- Trazas según §17 del SDD: BFS (7 pasos), Dijkstra (7 pasos), Kruskal (7 pasos)
- Mismos pasos para DFS, Bellman-Ford, Prim

---

### Fase 6 — Integración (modifica 3 archivos existentes)

**Modificar `include/app_state.h`**
- Agregar `ESTRUCTURA_GRAFO = 6` al final del enum `TipoEstructura`
- Agregar campo `GrafoState grafo_state;` al struct `AppState`
- Agregar include de `grafo_state.h`

**Modificar `src/app_state.c`**
- En `app_state_init()`: llamar `grafo_state_init(&estado->grafo_state)`
- En `app_state_destroy()`: llamar `grafo_state_destruir(&estado->grafo_state)`
- En `app_state_update_visuals()`: agregar `grafo_state_actualizar_visual` si activo
- En cada `switch(estado->estructura_activa)` existente: agregar `case ESTRUCTURA_GRAFO:` con comportamiento apropiado (o `break` vacío donde no aplique)

**Modificar `src/main.c`**

Reestructuración de pantallas:
```c
typedef enum {
    PANTALLA_HOME,               // menú raíz (2 cards)
    PANTALLA_MENU_SECUENCIALES,  // submenú con 6 estructuras (existente)
    PANTALLA_VISUALIZER,         // visualizador secuencial (sin cambios)
    PANTALLA_MENU_GRAFOS,        // submenú grafos
    PANTALLA_GRAFO_CONSTRUCCION, // insertar/eliminar vértices y aristas
    PANTALLA_GRAFO_BFS,
    PANTALLA_GRAFO_DFS,
    PANTALLA_GRAFO_DIJKSTRA,
    PANTALLA_GRAFO_BELLMAN_FORD,
    PANTALLA_GRAFO_PRIM,
    PANTALLA_GRAFO_KRUSKAL,
    PANTALLA_AYUDA
} TipoPantalla;
```

Nuevos campos de input para grafos (locales en main o en AppState):
```c
char input_origen[8];   // campo 1
char input_destino[8];  // campo 2
char input_peso[8];     // campo 3
int  input_campo_activo; // 0=origen, 1=destino, 2=peso
```

Pantalla HOME (menú raíz) — 2 cards grandes:
- "Estructuras Secuenciales" → `PANTALLA_MENU_SECUENCIALES`
- "Grafos" → `PANTALLA_MENU_GRAFOS`

Pantalla MENU_SECUENCIALES — lo que hoy hace HOME:
- 6 cards (pila, cola, cola_prioridad, lista, lista_circular, sublista)
- Botón "Volver" → `PANTALLA_HOME`
- Navegación numérica 1–6 igual que hoy

Pantalla MENU_GRAFOS — nuevo submenú:
- Card "Construcción del grafo" (crear/insertar/eliminar)
- Cards por algoritmo: BFS, DFS, Dijkstra, Bellman-Ford, Prim, Kruskal
- Botón "Volver" → `PANTALLA_HOME`

Pantallas de grafo (PANTALLA_GRAFO_*):
- Panel central: `grafo_view_draw()`
- Panel derecho: `grafo_code_viewer_get()` con el snippet correspondiente
- Panel inferior: `grafo_trace_get_pasos()` con la traza del algoritmo activo
- Sidebar izquierdo: botones de operación + 3 campos de input (origen/destino/peso)
- Botón "Volver" → `PANTALLA_MENU_GRAFOS`

---

### Fase 7 — Compilación y pruebas

**Comando de compilación:**
```
gcc -std=c11 -Wall -Wextra -pedantic -Iinclude \
    src/main.c src/ui.c src/app_state.c \
    src/algorithm_trace.c src/code_viewer.c \
    src/pila.c src/cola.c src/cola_prioridad.c \
    src/lista.c src/lista_circular.c src/sublista.c \
    src/pila_view.c src/cola_view.c src/cola_prioridad_view.c \
    src/lista_view.c src/lista_circular_view.c src/sublista_view.c \
    src/grafo.c src/grafo_state.c src/grafo_layout.c \
    src/grafo_view.c src/grafo_controller.c \
    src/grafo_code_viewer.c src/grafo_trace.c \
    -o visualstruct_grafos.exe \
    -lraylib -lopengl32 -lgdi32 -lwinmm
```

**Pruebas PT-01 a PT-20 del SDD §21:**
- PT-01: Crear grafo dirigido
- PT-02: Insertar vértice 10
- PT-03: Insertar arista 10→20 peso 5
- PT-04: Arista con vértice inexistente → `GRAFO_ERROR_NO_EXISTE`
- PT-05: Vértice duplicado → `GRAFO_ERROR_YA_EXISTE`
- PT-06: Eliminar vértice (y aristas incidentes)
- PT-07: Eliminar arista
- PT-08: BFS desde inicio 1
- PT-09: DFS desde inicio 1
- PT-10: Dijkstra origen 1 → destino 5
- PT-11: Dijkstra con peso negativo → error
- PT-12: Bellman-Ford con pesos negativos sin ciclo
- PT-13: Bellman-Ford con ciclo negativo → error
- PT-14: Prim en grafo no dirigido
- PT-15: Prim en grafo dirigido → error
- PT-16: Kruskal en grafo no dirigido
- PT-17: Kruskal en grafo desconectado → aviso
- PT-18: Ver código BFS
- PT-19: Ver explicación Dijkstra
- PT-20: Cierre y liberación de memoria

---

## Archivos a crear (14 nuevos)

| Archivo | Contenido |
|---|---|
| `include/grafo.h` | API pública del TAD (tipo opaco) |
| `src/grafo.c` | Lista de adyacencia + 6 algoritmos |
| `include/grafo_state.h` | Estado visual + structs visuales |
| `src/grafo_state.c` | Init/destrucción/sincronización |
| `include/grafo_layout.h` | Cálculo de posiciones circulares |
| `src/grafo_layout.c` | Layout circular adaptativo |
| `include/grafo_view.h` | Interfaz de renderizado |
| `src/grafo_view.c` | Dibujo con Raylib |
| `include/grafo_controller.h` | Interfaz del controlador |
| `src/grafo_controller.c` | Lógica de operaciones y algoritmos |
| `include/grafo_code_viewer.h` | Snippets C por algoritmo |
| `src/grafo_code_viewer.c` | Tabla de fragmentos pedagógicos |
| `include/grafo_trace.h` | Trazas pedagógicas |
| `src/grafo_trace.c` | 7 pasos por algoritmo |

## Archivos a modificar (3)

| Archivo | Cambio |
|---|---|
| `include/app_state.h` | + `ESTRUCTURA_GRAFO = 6`, + `GrafoState grafo_state` |
| `src/app_state.c` | + ramas `ESTRUCTURA_GRAFO` en los switch existentes |
| `src/main.c` | Reestructurar navegación + pantallas de grafos + 3 campos input |

## No se toca

- Todos los TADs existentes: `pila`, `cola`, `cola_prioridad`, `lista`, `lista_circular`, `sublista`
- Todas las vistas: `*_view.c`
- `src/ui.c`, `include/ui.h`
- `src/algorithm_trace.c`, `src/code_viewer.c`

---

## Restricciones obligatorias (RT del SDD)

- RT-01: Núcleo lógico en C estándar
- RT-03: `grafo.h` / `grafo.c` sin `raylib.h`
- RT-04: Solo API pública expuesta en `grafo.h`
- RT-06: Módulo gráfico accede al grafo solo mediante funciones públicas
- RT-07: Doxygen en todo módulo, estructura y función pública
- RT-08: Validar punteros nulos, vértices/aristas inexistentes, errores de memoria
- RT-09: Liberar resultados dinámicos con `grafo_liberar_recorrido` y `grafo_liberar_camino`
