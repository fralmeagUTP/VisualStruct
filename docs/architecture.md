# Arquitectura Actual

## Aplicacion activa
La aplicacion principal se compone de los siguientes grupos:

- `src/main.c`: ciclo principal, eventos y render general.
- `src/ui.c` y `include/ui.h`: layout, paneles y widgets reutilizables.
- `src/app_state.c` y `include/app_state.h`: estado global y despacho de operaciones.
- `src/code_viewer.c`: snippets C asociados a cada operacion.
- `src/algorithm_trace.c`: explicaciones paso a paso y complejidades.
- `src/*_view.c`: visualizacion grafica por estructura.
- `src/pila.c`, `src/cola.c`, `src/cola_prioridad.c`, `src/lista.c`: TAD desacoplados de Raylib.
- `src/main.c` tambien contiene la pantalla de ayuda interna detallada (`SCREEN_HELP`) con scroll y acceso por `F1`.

## Flujo de ejecucion
La app sigue un flujo unico por cuadro:

1. `src/main.c` procesa entrada de teclado y mouse.
2. Los eventos se traducen a operaciones de alto nivel sobre `AppState`.
3. `app_state` invoca la API publica del TAD activo.
4. La vista activa consulta un snapshot del TAD y lo dibuja sin modificarlo.
5. El panel de codigo acumula historial de snippets por operacion ejecutada y los paneles auxiliares muestran traza y complejidad de la operacion actual.
6. La ayuda interna puede abrirse en cualquier momento sin romper el estado de la sesion.

## Capas y responsabilidades
### Capa de dominio
- Corresponde a los TAD en `src/pila.c`, `src/cola.c`, `src/cola_prioridad.c` y `src/lista.c`.
- Debe mantenerse independiente de Raylib y de cualquier decision visual.

### Capa de aplicacion
- Corresponde a `src/app_state.c`.
- Centraliza la seleccion de estructura, la operacion actual, los mensajes y el ultimo contexto visible.
- Expone un serial de operacion para detectar cada evento ejecutado, incluso si se repite la misma accion.
- Es la capa autorizada para coordinar UI y TAD.

### Capa de presentacion
- Corresponde a `src/main.c`, `src/ui.c` y `src/*_view.c`.
- Traduce el estado a layout, controles, paneles y representaciones visuales.
- Mantiene historial textual del panel `Codigo C Asociado` y permite limpiar ese historial desde UI.
- Incluye un modulo de ayuda interna extensa para explicar el flujo funcional y tecnico de la app.
- No debe crear reglas de negocio nuevas ni alterar nodos por acceso directo.

## Invariantes de diseno
- Existe un unico `main` activo en `src/main.c`.
- Toda mutacion pasa por funciones publicas del TAD.
- Las vistas dibujan a partir de copias o snapshots, no desde manipulacion interna de nodos.
- El codigo pedagogico auxiliar usa snippets por operacion y conserva historial acumulado de ejecuciones en el panel de codigo.
- La ayuda interna no muta estructuras; solo presenta informacion y navegacion.

## Puntos de extension
Para agregar una nueva estructura o comportamiento docente, el camino esperado es:

1. Crear o adaptar el TAD con encabezado publico en `include/`.
2. Extender `TipoEstructura` y `TipoOperacion` en `include/app_state.h`.
3. Incorporar operaciones en `src/app_state.c`.
4. Crear una vista dedicada `src/*_view.c`.
5. Añadir snippet y traza en `src/code_viewer.c` y `src/algorithm_trace.c`.
6. Registrar botones, atajos y rotulos en `src/main.c`.

## Material legacy
Los archivos en `src/legacy/` son demostraciones anteriores con `main` propio:

- `pilas_raylib.c`
- `colas_raylib.c`
- `colas_prioridad_raylib.c`
- `listas_raylib.c`

Se conservan como referencia historica y fuente de ideas visuales, pero no participan en la compilacion de la app unificada.

## Regla de separacion
- Los TAD no incluyen `raylib.h`.
- Las vistas y la UI si pueden usar `raylib.h`.
- La UI no modifica nodos directamente; toda operacion pasa por `app_state` y por la API publica del TAD.

## Riesgos controlados por la refactorizacion
- Duplicacion de logica de interfaz entre demos independientes.
- Acoplamiento entre renderizado y operaciones de estructura.
- Multiples puntos de entrada dificiles de mantener o documentar.
- Cambios docentes que antes obligaban a editar varias demos por separado.
