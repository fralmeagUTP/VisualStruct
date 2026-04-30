# QA de Visualizacion de Ventanas

Este documento define una prueba manual reproducible para validar layout, legibilidad y uso de espacio en la app VisualStruct.

## Objetivo

Verificar que no existan solapes, recortes ni zonas inutiles en los paneles de la interfaz al cambiar tamano de ventana y estructura activa.

## Preparacion

1. Compilar el proyecto:

```powershell
gcc -std=c11 -Wall -Wextra -pedantic -Iinclude src/main.c src/ui.c src/app_state.c src/code_viewer.c src/algorithm_trace.c src/pila_view.c src/cola_view.c src/cola_prioridad_view.c src/lista_view.c src/lista_circular_view.c src/sublista_view.c src/pila.c src/cola.c src/cola_prioridad.c src/lista.c src/lista_circular.c src/sublista.c -o visualstruct_qa_layout -lraylib -lopengl32 -lgdi32 -lwinmm
```

2. Ejecutar:

```powershell
./visualstruct_qa_layout.exe
```

## Matriz de prueba (resolucion)

1. 1100x680 (minima)
2. 1280x760 (base)
3. 1366x768
4. 1600x900

## Escenarios funcionales por estructura

1. Pila:
- Inicializar, insertar 8 valores, eliminar 3
- Confirmar scroll vertical de pila y estado TOPE visible

2. Cola:
- Inicializar, encolar 12 valores
- Confirmar scroll horizontal de cola con rueda y drag de barra
- Confirmar etiqueta FRONT/BACK visible sin recorte

3. Cola de prioridad:
- Insertar valores con prioridades variadas
- Verificar nodo OUT y lectura de V/P sin solape

4. Lista:
- Insertar inicio/final, buscar, eliminar, invertir
- Verificar HEAD/NULL visibles y flechas de enlace claras

5. Lista circular:
- Insertar inicio/final, buscar, eliminar, invertir
- Verificar etiqueta `HEAD` y retorno circular del ultimo al primero
- Confirmar scroll horizontal y barra draggable

6. Sublistas:
- Inicializar, insertar padres, seleccionar padre, insertar/eliminar hijos
- Verificar resaltado de padre activo y agrupacion visual padre-hijos
- Confirmar scroll vertical cuando existan suficientes filas

7. Ayuda interna:
- Abrir con `F1` desde menu principal
- Verificar titulo, subtitulo, boton `Volver` y area de texto con scroll
- Cerrar con `F1` y confirmar retorno a pantalla previa

## Checklist de visualizacion

1. Panel lateral:
- Botones del menu visibles y clicables
- Seccion de contexto (Seleccion actual, Elementos, ayuda) no pisa inputs
- Inputs Valor/Prioridad no se recortan
- Prioridad solo aparece en Cola de prioridad
- Si el alto disponible es bajo, la ayuda se compacta sin superponerse

2. Panel central:
- Controles de operacion no invaden el lienzo grafico
- Lienzo muestra nodos completos sin cortar etiquetas
- Si aparece "Modo compacto", no se superpone al titulo

3. Panel derecho (Codigo C Asociado):
- Caja de resumen superior legible
- Snippet visible desde la primera linea
- Scroll vertical operativo cuando hay codigo largo
- Botones del encabezado (incluye `Limpiar`) visibles y sin solape

4. Panel inferior (Operacion, Traza y Complejidad):
- Caja Resumen sin texto cortado
- Caja Traza con titulo visible y scroll funcional
- Estado (OK/Error/Estado) no se superpone a otros elementos

5. Pantalla de ayuda:
- La banda superior (titulo, descripcion y `Volver`) no se recorta
- El bloque de texto largo mantiene legibilidad y margen interno
- Scrollbar de ayuda visible cuando el contenido excede el viewport

## Criterios de aceptacion

1. Cero solapes visibles entre textos, botones y cajas.
2. Cero recortes de texto en secciones clave (Resumen, Traza, Sidebar).
3. Scroll funcional donde aplique (pila, cola, lista, lista circular, sublistas, codigo y traza).
4. Navegacion estable al cambiar entre estructuras y volver al menu principal.

## Registro de hallazgos

Usar esta plantilla por hallazgo:

- Resolucion:
- Estructura activa:
- Zona afectada (Sidebar/Centro/Derecha/Inferior):
- Descripcion:
- Pasos para reproducir:
- Resultado esperado:
- Resultado actual:
- Severidad (Alta/Media/Baja):

## Hallazgos corregidos en esta iteracion

1. Sidebar con solape entre ayuda y campos de entrada en resoluciones ajustadas.
- Solucion: layout adaptativo por altura disponible y version compacta de ayuda.

2. Campo Prioridad visible en estructuras donde no aplica.
- Solucion: render condicional exclusivo para Cola de prioridad.

3. Texto de error de prioridad mostrado fuera de contexto.
- Solucion: validacion y mensaje visibles solo cuando la estructura activa usa prioridad.
