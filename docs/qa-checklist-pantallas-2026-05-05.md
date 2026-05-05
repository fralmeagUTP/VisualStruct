# Matriz QA por Pantalla - 2026-05-05

## Objetivo
Estandarizar la prueba manual de la app completa por pantalla con enfoque de usuario final:
- navegacion
- legibilidad
- coherencia de estado
- funcionalidad operativa

## Alcance
Pantallas incluidas:
1. Inicio raiz (`SCREEN_HOME_ROOT`)
2. Inicio secuenciales (`SCREEN_HOME_SECUENCIALES`)
3. Inicio grafos (`SCREEN_HOME_GRAFOS`)
4. Visualizador (`SCREEN_VISUALIZER`)
5. Ayuda (`SCREEN_HELP`)

## Convenciones
- Resultado: `PASS` / `FAIL` / `NA`
- Severidad:
  - `S1`: critica (bloquea uso)
  - `S2`: alta (rompe flujo principal)
  - `S3`: media (degrada UX o confunde)
  - `S4`: baja (detalle visual menor)

---

## 1) Pantalla Inicio Raiz

| ID | Caso | Pasos | Resultado esperado | Severidad si falla | Estado |
|---|---|---|---|---|---|
| HR-01 | Carga visual | Abrir `visualstruct.exe` | Header, footer y 2 tarjetas visibles, sin solapamientos | S2 | |
| HR-02 | Navegacion teclado | Flechas izquierda/derecha sobre tarjetas | Cambio de tarjeta seleccionada | S3 | |
| HR-03 | Accion Enter | Enter sobre tarjeta seleccionada | Abre submenu correspondiente | S2 | |
| HR-04 | Atajo numerico | `1` y `2` | `1` abre secuenciales, `2` abre grafos | S2 | |
| HR-05 | Ayuda global | `F1` | Abre ayuda sin cerrar app | S2 | |

---

## 2) Pantalla Inicio Secuenciales

| ID | Caso | Pasos | Resultado esperado | Severidad si falla | Estado |
|---|---|---|---|---|---|
| HS-01 | Render tarjetas | Entrar a secuenciales | 6 tarjetas legibles, sin texto montado | S3 | |
| HS-02 | Seleccion por teclado | Flechas + Enter | Cambia seleccion y abre visualizador correcto | S2 | |
| HS-03 | Atajos directos | Teclas `1..6` | Abre modulo correspondiente | S2 | |
| HS-04 | Volver | `ESC` o `H` | Regresa a inicio raiz | S3 | |
| HS-05 | Ayuda | `F1` | Abre/cierra ayuda conservando contexto | S2 | |

---

## 3) Pantalla Inicio Grafos

| ID | Caso | Pasos | Resultado esperado | Severidad si falla | Estado |
|---|---|---|---|---|---|
| HG-01 | Render inicial | Entrar a grafos | Cards y bloque de seleccion rapida legibles | S3 | |
| HG-02 | Crear/Editar | Seleccionar card crear | Entra al visualizador de grafos | S2 | |
| HG-03 | Cargar demo | Seleccionar card demo | Entra al visualizador con demo cargada | S2 | |
| HG-04 | Atajos algoritmos | `4..9` | Preselecciona/ejecuta BFS, DFS, Dijkstra, Bellman-Ford, Prim, Kruskal | S2 | |
| HG-05 | Volver | `ESC` o `H` | Regresa a inicio raiz | S3 | |

---

## 4) Pantalla Visualizador (General)

| ID | Caso | Pasos | Resultado esperado | Severidad si falla | Estado |
|---|---|---|---|---|---|
| HV-01 | Layout base | Entrar al visualizador | Sidebar, centro, derecho e inferior sin solaparse | S1 | |
| HV-02 | Cambio de estructura | `TAB` o botones sidebar | Cambia modulo sin cierres ni texto corrupto | S2 | |
| HV-03 | Inputs numericos | Foco en inputs, escribir, Enter | Cursor visible, acepta numerico, valida rango | S2 | |
| HV-04 | Mensajeria | Ejecutar operacion valida/invalida | Mensaje contextual claro y coherente | S3 | |
| HV-05 | Ayuda | `F1` desde visualizador | Abre/cierra ayuda sin perder estado | S2 | |

