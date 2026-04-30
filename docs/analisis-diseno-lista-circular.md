# Analisis y Diseno: Modulo Lista Circular

## 1. Objetivo de la funcionalidad
Agregar una nueva estructura visualizable llamada `Lista Circular` al ecosistema de VisualStruct, manteniendo:

- coherencia funcional con las estructuras existentes,
- separacion de capas (TAD, estado, presentacion),
- soporte pedagogico completo (vista grafica, snippet C, traza/complejidad, ayuda y QA).

## 2. Alcance funcional
La estructura `Lista Circular` debe permitir, desde UI y atajos:

1. Inicializar
2. Insertar al inicio
3. Insertar al final
4. Buscar valor
5. Eliminar primera ocurrencia
6. Invertir
7. Vaciar

Adicionalmente:

- visualizacion horizontal con scroll y barra draggable,
- indicador visual de circularidad (ultimo nodo enlaza al primero),
- integracion a menu principal, menu lateral y rotacion por `TAB`,
- historial de codigo C en panel derecho,
- traza y complejidad en panel inferior.

## 3. Requerimientos funcionales detallados
### RF-01: Registro de estructura en estado global
- `TipoEstructura` incorpora `ESTRUCTURA_LISTA_CIRCULAR`.
- `AppState` incorpora instancia `ListaCircular lista_circular`.

### RF-02: Operaciones de dominio
- API publica en `include/lista_circular.h`:
  - `lcir_inicializar`
  - `lcir_insertar_inicio`
  - `lcir_insertar_final`
  - `lcir_buscar_posiciones`
  - `lcir_eliminar_primero`
  - `lcir_invertir`
  - `lcir_vacia`
  - `lcir_contar`
  - `lcir_copiar_valores`
  - `lcir_formatear`
  - `lcir_destruir`

### RF-03: Integracion en AppState
- Cada operacion de alto nivel despacha al TAD circular cuando `estructura_activa` es circular.
- Mensajes, feedback y serial de operacion se mantienen consistentes con el resto de estructuras.

### RF-04: Visualizacion dedicada
- Nuevo modulo `lista_circular_view` renderiza nodos, enlaces y retorno circular.
- Debe resaltar insercion, busqueda y eliminacion con el mismo patron visual de la app.

### RF-05: Integracion pedagogica
- `code_viewer` incluye snippets representativos para operaciones de lista circular.
- `algorithm_trace` incluye pasos y complejidades por operacion.
- Ayuda interna y documentacion externa incorporan la nueva estructura.

## 4. Modelo de datos y decisiones de diseno
## 4.1 Estructura de datos seleccionada
Se usa lista simplemente enlazada circular con dos punteros:

- `cabeza`: primer nodo logico
- `cola`: ultimo nodo logico

Invariante principal:

- Si la lista no esta vacia, `cola->sgte == cabeza`.

## 4.2 Justificacion tecnica
- Permite `insertar_inicio` e `insertar_final` en `O(1)`.
- Facilita el dibujo de circularidad en UI (conexion explicita cola->cabeza).
- Conserva simplicidad del TAD para objetivos docentes.

## 4.3 Complejidades esperadas
- Inicializar: `O(1)`
- Insertar inicio: `O(1)`
- Insertar final: `O(1)`
- Buscar: `O(n)`
- Eliminar primera ocurrencia: `O(n)`
- Invertir: `O(n)`
- Vaciar: `O(n)`
- Espacio auxiliar en operaciones: `O(1)`

## 5. Diseno de interfaz y experiencia
## 5.1 Integracion de navegacion
- Home: quinta tarjeta para `Lista Circular`.
- Sidebar: nuevo boton `Lista Circular`.
- Atajos directos: `5` para seleccionar circular.
- Ciclo de `TAB`: ahora rota entre cinco estructuras.

## 5.2 Controles contextuales
Comparte el set de siete operaciones de `Lista`:

- `Inicializar (I)`
- `Inicio (Z)`
- `Final (A)`
- `Buscar (B)`
- `Eliminar (D)`
- `Invertir (R)`
- `Vaciar (V)`

## 5.3 Visualizacion en panel central
- Disposicion horizontal de nodos.
- Flechas entre nodos consecutivos.
- Conexion superior de retorno desde ultimo nodo hacia `HEAD`.
- Scroll horizontal con rueda y barra de arrastre.

## 6. Integracion por capas
1. Dominio:
  - `include/lista_circular.h`
  - `src/lista_circular.c`
2. Aplicacion:
  - `include/app_state.h`
  - `src/app_state.c`
3. Presentacion:
  - `include/lista_circular_view.h`
  - `src/lista_circular_view.c`
  - `src/main.c`
4. Pedagogia:
  - `src/code_viewer.c`
  - `src/algorithm_trace.c`
5. Build y docs:
  - `Makefile`
  - `README.md` y `docs/*.md`

## 7. Criterios de aceptacion
Se considera implementada correctamente cuando:

1. Compila sin errores con flags estrictos (`-Wall -Wextra -pedantic`).
2. La estructura aparece y navega correctamente en Home, Sidebar, `TAB` y tecla `5`.
3. Las 7 operaciones funcionan y actualizan estado, mensaje e historial.
4. La vista muestra retorno circular y permite inspeccion con scroll.
5. Snippets y trazas reflejan la operacion circular ejecutada.
6. Documentacion y QA quedan sincronizados.

## 8. Riesgos y mitigaciones
- Riesgo: romper contratos de listas existentes.
  - Mitigacion: no alterar API de `lista.h`; nuevo TAD aislado.
- Riesgo: inconsistencias de navegacion por pasar de 4 a 5 estructuras.
  - Mitigacion: revisar todos los `% 4` y atajos hardcodeados.
- Riesgo: degradacion visual en resoluciones bajas por tarjeta extra.
  - Mitigacion: ajustar ancho de cards y validar con QA de ventanas.

## 9. Plan de pruebas minimo
1. Pruebas funcionales de las 7 operaciones en lista circular.
2. Pruebas de navegacion (`1..5`, `TAB`, `H`, `F1`).
3. Pruebas de paneles pedagogicos (historial, traza, complejidad).
4. Pruebas de layout y scroll en resoluciones de la matriz QA.
