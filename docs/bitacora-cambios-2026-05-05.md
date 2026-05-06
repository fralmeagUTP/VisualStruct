# Bitacora de Cambios - 2026-05-05

## Resumen
Se consolido la actualizacion funcional y documental de la app, con foco en el modulo de grafos y experiencia de usuario final.

## Cambios funcionales
1. MST en modo basico:
- ejecucion directa de Prim y Kruskal
- botones simplificados para uso rapido

2. Demo de grafo configurable:
- `Valor` define cantidad de nodos al cargar demo
- rango operativo controlado (2..32)

3. Demo aleatoria dispersa:
- aristas y pesos aleatorios
- conectividad base garantizada
- densidad reducida para evitar grafos fuertemente conexos

4. Sincronizacion visual de layout:
- evita recircularizacion forzada en cada refresco
- conserva mejor la percepcion de variacion entre demos

5. Ajustes de resaltado:
- aristas en no dirigido se resaltan aunque el sentido llegue invertido

## Documentacion actualizada
- `README.md`
- `docs/modulos-funcionalidades.md` (nuevo)
- `docs/architecture.md`
- `docs/guia-docente.md`
- `docs/qa-manual.md`
- `docs/analisis-diseno-grafos.md`
- `docs/plan-modulo-grafos.md`

## Estado final
- Compilacion local exitosa.
- Ejecutable principal actualizado.
- Documentacion alineada con el comportamiento actual.

## Actualizacion adicional (2026-05-05 tarde)
1. Reorganizacion visual de Construccion:
- sidebar de entradas con espaciado mayor para evitar texto montado
- mensajes de validacion (`Origen invalido`, `Destino invalido`) sin superposicion.

2. Simplificacion en vista Caminos:
- se elimino boton `Aplicar peso a arista`
- botonera redistribuida a 6 acciones clave
- texto de resumen actualizado a:
  - `Sin arista directa Vx->Vy` cuando aplica.

3. Ajuste de visual en MST:
- en Prim/Kruskal no se marca vertice `Final`
- durante pasos se marca un unico vertice activo
- al cierre los vertices vuelven a color base y se conserva MST en lila.

4. Paneles pedagogicos refinados:
- modo Construccion con resumen derecho compacto
- traza inferior compacta para reducir redundancia visual.
