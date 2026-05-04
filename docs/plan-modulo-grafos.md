# Plan: Modulo de Grafos y Navegacion

## Contexto
Proyecto: `C:\Users\fralm\Desktop\VisualStruct  Version 2`
Lenguaje: C estandar con Raylib.

## Objetivo
Integrar el modulo de grafos a la app unificada sin romper el flujo de estructuras secuenciales y conservando la capa pedagogica (codigo, traza y complejidad).

## Navegacion definida
- Portada principal con dos rutas: `Secuenciales` y `Grafos`.
- Submenu Secuenciales: Pila, Cola, Cola de Prioridad, Lista, Lista Circular, Sublistas.
- Submenu Grafos: Construccion + algoritmos (BFS, DFS, Dijkstra, Bellman-Ford, Prim, Kruskal).
- Ayuda global disponible con `F1`.

## Componentes del modulo Grafo
1. TAD: `include/grafo.h`, `src/grafo.c`.
2. Estado visual: `include/grafo_state.h`, `src/grafo_state.c`.
3. Layout: `include/grafo_layout.h`, `src/grafo_layout.c`.
4. Vista: `include/grafo_view.h`, `src/grafo_view.c`.
5. Controlador: `include/grafo_controller.h`, `src/grafo_controller.c`.
6. Pedagogia: `include/grafo_code_viewer.h`, `src/grafo_code_viewer.c`, `include/grafo_trace.h`, `src/grafo_trace.c`, `include/grafo_pedagogy.h`, `src/grafo_pedagogy.c`.

## Reglas tecnicas
- TAD de grafo sin `raylib.h`.
- UI sin acceso directo a internals del TAD.
- Toda operacion pasa por `app_state` y/o `grafo_controller`.
- Resultados dinamicos de algoritmos deben liberarse correctamente.

## Integracion en archivos existentes
- `include/app_state.h`: registrar `ESTRUCTURA_GRAFO` y estado asociado.
- `src/app_state.c`: agregar despacho de operaciones y sincronizacion visual.
- `src/main.c`: integrar menu, atajos, controles y paneles de grafos.
- `Makefile`: incluir todos los `src/grafo_*.c`.

## Validacion minima
1. Compila con `-std=c11 -Wall -Wextra -pedantic`.
2. Navegacion estable entre menu principal, secuenciales y grafos.
3. Operaciones CRUD de vertices/aristas correctas.
4. Algoritmos ejecutan y actualizan lienzo, codigo y traza.
5. QA manual y QA visual actualizados.

## Actualizacion de alcance UI (2026-05-04)
1. Priorizar experiencia de usuario final en pantalla de construccion de grafo.
2. Mantener un modo basico como vista por defecto y exponer modo avanzado bajo demanda.
3. Estandarizar tipografia y legibilidad de paneles, incluyendo etiquetas de vertices y pesos.
4. Asegurar que `Valor`, `Origen`, `Destino` y `Peso` se comporten como campos numericos.
5. Bloquear aplicacion de `Origen/Destino` cuando el vertice no exista en el grafo.
