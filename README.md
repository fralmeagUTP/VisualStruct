# VisualStruct UTP: Estructuras Secuenciales y Grafos en C con Raylib

## Objetivo
Aplicacion de escritorio para apoyo docente en la Universidad Tecnologica de Pereira.
Visualiza estructuras secuenciales y grafos mostrando de forma sincronizada:
- representacion grafica
- operacion y estado
- codigo C asociado (historial)
- traza y complejidad

## Estado actual (2026-05-05)
- App unificada estable para 7 modulos: Pila, Cola, Cola de Prioridad, Lista, Lista Circular, Sublistas y Grafo.
- Modulo de grafos con 4 vistas: Construccion, Recorridos, Caminos y MST.
- Entradas de grafo numericas (`Valor`, `Origen`, `Destino`, `Peso`) con cursor visible y validacion.
- Demo de grafo configurable por cantidad de nodos (`Valor`) con aristas aleatorias dispersas.
- Panel de construccion simplificado para reducir redundancia visual:
  - resumen derecho compacto
  - panel inferior de estado reducido a una sola traza util
  - separacion vertical reforzada en inputs para evitar texto montado

## Requisitos
- GCC con soporte C11
- Raylib instalada
- Windows, Linux o macOS con librerias graficas compatibles

## Compilacion
### Opcion 1: Makefile
```bash
make
```

### Opcion 2: Comando directo (Windows/MinGW)
```bash
gcc -std=c11 -Wall -Wextra -pedantic -Iinclude \
  src/main.c src/ui.c src/app_state.c src/code_viewer.c src/algorithm_trace.c \
  src/pila_view.c src/cola_view.c src/cola_prioridad_view.c src/lista_view.c src/lista_circular_view.c src/sublista_view.c src/grafo_view.c \
  src/pila.c src/cola.c src/cola_prioridad.c src/lista.c src/lista_circular.c src/sublista.c \
  src/grafo.c src/grafo_state.c src/grafo_layout.c src/grafo_controller.c src/grafo_pedagogy.c src/grafo_code_viewer.c src/grafo_trace.c \
  -o visualstruct -lraylib -lopengl32 -lgdi32 -lwinmm
```

## Ejecucion
```bash
./visualstruct.exe
```

## Test E2E visual automatico
Ejecuta capturas automáticas por pantalla y valida reglas visuales minimas.

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run_visual_e2e.ps1 -ExePath .\visualstruct.exe -OutputDir .\artifacts\e2e_visual
```

Salida esperada:
- Capturas PNG en `artifacts/e2e_visual/` (home + grafos por modo/algoritmo).
- `manifest.txt` y `report.json`.
- Informe Markdown en `docs/informe-e2e-visual-YYYY-MM-DD.md`.

## Modulos y funcionalidades
### 1) Pila
- Inicializar, push, pop, vaciar.
- Visual LIFO con marca de tope.

### 2) Cola
- Inicializar, encolar, desencolar, vaciar.
- Visual FIFO con `FRONT` y `BACK`.

### 3) Cola de Prioridad
- Insercion con prioridad.
- Extraccion por prioridad minima.

### 4) Lista
- Insertar inicio/final, eliminar, buscar, invertir.

### 5) Lista Circular
- Insertar inicio/final, eliminar, buscar, invertir.
- Mantiene cierre circular en visualizacion.

### 6) Sublistas
- Gestion de nodos padre e hijos.
- Seleccion de padre activo y operaciones sobre su sublista.

### 7) Grafo
- Construccion: crear/eliminar vertices y aristas, dirigido/no dirigido.
- Recorridos: BFS y DFS con vertice de inicio definido por el usuario.
- Caminos minimos: Dijkstra y Bellman-Ford (con pesos existentes en aristas).
- MST: Prim y Kruskal (en no dirigido).
- Paso a paso, autoplay, traza pedagogica y resumen de estado.
- En Caminos, si no hay arista directa `origen->destino`, el resumen muestra:
  - `Sin arista directa Vx->Vy`

## Cambios recientes del modulo Grafo
- Legibilidad de texto mejorada (etiquetas y distancias) y ajuste de offsets.
- Correccion de resaltado de aristas en no dirigidos cuando la arista llega invertida.
- Vista MST simplificada en modo basico con ejecucion directa.
- Vista Caminos simplificada:
  - se elimino boton `Aplicar peso a arista`
  - botonera reorganizada en 6 acciones (algoritmo, ejecutar, paso-, paso+, reiniciar, auto)
- `Cargar demo` ahora:
  - usa `Valor` como cantidad de nodos
  - respeta modo dirigido/no dirigido
  - genera aristas y pesos aleatorios
  - evita densidad excesiva (grafo disperso)
- Visual de MST ajustada:
  - no marca vertice `Final` en Prim/Kruskal
  - durante pasos deja solo un vertice `Activo`
  - al final devuelve vertices a color base y conserva aristas MST en lila

## Controles importantes
- `F1`: abrir/cerrar ayuda.
- `H` o `ESC`: volver al menu principal desde visualizador.
- `TAB`: cambiar estructura activa.
- Grafo:
  - `I`: inicializar
  - `A` / `D`: vertice + / vertice -
  - `G` / `X`: arista + / arista -
  - `T`: dirigido/no dirigido
  - `M`: cargar demo aleatoria
  - `,` `.` `/`: paso - / paso + / reiniciar
  - `Home` / `End`: inicio / final
  - `P`: autoplay
  - `O`: velocidad autoplay

## Documentacion del proyecto
- `docs/architecture.md`: arquitectura por capas.
- `docs/modulos-funcionalidades.md`: matriz completa de modulos y flujos.
- `docs/guia-docente.md`: uso sugerido en clase.
- `docs/qa-manual.md`: checklist QA manual.
- `docs/informe-e2e-visual-YYYY-MM-DD.md`: resultado de pruebas E2E visuales automaticas.
- `docs/analisis-diseno-grafos.md`: diseno funcional y tecnico de grafos.
- `docs/plan-modulo-grafos.md`: plan de evolucion del modulo.
- `docs/bitacora-cambios-2026-05-04.md` y `docs/bitacora-cambios-2026-05-05.md`.

## Versionado
- `v0.0.1`: version historica inicial del repositorio.
- `v0.0.2`: mejoras de UI/UX y validaciones de grafos.
- `v0.0.3`: mejoras integrales de grafo (flujos, entradas numericas, MST y documentacion).
- `v0.0.4`: consolidacion visual de grafos (scroll, resumen pedagogico y flujo didactico).

## Estructura de carpetas
- `include/`: encabezados publicos.
- `src/`: implementacion activa.
- `src/legacy/`: referencias historicas no incluidas en build principal.
- `assets/`: logos y recursos.
- `docs/`: documentacion funcional y tecnica.
- `tests/`: pruebas de smoke/regresion.
