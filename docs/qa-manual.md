# Guia de Pruebas Manuales

## Matriz por Pantalla
- Ver matriz operativa de esta ronda en docs/qa-checklist-pantallas-2026-05-05.md.

## Objetivo
Verificar comportamiento funcional y consistencia pedagogica de todos los modulos.

## Preparacion
1. Compilar.
2. Ejecutar `visualstruct.exe`.
3. Confirmar carga de panel central, panel de codigo y panel de traza.

## Prueba E2E visual automatica
Ejecutar:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run_visual_e2e.ps1 -ExePath .\visualstruct.exe -OutputDir .\artifacts\e2e_visual
```

Validaciones automaticas incluidas:
- resolucion minima por captura
- diversidad visual minima por pantalla
- densidad minima de trazado en escenas de grafo
- presencia de contenido en regiones de header, panel derecho y panel inferior
- presencia de contenido y variacion visual en area de dibujo para escenas de grafo

Evidencias generadas:
- `artifacts/e2e_visual/*.png`
- `artifacts/e2e_visual/report.json`
- `docs/informe-e2e-visual-YYYY-MM-DD.md`

## Matriz E2E visual (cobertura ampliada)
Ejecutar:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run_visual_e2e_matrix.ps1 -ExePath .\visualstruct.exe -OutputRoot .\artifacts\e2e_visual_matrix -Runs 3
```

Validaciones adicionales:
- estabilidad de PASS en multiples corridas
- verificacion de variacion visual entre corridas para escenas de grafo (aleatoriedad)

## Checklist global
- Cambio estable entre 7 modulos.
- Mensajes de estado coherentes en exito/error.
- Inputs numericos operativos y con cursor visible.
- Ayuda (`F1`) abre y cierra sin perder contexto.
- Header institucional sin solapes: logos visibles, proporcionales y sin cruzar la linea inferior.

## Secuenciales
### Pila
- Push x3, Pop x1, vaciar.

### Cola
- Encolar x3, desencolar x1, validar `FRONT/BACK`.

### Cola de prioridad
- Insertar con prioridades distintas, validar orden de salida.

### Lista
- Insertar inicio/final, buscar, invertir, eliminar.

### Lista Circular
- Insertar varios nodos, validar cierre, buscar, invertir.

### Sublistas
- Crear padres, seleccionar padre, agregar/eliminar hijos.

## Grafo
### Construccion
1. Inicializar.
2. Insertar vertices.
3. Insertar aristas con peso.
4. Alternar dirigido/no dirigido.
5. Validar que origen/destino invalidos no aplican cambios.
6. Confirmar que mensajes `Origen invalido` / `Destino invalido` no se montan sobre otros campos.

### Demo configurable
1. Definir `Valor` (ej. 8, luego 14).
2. Pulsar `Cargar demo` en cada caso.
3. Verificar que cantidad de vertices corresponde a `Valor`.
4. Repetir `Cargar demo` con el mismo valor y confirmar variacion de aristas/pesos.
5. Confirmar que el grafo no queda excesivamente denso.

### Recorridos
1. Definir inicio.
2. Ejecutar BFS y DFS.
3. Verificar lista de vertices recorridos y progreso de pasos.

### Caminos minimos
1. Definir origen/destino/peso.
2. Ejecutar Dijkstra y Bellman-Ford.
3. Confirmar ruta y costo.
4. Verificar que no existe boton `Aplicar peso a arista` en esta vista.
5. Si no hay arista directa origen->destino, validar texto:
   - `Sin arista directa Vx->Vy`.

### MST
1. Cambiar a vista MST.
2. En modo basico ejecutar Prim y Kruskal.
3. Verificar resultado y avance de paso.
4. Activar modo avanzado y validar controles de paso/autoplay.

## Paneles pedagogicos
- `Codigo C Asociado`: agrega entrada por operacion.
- `Limpiar`: reinicia historial.
- `Operacion, Traza y Complejidad`: cambia segun accion actual.
- En modo Grafo/Construccion:
  - panel derecho compacto (estado, conteos y siguiente paso)
  - panel inferior con traza corta (sin duplicacion de textos largos).

## Criterio de salida
Aprobado si:
- no hay cierres inesperados
- las operaciones afectan la estructura correcta
- codigo/traza/visualizacion permanecen sincronizados
- no hay regresiones visibles en inputs y controles de grafo
