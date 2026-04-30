# Guia Docente

## Proposito
Esta aplicacion esta pensada para explicar operaciones de estructuras secuenciales mostrando tres planos al mismo tiempo:

- estado grafico de la estructura
- historial de codigo C relacionado con cada operacion ejecutada
- explicacion breve con complejidad

## Secuencia sugerida para clase
1. Seleccionar la estructura a estudiar.
2. Inicializar o vaciar para partir de un estado conocido.
3. Insertar algunos elementos variando `valor` y `prioridad`.
4. Pedir al estudiante que anticipe el resultado de la siguiente operacion.
5. Ejecutar la accion y comparar la prediccion con el cambio visual.
6. Relacionar el resultado con el snippet y la complejidad mostrada.
7. Al finalizar una secuencia, usar `Limpiar` en el panel de codigo para comenzar un nuevo bloque didactico.
8. Usar `F1` para abrir la ayuda interna cuando se necesite reforzar conceptos de uso o arquitectura.

## Ideas por estructura
### Pila
- Explicar LIFO con varias inserciones seguidas y un `pop`.
- Usar el marcador de tope para reforzar el concepto.

### Cola
- Contrastar `FRONT` y `BACK`.
- Mostrar como el elemento que entra ultimo no es el primero en salir.

### Cola de prioridad
- Variar prioridades para discutir por que el orden visual de insercion no siempre coincide con la salida.
- Usar el indicador `OUT` para anticipar el siguiente elemento extraido.

### Lista
- Demostrar insercion al inicio y al final.
- Ejecutar busqueda para remarcar coincidencias.
- Invertir la lista y observar el efecto estructural.

### Lista circular
- Explicar el cierre del ciclo: el ultimo nodo vuelve al primero.
- Comparar insercion al inicio y al final resaltando que ambas mantienen circularidad.
- Buscar un valor y discutir por que el recorrido debe detenerse al volver a `HEAD`.
- Invertir y verificar que la estructura sigue cerrada (sin `NULL` final).

## Recomendaciones de uso
- Mantener pocos elementos al inicio para que el patron sea evidente.
- Pedir predicciones antes de pulsar el boton de operacion.
- Repetir una misma secuencia cambiando un solo valor para comparar comportamientos.
- Usar los mensajes de estado y el panel de traza como apoyo para estudiantes que recien comienzan.
- Aprovechar el historial de codigo para reconstruir toda la secuencia ejecutada sin depender de memoria de pasos previos.
- Integrar la pantalla de ayuda como apoyo transversal para aclarar modulos, atajos y errores comunes.

## Limitaciones actuales
- La visualizacion prioriza claridad pedagogica sobre animaciones complejas.
- El proyecto cubre estructuras secuenciales basicas del MVP.
- Ahora incluye cinco estructuras visualizables: pila, cola, cola de prioridad, lista y lista circular.
- El material en `src/legacy` no representa la arquitectura vigente.
