# Analisis y Diseno: Modulo Grafo

## 1. Objetivo
Integrar un modulo de grafos en VisualStruct con enfoque docente, manteniendo separacion por capas y consistencia con los paneles pedagogicos de la aplicacion.

## 2. Alcance funcional
El modulo permite:

1. Crear y reinicializar grafo.
2. Insertar y eliminar vertices.
3. Insertar y eliminar aristas con peso.
4. Alternar entre modo dirigido y no dirigido.
5. Cargar escenarios demo.
6. Ejecutar BFS, DFS, Dijkstra, Bellman-Ford, Prim y Kruskal.
7. Navegar la ejecucion por pasos, autoplay y exportacion de resumen.

## 3. Estructura tecnica por capas
### Dominio
- `include/grafo.h` y `src/grafo.c`.
- TAD desacoplado de Raylib y expuesto por API publica.

### Aplicacion y control
- `src/app_state.c` coordina operaciones de alto nivel para `ESTRUCTURA_GRAFO`.
- `src/grafo_controller.c` gestiona validaciones, ejecucion de algoritmos y estado de pasos.

### Presentacion
- `src/grafo_view.c` dibuja vertices, aristas y estados visuales.
- `src/grafo_state.c` y `src/grafo_layout.c` mantienen snapshot y layout del lienzo.
- `src/main.c` integra menu, controles y rutas de navegacion.

### Pedagogia
- `src/grafo_code_viewer.c`: snippet del algoritmo activo.
- `src/grafo_trace.c`: traza textual por algoritmo.
- `src/grafo_pedagogy.c`: metricas por paso, camino parcial, tabla de distancias y exportacion.

## 4. Requerimientos de interfaz
- Portada principal con dos entradas: `Secuenciales` y `Grafos`.
- Submenu de grafos con acceso a construccion y algoritmos.
- Campos laterales para `origen`, `destino` y `peso`.
- Panel inferior enriquecido con tipo de paso, progreso y metricas.

## 5. Complejidades esperadas (referencia docente)
- BFS: `O(V + E)`.
- DFS: `O(V + E)`.
- Dijkstra (implementacion base sin heap): `O(V^2 + E)`.
- Bellman-Ford: `O(V * E)`.
- Prim (seleccion lineal): `O(V^2 + E)`.
- Kruskal: `O(E log E)` por ordenamiento de aristas.

## 6. Criterios de aceptacion
1. La app compila con `-Wall -Wextra -pedantic`.
2. El menu permite entrar a grafos y volver sin perder estabilidad.
3. Las operaciones de vertices/aristas reflejan estado valido e invalido.
4. Cada algoritmo actualiza vista, codigo y traza de forma sincronizada.
5. La navegacion por pasos y autoplay mantiene coherencia visual.
6. La exportacion textual entrega resumen util para clase.

## 7. Riesgos y mitigaciones
- Riesgo: acoplar logica de algoritmo con UI.
  - Mitigacion: mantener TAD y controlador sin dependencias de render.
- Riesgo: inconsistencias entre paneles pedagogicos.
  - Mitigacion: centralizar metricas y trazas en `grafo_pedagogy.c` y `grafo_trace.c`.
- Riesgo: sobrecarga visual en resoluciones bajas.
  - Mitigacion: validar con `docs/qa-visualizacion-ventanas.md` y modo compacto.
