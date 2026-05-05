# Analisis y Diseno: Modulo Grafo

## 1. Objetivo
Ofrecer un modulo de grafos util para docencia, con flujo simple para usuario final y profundidad tecnica disponible en modo avanzado.

## 2. Alcance funcional actual
1. Construccion de grafo dirigido/no dirigido.
2. CRUD de vertices y aristas ponderadas.
3. Demos configurables por cantidad de nodos.
4. Recorridos BFS/DFS con inicio definido por usuario.
5. Caminos minimos con Dijkstra y Bellman-Ford.
6. MST con Prim y Kruskal.
7. Navegacion por pasos, autoplay y trazabilidad.

## 3. Arquitectura del modulo
### Dominio
- `src/grafo.c` / `include/grafo.h`
- Implementa estructuras y algoritmos.

### Aplicacion
- `src/app_state.c`: valida entradas, ejecuta operaciones y gestiona mensajes.
- `src/grafo_controller.c`: convierte resultados de algoritmo a pasos visuales.

### Presentacion
- `src/grafo_state.c`: snapshot visual.
- `src/grafo_layout.c`: posicionamiento de vertices.
- `src/grafo_view.c`: render de vertices, aristas, etiquetas y leyenda.
- `src/main.c`: UI de modos (Construccion, Recorridos, Caminos, MST).

### Pedagogia
- `src/grafo_code_viewer.c`, `src/grafo_trace.c`, `src/grafo_pedagogy.c`.

## 4. Decisiones UI/UX aplicadas
- Inputs de grafo numericos con cursor visible.
- Modo basico para simplificar pantalla.
- Modo avanzado para controles detallados.
- Vista MST basica con ejecucion directa de Prim/Kruskal.
- Demo aleatoria dispersa para evitar grafos sobreconectados.

## 5. Complejidades de referencia
- BFS: O(V + E)
- DFS: O(V + E)
- Dijkstra (sin heap): O(V^2 + E)
- Bellman-Ford: O(V * E)
- Prim (seleccion lineal): O(V^2 + E)
- Kruskal: O(E log E)

## 6. Criterios de aceptacion
1. Compilacion limpia con `-Wall -Wextra -pedantic`.
2. Entradas invalidas no alteran el estado del grafo.
3. Algoritmos sincronizan lienzo, panel de codigo y traza.
4. Demos responden al valor de nodos y mantienen variabilidad.
5. MST funciona en modo basico y avanzado.

## 7. Riesgos pendientes
- Prim/Kruskal usan indices por ID en arrays fijos: requiere refactor para IDs dispersos/grandes.
- Ajuste de densidad de demo puede requerir parametrizacion por perfil docente.

## 8. Siguiente mejora recomendada
- Introducir configurador de demo (densidad, rango de pesos, dirigido).
- Refactor interno de MST con mapeo de IDs a indices compactos.
