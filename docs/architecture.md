# Arquitectura Actual

## Aplicacion activa
Componentes principales:
- `src/main.c`: ciclo principal, eventos, rutas de pantalla y render global.
- `src/ui.c` + `include/ui.h`: layout, paneles y widgets reutilizables.
- `src/app_state.c` + `include/app_state.h`: estado global, despacho de operaciones y reglas de validacion.
- `src/code_viewer.c` + `src/algorithm_trace.c`: snippets y traza general de estructuras secuenciales.
- Modulo Grafo:
  - Dominio: `src/grafo.c`, `include/grafo.h`
  - Control: `src/grafo_controller.c`, `src/grafo_state.c`
  - Render/layout: `src/grafo_view.c`, `src/grafo_layout.c`
  - Pedagogia: `src/grafo_code_viewer.c`, `src/grafo_trace.c`, `src/grafo_pedagogy.c`

## Flujo por frame
1. `main` captura teclado/mouse.
2. Eventos se traducen a acciones de `app_state`.
3. `app_state` ejecuta sobre TADs y controlador de grafo.
4. Vista dibuja snapshot sin mutar dominio.
5. Paneles pedagogicos reflejan resultado de la operacion actual.

## Capas y responsabilidades
### Dominio
- TADs desacoplados de Raylib.
- Reglas de negocio y algoritmos.

### Aplicacion
- Orquesta estado, validaciones, mensajes y sincronizacion.
- Punto unico para mutaciones desde UI.

### Presentacion
- Dibujo, navegacion, inputs y feedback visual.
- No modifica internals de TAD directamente.

### Pedagogica
- Mapea operaciones a snippets, traza y complejidad.
- En grafo agrega metricas por paso, resumen de ruta y estado de ejecucion.

## Invariantes
- Un solo entrypoint activo: `src/main.c`.
- Todas las mutaciones pasan por API publica del TAD y `app_state`.
- TADs no dependen de `raylib.h`.

## Decisiones recientes de arquitectura (2026-05-05)
- `Cargar demo` de Grafo ahora es generador parametrico (cantidad de nodos por `Valor`).
- Generacion aleatoria de aristas/pesos con baja densidad para evitar sobreconexion.
- Sincronizacion visual de layout preserva posiciones existentes y evita recircularizar cada refresco.
- Vista MST en modo basico prioriza ejecucion directa (Prim/Kruskal) y deja controles finos en modo avanzado.
- Vista Caminos elimina accion de peso en runtime para separar responsabilidades:
  - Construccion define pesos.
  - Caminos consume pesos y ejecuta algoritmo.
- Paneles pedagogicos en Construccion se compactan para reducir duplicidad entre resumen y traza.

## Riesgos vigentes
- IDs de vertices muy grandes en Prim/Kruskal pueden requerir refactor a indices compactos.
- Ajustes de densidad de demo pueden necesitar calibracion adicional por tamano del grafo.

## Extensiones recomendadas
1. Compactar internamente IDs de vertices para algoritmos MST.
2. Agregar plantilla de configuracion de demo (densidad, rango de pesos, dirigido).
3. Separar controles por rol (estudiante/docente) para reducir carga cognitiva.
