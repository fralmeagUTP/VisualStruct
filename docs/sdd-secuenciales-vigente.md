# SDD Vigente: Modulo Secuencial (VisualStruct)

## 1. Informacion general
### 1.1 Nombre
VisualStruct UTP - Modulo Secuencial (app unificada en C con Raylib).

### 1.2 Proposito
Definir el diseno y alcance vigente del modulo secuencial integrado en VisualStruct.
Este documento reemplaza como referencia operativa al SDD inicial de "modulo 1" y refleja
el estado actual del codigo en `main`.

### 1.3 Contexto
- Lenguaje: C11
- UI: Raylib
- Arquitectura: app unica con `src/main.c` + `AppState` + vistas por estructura.
- Integracion: convive con modulo Grafo dentro del mismo ejecutable.

## 2. Alcance funcional vigente
El modulo secuencial incluye seis estructuras:

1. Pila
2. Cola FIFO
3. Cola de prioridad
4. Lista enlazada simple
5. Lista circular
6. Sublistas (modelo padre-hijo)

Cada estructura mantiene:

- visualizacion en panel central
- operaciones por botones y atajos
- snippet C asociado en panel derecho (historial)
- traza y complejidad en panel inferior
- feedback de estado (exito/error)

## 3. Operaciones por estructura
### 3.1 Pila
- Inicializar
- Insertar (push)
- Eliminar (pop)
- Vaciar

### 3.2 Cola
- Inicializar
- Insertar (encolar)
- Eliminar (desencolar)
- Vaciar

### 3.3 Cola de prioridad
- Inicializar
- Insertar con prioridad
- Eliminar siguiente por prioridad
- Vaciar

### 3.4 Lista simple
- Inicializar
- Insertar al inicio
- Insertar al final
- Buscar
- Eliminar primera ocurrencia
- Invertir
- Vaciar

### 3.5 Lista circular
- Inicializar
- Insertar al inicio
- Insertar al final
- Buscar
- Eliminar primera ocurrencia
- Invertir
- Vaciar

### 3.6 Sublistas
- Inicializar
- Insertar padre
- Seleccionar padre activo (buscar)
- Eliminar padre
- Insertar hijo sobre padre activo
- Eliminar hijo sobre padre activo
- Vaciar

## 4. Requerimientos de interfaz
1. Navegacion secuencial desde Home (`1 Secuenciales`) y submenu (`1..6`).
2. Visualizador unificado con cambio rapido de estructura (`TAB` y `1..7`).
3. Campos de entrada:
- `Valor` para todas las estructuras secuenciales.
- `Prioridad` solo para cola de prioridad.
4. Ayuda interna con `F1` sin perder contexto de sesion.
5. Scroll en vistas que exceden el area de dibujo:
- horizontal en cola/listas/cola de prioridad
- vertical en pila/sublistas segun contenido

## 5. Requerimientos tecnicos
### 5.1 Separacion por capas
- Dominio: TAD en `src/*.c` sin `raylib.h`.
- Aplicacion: `src/app_state.c` centraliza operaciones y mensajes.
- Presentacion: `src/main.c`, `src/ui.c`, `src/*_view.c`.

### 5.2 Invariantes
1. Unico `main` activo en `src/main.c`.
2. Toda mutacion pasa por API publica del TAD.
3. La UI no manipula nodos internos directamente.
4. El historial de codigo se acumula por operacion y puede limpiarse desde UI.

### 5.3 Build objetivo
- Compilar con `-std=c11 -Wall -Wextra -pedantic`.
- Integrar solo codigo activo (excluir `src/legacy` del build principal).

## 6. Complejidades de referencia docente
### 6.1 Pila
- Push/Pop: `O(1)`

### 6.2 Cola
- Encolar/Desencolar: `O(1)` amortizado segun implementacion enlazada

### 6.3 Cola de prioridad
- Insercion/Extraccion: depende de implementacion actual del TAD (lineal en lista enlazada)

### 6.4 Lista simple y lista circular
- Buscar/Eliminar por valor: `O(n)`
- Insertar inicio: `O(1)`
- Insertar final:
- Lista simple: `O(n)` si no hay puntero cola.
- Lista circular: `O(1)` con puntero cola.

### 6.5 Sublistas
- Buscar padre: `O(n)`
- Operaciones de hijo: `O(m)` sobre sublista activa

## 7. Criterios de aceptacion
1. La app compila y abre sin cierres inesperados.
2. Las seis estructuras secuenciales son accesibles por menu y atajos.
3. Operaciones validas e invalidas generan feedback coherente.
4. Vista, snippet, traza y complejidad se mantienen sincronizados tras cada operacion.
5. QA manual y QA de visualizacion pasan en la matriz de resoluciones definida.

## 8. Validacion recomendada
1. `make clean && make`
2. `doxygen Doxyfile`
3. Ejecutar `docs/qa-manual.md`
4. Ejecutar `docs/qa-visualizacion-ventanas.md`

## 9. Relacion con documentos anteriores
- El PDF `Sdd Visualizador Estructuras Datos Secuenciales C Raylib.pdf` se conserva
  como antecedente historico del modulo inicial.
- Este documento representa el alcance vigente del modulo secuencial integrado.
