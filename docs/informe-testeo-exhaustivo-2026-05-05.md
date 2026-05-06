# Informe de Testeo Exhaustivo (Teorico + Practico) - 2026-05-05

## 1) Objetivo
Evaluar la aplicacion con enfoque de testeo exhaustivo: cubrir combinaciones de entradas, precondiciones y caminos de ejecucion, y reportar alcance real, resultados y limites.

## 2) Nota metodologica clave
El testeo exhaustivo total en una app interactiva como esta no es computacionalmente factible.
Por tanto, se aplico una estrategia equivalente:
- Exhaustivo acotado por particiones y fronteras.
- Cobertura automatizada de logica central (smoke/regresion).
- Verificacion de escenarios invalidos criticos.
- Trazabilidad contra requerimientos documentados.

## 3) Espacio teorico de combinaciones (resumen)

### 3.1 Modulo de grafos
- Nodos demo: `n` en `[2, 32]`.
- Tipo de grafo: `dirigido/no dirigido` (2 opciones).
- Peso por arista en entradas manuales: `[-999, 999]` (1999 valores).

Solo para grafo no dirigido simple con `n=32`:
- Pares posibles de vertices: `32*31/2 = 496`.
- Cada par puede estar ausente o tener un peso: `1 + 1999 = 2000` estados.
- Combinaciones: `2000^496` (astronomico).

Ademas se combinan:
- Algoritmos: BFS, DFS, Dijkstra, Bellman-Ford, Prim, Kruskal.
- Modos/pasos (manual, auto, siguiente/anterior, reinicio).
- Validaciones UI (inicio, origen, destino, peso, estados de error).

Conclusión: cobertura total teorica es inabordable; se valida por cobertura estructurada de alto riesgo.

## 4) Pruebas ejecutadas (evidencia)

Fecha de ejecucion: **2026-05-05**

1. `secuenciales_smoke_test.exe`
- Resultado: **PASS**
- Cobertura: pila, cola, cola prioridad, lista, lista circular, sublistas.
- Casos en verde: **51**.

2. `grafo_smoke_test.exe`
- Resultado: **PASS**
- Cobertura: CRUD de vertices/aristas, BFS/DFS, Dijkstra/Bellman-Ford, Prim/Kruskal, IDs dispersos, casos invalidos (negativos/ciclo negativo/MST en dirigido).
- Casos en verde: **64**.

3. `grafo_demo_smoke_test.exe`
- Resultado: **PASS**
- Cobertura: cantidad de nodos solicitada, conectividad base (`>= n-1`), variacion de aristas en cargas consecutivas.
- Casos en verde: **3**.

Total ronda automatizada: **118 PASS / 0 FAIL**.

## 5) Cobertura por requisito funcional

| Area | Cobertura | Estado |
|---|---|---|
| Estructuras secuenciales | Operaciones base + bordes de vacio | PASS |
| Recorridos BFS/DFS | Ejecucion y conteo de vertices visitados | PASS |
| Camino minimo (Dijkstra/Bellman-Ford) | Ruta, costo, restricciones de pesos | PASS |
| MST (Prim/Kruskal) | Costo total, `n-1` aristas, no dirigido | PASS |
| Manejo de errores criticos | Rechazo de configuraciones invalidas | PASS |
| Demo de grafos | Nodos configurables + aleatoriedad de aristas | PASS |

## 6) Hallazgos y riesgos

### 6.1 Hallazgos criticos
- No se detectaron fallos criticos en la capa logica bajo las pruebas automatizadas ejecutadas.

### 6.2 Riesgos residuales (no bloqueantes)
1. La UI requiere pruebas E2E automatizadas (eventos de mouse/teclado + aserciones visuales) para acercarse mas al ideal exhaustivo en presentacion.
2. La combinatoria de layouts y densidad de grafos muy grande necesita muestreo sistematico adicional (pairwise + fronteras visuales por resolucion).

## 7) Conclusión
Con base en el enfoque de exhaustivo acotado, la aplicacion presenta **estado funcional estable** en los modulos principales y algoritmos de grafos.

Resultado global de esta ronda:
- **Cobertura teorica total: no alcanzable (por complejidad combinatoria).**
- **Cobertura practica ejecutada: 118/118 pruebas automatizadas en PASS, sin fallos criticos detectados.**

## 8) Recomendaciones para acercarse mas a “exhaustivo”
1. Agregar suite E2E visual para pantallas de grafos (con captura y comparacion de layout).
2. Introducir pruebas combinatorias pairwise para `(modo, algoritmo, dirigido, inicio/origen/destino, peso)`.
3. Ampliar fuzzing de entradas numericas y secuencias de interaccion (reiniciar, cambiar modo, ejecutar, editar grafo).
4. Versionar reportes por fecha para seguimiento de regresiones.
