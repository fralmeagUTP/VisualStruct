# Guia de Pruebas Manuales

## Objetivo
Esta guia define una verificacion manual minima para confirmar que la app unificada mantiene comportamiento correcto y feedback pedagogico consistente.

## Preparacion
1. Compilar la aplicacion.
2. Ejecutar el binario generado.
3. Verificar que carguen la ventana principal, el menu lateral y los paneles de codigo y traza.

## Chequeos globales
- Cambiar entre Pila, Cola, Cola de Prioridad, Lista, Lista Circular, Sublistas y Grafo desde la barra lateral.
- Verificar que el titulo, los botones contextuales y el conteo de elementos cambian con la estructura activa.
- Confirmar que los campos `Valor` y `Prioridad` aceptan edicion.
- En Grafo, confirmar edicion de `Origen`, `Destino` y `Peso`.
- Escribir una entrada invalida y comprobar que la caja se marca en rojo.
- Verificar que el estado de operacion muestre mensaje coherente tras acciones validas e invalidas.
- Abrir/cerrar ayuda con `F1` desde menu principal y visualizador sin bloqueos.
- Abrir ayuda desde boton lateral `Ayuda (F1)` y verificar retorno correcto con `Volver` o `ESC`.

## Pila
1. Inicializar la pila.
2. Insertar tres valores distintos.
3. Confirmar que el ultimo insertado aparece en el tope y con marca `NEW`.
4. Eliminar un elemento.
5. Verificar que el tope cambia y que aparece el nodo fantasma con `POP`.
6. Vaciar y confirmar el mensaje `Pila vacia`.

## Cola
1. Inicializar la cola.
2. Insertar al menos tres valores (idealmente mas de los que caben sin scroll).
3. Confirmar que aparecen `FRONT` y `BACK` en posiciones correctas.
4. Usar rueda del mouse sobre el panel central y verificar desplazamiento horizontal.
5. Eliminar un elemento.
6. Verificar avance del frente y presencia del nodo fantasma `OUT`.
7. Vaciar y confirmar el mensaje `Cola vacia`.

## Cola de prioridad
1. Inicializar la estructura.
2. Insertar varios elementos con prioridades distintas (incluyendo mas de los visibles en ancho).
3. Verificar que la marca `OUT` coincida con la menor prioridad.
4. Usar rueda del mouse sobre el panel central y verificar desplazamiento horizontal.
5. Eliminar un elemento.
6. Confirmar que el nodo fantasma muestra `V:` y `P:` correctos.
7. Editar una prioridad invalida y comprobar feedback en rojo.

## Lista
1. Inicializar la lista.
2. Insertar un valor al inicio y otro al final; luego agregar suficientes nodos para forzar overflow horizontal.
3. Confirmar presencia de `HEAD` y `NULL`.
4. Usar rueda del mouse sobre el panel central y verificar desplazamiento horizontal.
5. Buscar un valor existente y verificar resaltado.
6. Invertir la lista y observar cambio de orden.
7. Eliminar y verificar nodo fantasma `DEL`.

## Lista circular
1. Inicializar la lista circular.
2. Insertar al inicio y al final al menos 6 valores para forzar overflow horizontal.
3. Verificar que se muestre `HEAD` y la conexion de retorno (ultimo nodo vuelve al primero).
4. Usar rueda del mouse sobre el panel central y verificar desplazamiento horizontal.
5. Buscar un valor existente y comprobar resaltado de coincidencias.
6. Invertir la lista y validar que se conserva la circularidad.
7. Eliminar un valor y verificar nodo fantasma `DEL`.

## Sublistas
1. Inicializar la estructura de sublistas.
2. Insertar al menos dos nodos padre con valores distintos.
3. Buscar uno de los padres para dejarlo activo.
4. Insertar dos hijos sobre el padre activo y validar que se rendericen debajo del padre correcto.
5. Cambiar de padre activo con `Buscar` y agregar un hijo adicional; confirmar independencia entre sublistas.
6. Eliminar un hijo en el padre activo y validar feedback visual.
7. Eliminar un padre y confirmar que tambien desaparece su sublista asociada.

## Grafos
1. Inicializar el grafo y alternar entre modo dirigido y no dirigido.
2. Insertar vertices y aristas manualmente verificando validaciones de origen, destino y peso.
3. Cargar demos con el boton `Cargar demo` hasta recorrer los escenarios de BFS, caminos, Bellman-Ford y MST.
4. Ejecutar BFS/DFS y comprobar que el panel inferior distingue seleccion de vertice frente a consolidacion.
5. Ejecutar Dijkstra y confirmar en el panel inferior: arista actual, tipo de paso, camino parcial, cerrados y metricas `Mejoras`, `Sin cambio`, `Empeora`.
6. Ejecutar Bellman-Ford y verificar el paso final explicito de chequeo/consolidacion.
7. Activar autoplay, cambiar velocidad y usar `Home`/`End` sin bloqueos.
8. Pulsar `C` o `Exportar` y comprobar que el resumen se copia al portapapeles.
9. Confirmar que el lienzo central muestra leyenda para activo, procesada y mejora.

## Paneles pedagogicos
- Tras cada operacion, revisar que el panel `Codigo C Asociado (Historial)` agregue una nueva entrada con estructura, operacion y snippet.
- Presionar `Limpiar` en el panel de codigo y confirmar que el historial se reinicia.
- Confirmar que la traza textual y la complejidad cambian con la operacion actual.
- Probar scroll en panel de codigo y panel de traza cuando el texto excede el espacio visible.
- En Grafos, verificar que panel inferior, lienzo y traza se mantengan sincronizados al avanzar, retroceder, reiniciar y autoplay.

## Pantalla de ayuda
- Verificar que el contenido sea extenso y legible en resolucion minima y base.
- Confirmar scroll vertical funcional dentro del cuerpo de ayuda.
- Validar que `F1` alterna apertura/cierre de ayuda conservando el contexto previo.

## Criterios de salida
Se considera aceptable una iteracion si:

- no hay cierres inesperados
- la estructura visible coincide con la operacion aplicada
- el feedback visual de error funciona
- snippet, traza y complejidad permanecen sincronizados con la accion
- la app sigue compilando despues del cambio evaluado
