# VisualStruct UTP: Estructuras Secuenciales y Grafos en C con Raylib

## Objetivo
Aplicacion de escritorio para apoyo docente en la Universidad Tecnologica de Pereira.
El sistema visualiza estructuras secuenciales y grafos en C estandar mostrando simultaneamente representacion grafica, operacion, codigo asociado, traza y complejidad.

## Requisitos
- GCC con soporte C11
- Raylib instalada en el sistema
- Windows, Linux o macOS con librerias graficas compatibles

## Entorno recomendado por sistema operativo
### Windows (MSYS2/MinGW)
1. Instalar MSYS2 y abrir terminal `mingw64.exe` (o `ucrt64.exe`).
2. Instalar toolchain y Raylib con `pacman`:
```bash
pacman -Syu
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-raylib make pkgconf
```
3. Verificar:
```bash
gcc --version
pkg-config --modversion raylib
```

### Linux
1. Instalar GCC, Make y Raylib desde el gestor de paquetes de tu distribucion.
2. Verificar:
```bash
gcc --version
pkg-config --modversion raylib
```

### macOS
1. Instalar Xcode Command Line Tools.
2. Instalar Raylib y `pkg-config`:
```bash
xcode-select --install
brew install raylib pkg-config
```
3. Verificar:
```bash
clang --version
pkg-config --modversion raylib
```

## IntelliSense en VS Code
- Se incluye `.vscode/c_cpp_properties.json` con `includePath` apuntando a `C:/msys64/mingw64/include`.
- Si tu instalacion de Raylib usa otra ruta, ajusta ese archivo para que VS Code resuelva `raylib.h` sin advertencias.

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

## Documentacion Tecnica
### Generar con Doxygen
```bash
doxygen Doxyfile
```

### Salida esperada
- HTML en `docs/doxygen/html/index.html`
- Se documenta solo la aplicacion activa y se excluye `src/legacy`

### Documentos incluidos
- `docs/architecture.md`: mapa tecnico de capas, flujo y extension.
- `docs/guia-docente.md`: sugerencias de uso en clase y secuencias de demostracion.
- `docs/contribucion.md`: reglas para extender el proyecto sin romper la arquitectura actual.
- `docs/qa-manual.md`: checklist de verificacion manual para UI, operaciones y paneles pedagogicos.
- `docs/qa-visualizacion-ventanas.md`: verificacion de layout, legibilidad y uso de espacio.
- `docs/analisis-diseno-lista-circular.md`: analisis tecnico y de interfaz para la funcionalidad de lista circular.
- `docs/analisis-diseno-sublistas.md`: analisis funcional y tecnico del TAD jerarquico de sublistas.
- `docs/analisis-diseno-grafos.md`: analisis funcional y tecnico del modulo de grafos y su integracion pedagogica.
- `docs/plan-modulo-grafos.md`: plan incremental de implementacion del modulo de grafos.

## Uso Basico
1. Ejecutar la aplicacion.
2. En la portada, elegir `1 Secuenciales` o `2 Grafos`.
3. En `Secuenciales`, seleccionar: Pila, Cola, Cola de Prioridad, Lista, Lista Circular o Sublistas.
4. En `Grafos`, elegir construccion o algoritmo (BFS, DFS, Dijkstra, Bellman-Ford, Prim, Kruskal).
5. Usar los botones contextuales del panel central para ejecutar operaciones segun la estructura activa.
6. En modo Grafo, editar `valor`, `origen`, `destino` y `peso` en el panel lateral o por teclado.
7. Observar simultaneamente panel central (grafico), panel derecho (codigo C con historial acumulado) y panel inferior (traza y complejidad).
8. Usar el boton `Limpiar` del panel de codigo para reiniciar el historial cuando se quiera comenzar una nueva secuencia.
9. Abrir `Ayuda (F1)` para consultar explicaciones detalladas de cada modulo funcional y tecnico.