### 4.1 Submodulo Grafo - Construccion

| ID | Caso | Pasos | Resultado esperado | Severidad si falla | Estado |
|---|---|---|---|---|---|
| GV-C01 | Inicializar | Click `Inicializar` | Grafo limpio y estado coherente | S2 | |
| GV-C02 | Vertices | Insertar varios vertices | Se dibujan correctamente | S2 | |
| GV-C03 | Aristas | Definir origen/destino/peso y crear arista | Se crea arista con peso correcto | S2 | |
| GV-C04 | Modo dirigido | Toggle dirigido/no dirigido | Flechas coherentes en dirigido | S2 | |
| GV-C05 | Demo | Definir `Valor` y cargar demo | Respeta nodos y mantiene conectividad base | S2 | |

### 4.2 Submodulo Grafo - Recorridos

| ID | Caso | Pasos | Resultado esperado | Severidad si falla | Estado |
|---|---|---|---|---|---|
| GV-R01 | Inicio de recorrido | Definir inicio valido | Se acepta y se usa en BFS/DFS | S2 | |
| GV-R02 | BFS/DFS | Ejecutar ambos algoritmos | Lista de vertices recorridos visible y coherente | S2 | |
| GV-R03 | Paso a paso | `Anterior/Siguiente/Reiniciar` | Navega pasos sin inconsistencias | S3 | |
| GV-R04 | Autoplay | ON/OFF | Cambia estado y avanza correctamente | S3 | |

### 4.3 Submodulo Grafo - Caminos Minimos

| ID | Caso | Pasos | Resultado esperado | Severidad si falla | Estado |
|---|---|---|---|---|---|
| GV-M01 | Parametros | Definir origen, destino y peso | Campos operativos y validados | S2 | |
| GV-M02 | Dijkstra | Ejecutar camino minimo | Ruta/costo coherentes con pesos | S2 | |
| GV-M03 | Bellman-Ford | Ejecutar camino minimo | Ruta/costo coherentes; errores negativos bien reportados | S2 | |
| GV-M04 | Resumen camino | Revisar panel derecho | Muestra algoritmo, estado, costo y ruta | S3 | |

### 4.4 Submodulo Grafo - MST

| ID | Caso | Pasos | Resultado esperado | Severidad si falla | Estado |
|---|---|---|---|---|---|
| GV-T01 | Prim | Ejecutar desde inicio valido | Genera arbol y costo | S2 | |
| GV-T02 | Kruskal | Ejecutar algoritmo | Genera arbol y costo | S2 | |
| GV-T03 | Control de pasos | Paso+/Paso-/Reiniciar | Estado sincronizado en traza y grafo | S3 | |

---

## 5) Pantalla Ayuda

| ID | Caso | Pasos | Resultado esperado | Severidad si falla | Estado |
|---|---|---|---|---|---|
| HY-01 | Apertura | `F1` desde cualquier pantalla | Abre ayuda con contenido completo | S2 | |
| HY-02 | Scroll | Rueda mouse en viewport | Desplaza contenido sin saltos | S3 | |
| HY-03 | Volver | Boton volver / `F1` / `ESC` | Regresa a pantalla previa | S2 | |
| HY-04 | Legibilidad | Revisar titulos y texto | Sin solapamientos ni cortes | S3 | |

---

## Registro de Hallazgos

| ID Hallazgo | Pantalla | Caso | Descripcion | Severidad | Estado | Evidencia |
|---|---|---|---|---|---|---|
| H-2026-05-05-01 | Visualizador/Home/Ayuda | HV-01 | Compactacion de paneles de grafo afectaba pantallas fuera del visualizador | S1 | RESUELTO | `src/main.c` (condicion `screen_mode == SCREEN_VISUALIZER`) |

## Resultado de la Ronda Actual
- Prueba manual guiada: completada por flujo principal.
- Hallazgos criticos abiertos: `0`.
- Hallazgos criticos resueltos en la ronda: `1`.
