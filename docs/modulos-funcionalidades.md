# Modulos y Funcionalidades

## Resumen
VisualStruct integra 7 modulos funcionales en una sola app y sincroniza visualizacion, codigo, traza y complejidad.

## Matriz de modulos
| Modulo | Operaciones principales | Vista pedagogica |
|---|---|---|
| Pila | Inicializar, Push, Pop, Vaciar | Tope, estado LIFO, snippet y traza |
| Cola | Inicializar, Encolar, Desencolar, Vaciar | FRONT/BACK, estado FIFO |
| Cola Prioridad | Inicializar, Encolar(valor, prioridad), Desencolar | Orden por prioridad |
| Lista | Inicializar, Insertar inicio/final, Eliminar, Buscar, Invertir | HEAD/NULL, recorridos |
| Lista Circular | Inicializar, Insertar inicio/final, Eliminar, Buscar, Invertir | Cierre circular |
| Sublistas | Insertar/eliminar padre, seleccionar padre, insertar/eliminar hijo | Jerarquia padre-hijo |
| Grafo | Construccion, Recorridos, Caminos minimos, MST | Paso a paso, metricas y codigo |

## Detalle del modulo Grafo

### 1) Construccion
- Crear y reinicializar grafo.
- Insertar/eliminar vertices.
- Insertar/eliminar aristas con peso.
- Alternar dirigido/no dirigido.
- Cargar demo aleatoria configurable por numero de nodos.
- Panel lateral de entradas con espaciado fijo para validaciones:
  - evita superposicion de `Origen invalido` / `Destino invalido`.

### 2) Recorridos
- BFS y DFS.
- Vertice de inicio definido por el usuario.
- Lista de vertices recorridos visible durante la ejecucion.

### 3) Caminos minimos
- Dijkstra y Bellman-Ford.
- Uso explicito de pesos en aristas (definidos en Construccion o demo).
- Validacion de origen/destino.
- Resumen de ruta, costo y estados por paso.
- Sin boton `Aplicar peso a arista` en vista Caminos (flujo simplificado).
- Si no hay arista directa origen->destino se informa:
  - `Sin arista directa Vx->Vy`.

### 4) MST
- Prim (requiere inicio) y Kruskal.
- Modo basico simplificado para usuario final.
- Modo avanzado con control de pasos, autoplay y velocidad.
- Visual de cierre:
  - no se marca vertice `Final`
  - se mantiene arbol MST en lila
  - vertices vuelven a color base al terminar.

## Entradas numericas
En Grafo, los campos `Valor`, `Origen`, `Destino` y `Peso`:
- aceptan solo datos numericos
- muestran cursor visible al tener foco
- validan formato y existencia de vertices cuando aplica

## Demo aleatoria de grafos
`Cargar demo (M)`:
- toma `Valor` como cantidad de nodos (rango operativo 2..32)
- respeta modo dirigido/no dirigido actual
- garantiza conectividad base
- agrega aristas aleatorias dispersas para evitar grafos sobreconectados
- asigna pesos aleatorios

## Controles de navegacion
- `F1`: ayuda
- `TAB`: cambiar estructura
- `H` / `ESC`: volver al menu
- En grafo: `,` `.` `/` `Home` `End` `P` `O` `T` `M`

## Archivos clave por capa
- Dominio: `src/pila.c`, `src/cola.c`, `src/cola_prioridad.c`, `src/lista.c`, `src/lista_circular.c`, `src/sublista.c`, `src/grafo.c`
- Aplicacion: `src/app_state.c`, `src/grafo_controller.c`
- Presentacion: `src/main.c`, `src/ui.c`, `src/*_view.c`, `src/grafo_view.c`
- Pedagogia grafo: `src/grafo_code_viewer.c`, `src/grafo_trace.c`, `src/grafo_pedagogy.c`
