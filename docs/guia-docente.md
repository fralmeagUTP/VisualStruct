# Guia Docente

## Proposito
Usar VisualStruct para explicar estructuras y algoritmos con evidencia visual y traza tecnica sincronizada.

## Flujo sugerido de clase
1. Seleccionar modulo.
2. Inicializar.
3. Cargar/crear caso de prueba.
4. Pedir prediccion al estudiante.
5. Ejecutar operacion/algoritmo.
6. Validar contra visual, codigo y traza.

## Recomendaciones por modulo

### Pila y Cola
- Empezar con 3-5 elementos.
- Contrastar LIFO vs FIFO con el mismo conjunto de valores.

### Cola de Prioridad
- Variar prioridad manteniendo valor para mostrar desacople entre orden de llegada y salida.

### Lista y Lista Circular
- Enfatizar diferencia entre fin en `NULL` vs cierre circular.
- Usar invertir para verificar comprension de enlaces.

### Sublistas
- Trabajar con 2 padres y 2-3 hijos por padre para evidenciar independencia.

### Grafo
- Construccion: crear vertices/aristas y alternar dirigido/no dirigido.
- Recorridos: definir inicio y comparar BFS vs DFS.
- Caminos: demostrar impacto del peso con Dijkstra y Bellman-Ford.
  - los pesos se definen en Construccion (no en la vista Caminos).
- MST:
  - modo basico: ejecutar Prim/Kruskal directo
  - modo avanzado: usar paso a paso y autoplay para analisis fino

## Demo de grafo para clase
`Cargar demo (M)`:
- usa `Valor` como numero de nodos
- genera aristas aleatorias dispersas
- mantiene conectividad base para que recorridos y caminos sean ejecutables
- recomienda usar 6-15 nodos para explicacion en pantalla

## Nota didactica reciente
- En resumen de caminos, si no existe arista directa origen->destino, la app muestra:
  - `Sin arista directa Vx->Vy`
- Esto permite diferenciar entre:
  - costo total de ruta (multiarista)
  - peso de arista directa (si existe).

## Buenas practicas de sesion
- Limpiar historial de codigo al cerrar cada bloque tematico.
- Registrar en pizarra: entrada -> algoritmo -> resultado esperado.
- Cambiar un solo parametro por iteracion para facilitar comparacion.

## Limitaciones conocidas
- Prim/Kruskal actualmente requieren cuidado con IDs de vertices muy grandes.
- En grafos muy grandes, la lectura de etiquetas puede requerir zoom visual (modo avanzado + menos nodos por practica).
