# Bitacora de Cambios - 2026-05-04

## Resumen
En la fecha `2026-05-04` se realizaron mejoras funcionales y de usabilidad sobre el modulo de grafos y su capa de presentacion, con foco en legibilidad, simplificacion de flujo para usuario final y validaciones de entrada.

## Cambios funcionales en modulo de grafos

1. Simplificacion de pantalla de construccion:
- Se introdujo un modo basico enfocado en flujo corto de creacion (`Inicializar -> Vertice -> Arista`).
- Se agregaron controles de alternancia `Modo avanzado` y `Ver detalles` para ocultar o mostrar paneles secundarios.

2. Entradas numericas en panel lateral:
- Los campos `Valor`, `Origen`, `Destino` y `Peso` se comportan como entradas numericas.
- Se elimino el comportamiento de concatenacion tipo cadena al enfocar y escribir.
- Se agrego cursor visible en el campo activo.
- Se permitio ajuste por teclado (`UP/DOWN` y `KP +/-`) sobre el input activo.

3. Validacion de vertices de arista:
- `Origen` y `Destino` se validan en tiempo real contra vertices existentes en el grafo.
- Si el ID no existe, el campo queda invalido y no se aplica al estado interno.

4. Visualizacion de grafo dirigido:
- Se reforzo la representacion de direccion en aristas para modo dirigido.

## Cambios de legibilidad visual

1. Tipografia:
- Unificacion de tipografia de interfaz hacia Arial (objetivo de legibilidad consistente en UI).

2. Vertices y etiquetas:
- Se aumento el radio visual de vertices.
- Se aclaro el color base de vertices (gris mas claro).
- Se incremento tamano de texto para etiqueta de vertice y peso de arista.

3. Pesos de arista:
- Se ajusto el offset de labels de peso para separarlos de la linea de arista y mejorar lectura.

4. Ajustes de micro-layout:
- Correcciones de solapes de texto en paneles de estado/traza/codigo.
- Correcciones de alineacion en cajas laterales y encabezados operativos.

## Versionado y respaldos (GitHub)

1. Tags de version:
- `v0.0.1`: reservada para la version historica inicial (primera subida solicitada por el equipo).
- `v0.0.2`: version actual con ajustes de UI/UX y validaciones del modulo de grafos.

2. Politica recomendada de respaldo:
- Mantener tags inmutables por version entregada.
- Generar copia de seguridad por hito (`zip` de release o branch de respaldo).
- Registrar hallazgos QA antes de publicar una nueva version.

## Archivos impactados (principalmente)
- `src/main.c`
- `src/ui.c`
- `src/grafo_view.c`
- `src/app_state.c`
- `README.md`
- `docs/qa-manual.md`
- `docs/qa-visualizacion-ventanas.md`