## Controles Actuales
- `I`: inicializar estructura activa
- `A`: insertar/push/encolar, insertar al final en lista/lista circular, insertar padre en sublistas, o insertar vertice en Grafo
- `Z`: insertar al inicio en lista/lista circular o insertar hijo en sublistas
- `D`: eliminar/pop/desencolar o eliminar vertice en Grafo
- `B`: buscar en lista/lista circular o seleccionar padre activo en sublistas
- `R`: invertir lista/lista circular, eliminar hijo en sublistas, o ejecutar Kruskal en Grafo
- `V`: vaciar estructura activa
- `UP/DOWN`: cambiar valor
- `LEFT/RIGHT`: cambiar prioridad
- `ENTER`: confirmar valor/prioridad editados en los campos laterales
- `G` (en Grafo): insertar arista con `origen`, `destino` y `peso`
- `X` (en Grafo): eliminar arista entre `origen` y `destino`
- `4`/`5`/`6`/`7`/`8`/`9` (en Grafo): BFS, DFS, Dijkstra, Bellman-Ford, Prim y Kruskal
- `F1`: abrir/cerrar pantalla de ayuda detallada
- `H` o `ESC` (en visualizador): volver al menu principal
- `TAB` (en visualizador): recorrer estructuras activas (1..7)
- `,` / `.` / `/` (en Grafo): paso anterior, paso siguiente y reinicio del algoritmo
- `Home` / `End` (en Grafo): ir al inicio o al final del algoritmo
- `P` (en Grafo): activar/desactivar autoplay
- `O` (en Grafo): cambiar velocidad del autoplay
- `T` (en Grafo): alternar entre grafo dirigido y no dirigido
- `M` (en Grafo): cargar el siguiente escenario demo
- `C` (en Grafo): copiar al portapapeles un resumen textual del algoritmo actual
- `Mouse wheel` sobre panel central en Cola/Lista/Lista Circular/Cola de Prioridad: desplazamiento horizontal para navegar nodos fuera de ancho
- `Mouse wheel` sobre panel central en Pila: desplazamiento vertical
- Boton `Limpiar` en panel de codigo: reinicia el historial de snippets
- Boton lateral `Ayuda (F1)`: acceso guiado al manual interno

## Modulo de Grafos
- Soporta BFS, DFS, Dijkstra, Bellman-Ford, Prim y Kruskal.
- Permite construir grafos dirigidos o no dirigidos con insercion/eliminacion de vertices y aristas.
- El panel inferior muestra tipo de paso, progreso, arista actual, metricas por paso, tabla de distancias, camino parcial y conjunto de vertices cerrados.
- El lienzo central incluye leyenda visual para vertice activo, arista procesada y mejora real.
- `Cargar demo` cicla entre escenarios de recorrido, caminos, Bellman-Ford y MST.
- `Exportar` copia al portapapeles un resumen textual con algoritmo, paso, metricas, camino y tabla de distancias.

## Navegacion rapida
- Portada principal: `1` Secuenciales, `2` Grafos.
- Submenu Secuenciales: `1..6` seleccion directa de estructura.
- Visualizador: `1..7` seleccion directa (incluye Grafo), `TAB` para avanzar.
- Ayuda global: `F1` desde cualquier pantalla.

## Solucion de problemas
- Si no ves `Lista Circular`, `Sublistas` o `Grafo`, asegurate de ejecutar el binario mas reciente (`visualstruct.exe` o `visualstruct_eval_v11.exe` actualizado).
- Si compilas manualmente, incluye todos los `src/grafo_*.c` del comando de compilacion.

## Flujo recomendado de compilacion y validacion
1. Compilar:
```bash
make clean && make
```
2. Regenerar documentacion tecnica:
```bash
doxygen Doxyfile
```
3. Ejecutar QA manual funcional:
- seguir `docs/qa-manual.md`
4. Ejecutar QA visual de layout:
- seguir `docs/qa-visualizacion-ventanas.md`
5. Verificar salida Doxygen:
- abrir `docs/doxygen/html/index.html`

## Estructura de Carpetas
- include/: encabezados publicos
- src/: implementaciones activas de la aplicacion unificada
- src/legacy/: demos antiguas con `main` propio, conservadas como referencia de migracion
- assets/: logos y recursos visuales
- assets/fons/: fuentes tipograficas externas usadas por encabezado, pie y controles
- docs/: documentacion del proyecto

## Estado Actual
Arquitectura unificada operativa: app unica, estado global desacoplado, vistas por estructura (secuenciales + grafo), controles contextuales, ayuda interna detallada, historial de snippets C por operacion y soporte pedagogico ampliado para algoritmos de grafos.

## Notas de Arquitectura
- La app actual usa un unico punto de entrada en `src/main.c`.
- Los archivos legacy `*_raylib.c` ya no forman parte del build principal.
- Los TAD permanecen separados de Raylib en `src/*.c` y `include/*.h`.
