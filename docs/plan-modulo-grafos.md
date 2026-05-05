# Plan: Modulo de Grafos y Navegacion

## Contexto
Proyecto: `VisualStruct Version 2`
Lenguaje: C11 + Raylib.

## Objetivo
Mantener el modulo de grafos funcional, didactico y simple para usuario final sin perder capacidades avanzadas.

## Estado implementado
- Vistas: Construccion, Recorridos, Caminos, MST.
- Inputs numericos validados.
- Recorridos con inicio definido por usuario y lista de orden visible.
- Caminos minimos con pesos.
- MST con flujo basico y avanzado.
- Demo parametrica por numero de nodos y aleatoriedad dispersa.

## Fases de mejora (vigentes)
### Fase 1: UX simplificada
- Mantener flujo basico por vista con pocos botones.
- Dejar controles expertos en modo avanzado.

### Fase 2: Robustez algoritmica
- Refactor Prim/Kruskal para IDs no contiguos.
- Endurecer validaciones de entradas extremas.

### Fase 3: Demo configurable
- Exponer selector de densidad y rango de pesos.
- Guardar semilla opcional para reproducibilidad docente.

### Fase 4: QA y documentacion continua
- Ampliar smoke tests para escenarios de IDs grandes y grafos desconectados.
- Mantener bitacora de cambios por fecha.

## Reglas tecnicas
- TAD grafo sin dependencias de render.
- Mutaciones solo via `app_state` y `grafo_controller`.
- Liberacion segura de estructuras dinamicas de algoritmos.

## Criterio de cierre por fase
- Compila sin errores.
- QA manual actualizado y aprobado.
- Documentacion sincronizada con comportamiento real.
