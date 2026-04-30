# Visualizador de Estructuras de Datos Secuenciales en C con Raylib

## Objetivo
Aplicacion de escritorio para apoyo docente en la Universidad Tecnologica de Pereira.
El sistema visualiza estructuras secuenciales en C estandar mostrando simultaneamente representacion grafica, operacion, codigo asociado, traza y complejidad.

## Requisitos
- GCC con soporte C11
- Raylib instalada en el sistema
- Windows, Linux o macOS con librerias graficas compatibles

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
  src/pila_view.c src/cola_view.c src/cola_prioridad_view.c src/lista_view.c src/lista_circular_view.c src/sublista_view.c \
  src/pila.c src/cola.c src/cola_prioridad.c src/lista.c src/lista_circular.c src/sublista.c \
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

## Uso Basico
1. Ejecutar la aplicacion.
2. Seleccionar estructura en el menu lateral: Pila, Cola, Cola de Prioridad, Lista, Lista Circular o Sublistas.
3. Usar los botones contextuales del panel central para ejecutar operaciones segun la estructura activa.
4. Editar `valor` y `prioridad` directamente en el panel lateral o usar teclado.
5. Ajustar `valor` con `UP/DOWN`.
6. Ajustar `prioridad` con `LEFT/RIGHT` cuando la estructura activa sea Cola de Prioridad.
7. Observar simultaneamente panel central (grafico), panel derecho (codigo C con historial acumulado) y panel inferior (traza y complejidad).
8. Usar el boton `Limpiar` del panel de codigo para reiniciar el historial cuando se quiera comenzar una nueva secuencia.
9. Abrir `Ayuda (F1)` para consultar explicaciones detalladas de cada modulo funcional y tecnico.

## Controles Actuales
- `I`: inicializar estructura activa
- `A`: insertar/push/encolar, insertar al final en lista/lista circular, o insertar padre en sublistas
- `Z`: insertar al inicio en lista/lista circular o insertar hijo en sublistas
- `D`: eliminar/pop/desencolar
- `B`: buscar en lista/lista circular o seleccionar padre activo en sublistas
- `R`: invertir lista/lista circular o eliminar hijo en sublistas
- `V`: vaciar estructura activa
- `UP/DOWN`: cambiar valor
- `LEFT/RIGHT`: cambiar prioridad
- `ENTER`: confirmar valor/prioridad editados en los campos laterales
- `F1`: abrir/cerrar pantalla de ayuda detallada
- `H` o `ESC` (en visualizador): volver al menu principal
- `Mouse wheel` sobre panel central en Cola/Lista/Lista Circular/Cola de Prioridad: desplazamiento horizontal para navegar nodos fuera de ancho
- `Mouse wheel` sobre panel central en Pila: desplazamiento vertical
- Boton `Limpiar` en panel de codigo: reinicia el historial de snippets
- Boton lateral `Ayuda (F1)`: acceso guiado al manual interno

## Estructura de Carpetas
- include/: encabezados publicos
- src/: implementaciones activas de la aplicacion unificada
- src/legacy/: demos antiguas con `main` propio, conservadas como referencia de migracion
- assets/: logos y recursos visuales
- assets/fons/: fuentes tipograficas externas usadas por encabezado, pie y controles
- docs/: documentacion del proyecto

## Estado Actual
Arquitectura unificada operativa: app unica, estado global desacoplado, vistas por estructura (incluyendo Lista Circular y Sublistas), controles contextuales, ayuda interna detallada, historial de snippets C por operacion, complejidades base del MVP y feedback animado breve en inserciones/eliminaciones.

## Notas de Arquitectura
- La app actual usa un unico punto de entrada en `src/main.c`.
- Los archivos legacy `*_raylib.c` ya no forman parte del build principal.
- Los TAD permanecen separados de Raylib en `src/*.c` y `include/*.h`.
