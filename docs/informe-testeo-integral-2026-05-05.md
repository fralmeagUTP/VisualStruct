# Informe de Testeo Integral - 2026-05-05

## 1) Objetivo
Verificar que la aplicacion cumple las funcionalidades documentadas en:
- `README.md`
- `docs/modulos-funcionalidades.md`
- `docs/qa-manual.md`
- `docs/analisis-diseno-grafos.md`

## 2) Alcance
Se evaluo:
1. Compilacion del proyecto.
2. Capa de dominio (TADs secuenciales y grafo).
3. Funcionalidad de demos de grafo (cantidad de nodos + aleatoriedad de aristas).
4. Presencia de funcionalidades UI/flujo por inspeccion de codigo.

No se incluyo automatizacion de clicks/escenarios visuales complejos (se marca como validacion manual pendiente).

## 3) Entorno de prueba
- SO: Windows
- Compilador: GCC (C11)
- Build flags: `-std=c11 -Wall -Wextra -pedantic`
- Fecha de ejecucion: 2026-05-05

## 4) Pruebas ejecutadas

### 4.1 Build principal
- Compilacion de binario completo: `visualstruct_test_build.exe`
- Resultado: **PASS**
- Evidencia: build completo sin errores.

### 4.2 Smoke test secuenciales
- Binario: `secuenciales_smoke_test.exe`
- Archivo: `tests/secuenciales_smoke_test.c`
- Cobertura:
  - Pila
  - Cola
  - Cola de Prioridad
  - Lista
  - Lista Circular
  - Sublistas
- Resultado: **PASS**
- Casos `PASS`: **51**

### 4.3 Smoke test grafo (algoritmos)
- Binario: `grafo_smoke_test.exe`
- Archivo: `tests/grafo_smoke_test.c`
- Cobertura:
  - CRUD basico de vertices/aristas
  - BFS / DFS
  - Dijkstra / Bellman-Ford
  - Prim / Kruskal
  - Prim / Kruskal con IDs dispersos (10, 42, 777, 5000)
  - Casos de error (pesos negativos, ciclo negativo, MST en dirigido)
- Resultado: **PASS**
- Casos `PASS`: **64**

### 4.4 Smoke test demo configurable y aleatoria
- Binario: `grafo_demo_smoke_test.exe`
- Archivo: `tests/grafo_demo_smoke_test.c`
- Cobertura:
  - `Cargar demo` respeta cantidad de nodos (`Valor`)
  - Se garantiza conectividad base (`aristas >= n-1`)
  - Hay variacion de aristas entre cargas consecutivas
- Resultado: **PASS**
- Casos `PASS`: **3**

## 5) Matriz de cumplimiento contra documentos

| Requisito documentado | Fuente | Estado | Evidencia |
|---|---|---|---|
| 7 modulos funcionales en app unificada | README / modulos-funcionalidades | PASS | Smoke secuenciales + grafo |
| BFS/DFS operativos | README / analisis-grafos | PASS | `grafo_smoke_test` |
| Dijkstra/Bellman-Ford con pesos | README / analisis-grafos | PASS | `grafo_smoke_test` |
| Prim/Kruskal en no dirigido | README / analisis-grafos | PASS | `grafo_smoke_test` |
| Rechazo de escenarios invalidos (p. ej. Dijkstra con negativos) | qa-manual / analisis-grafos | PASS | `grafo_smoke_test` |
| Demo configurable por cantidad de nodos | README / modulos-funcionalidades | PASS | `grafo_demo_smoke_test` |
| Demo aleatoria de aristas (dispersa) | README / modulos-funcionalidades | PASS | `grafo_demo_smoke_test` + inspeccion en `src/app_state.c` |
| Controles UI de grafo (modos, teclas, flujo MST basico) | README / qa-manual | PASS (inspeccion) | `src/main.c` |
| Inputs numericos de grafo (valor/origen/destino/peso) | README / qa-manual | PASS (inspeccion) | `src/main.c` (`input_focus`, validaciones) |
| Render/legibilidad detallada en todos los escenarios visuales | qa-manual | PENDIENTE MANUAL | Requiere recorrido visual interactivo completo |

## 6) Hallazgos

### Hallazgos criticos
- **Ninguno** en pruebas automatizadas.

### Hallazgos no criticos / riesgos
1. Aun falta automatizacion E2E de UI (click/teclado + aserciones visuales).
2. Riesgo tecnico de Prim/Kruskal con IDs de vertices dispersos: **RESUELTO**.
   - Evidencia: caso agregado en `tests/grafo_smoke_test.c` y ejecucion `PASS`.
   - Mitigacion aplicada: `src/grafo.c` usa indices compactos dinamicos para Prim/Kruskal (sin indexar por ID directo).

## 7) Conclusion
La app **cumple funcionalmente** con lo documentado en nivel de dominio y logica principal, incluyendo el nuevo flujo de demo configurable/aleatoria en grafos.

Cobertura automatizada total ejecutada en esta ronda:
- **118 casos PASS** (51 secuenciales + 64 grafo + 3 demo)
- **0 casos FAIL**

## 8) Recomendaciones siguientes
1. Ejecutar checklist visual por pantalla con la matriz `docs/qa-checklist-pantallas-2026-05-05.md`.
2. Agregar pruebas automatizadas para `app_state` (flujo UI-lógico).
3. Mantener este informe por fecha para trazabilidad de regresiones.
