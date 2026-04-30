/**
 * @file grafo_controller.c
 * @brief Implementación del controlador del grafo
 */

#include "grafo_controller.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const float GRAFO_AUTOPLAY_INTERVALOS[] = {1.2f, 0.7f, 0.35f};

static GrafoPasoTipo grafo_controller_tipo_paso(const GrafoController *controller, int paso) {
    if (controller == NULL) {
        return GRAFO_PASO_CONSOLIDACION;
    }
    if (paso >= 0 && paso < controller->script_aristas_count) {
        return GRAFO_PASO_ARISTA;
    }
    if (paso >= 0 && paso < controller->script_vertices_count) {
        return GRAFO_PASO_VERTICE;
    }
    return GRAFO_PASO_CONSOLIDACION;
}

/** @brief Limpia buffers internos de secuencia de pasos. */
static void grafo_controller_limpiar_script(GrafoController *controller) {
    if (controller == NULL) {
        return;
    }
    controller->script_vertices_count = 0;
    controller->script_aristas_count = 0;
}

/** @brief Carga la secuencia real de pasos según algoritmo seleccionado. */
static void grafo_controller_construir_script(GrafoController *controller) {
    if (controller == NULL || controller->grafo_tad == NULL) {
        return;
    }

    grafo_controller_limpiar_script(controller);

    switch (controller->algoritmo_seleccionado) {
    case GRAFO_ALGO_BFS:
    case GRAFO_ALGO_DFS: {
        GrafoRecorrido r = (controller->algoritmo_seleccionado == GRAFO_ALGO_BFS)
                               ? grafo_bfs(controller->grafo_tad, controller->vertice_inicio)
                               : grafo_dfs(controller->grafo_tad, controller->vertice_inicio);
        if (r.estado == GRAFO_OK && r.vertices != NULL) {
            size_t i;
            size_t limite = (r.cantidad > 256) ? 256 : r.cantidad;
            for (i = 0; i < limite; i++) {
                controller->script_vertices[(int)i] = r.vertices[i];
            }
            controller->script_vertices_count = (int)limite;
        }
        grafo_liberar_recorrido(&r);
        break;
    }
    case GRAFO_ALGO_DIJKSTRA:
    case GRAFO_ALGO_BELLMAN_FORD: {
        GrafoCamino c = (controller->algoritmo_seleccionado == GRAFO_ALGO_DIJKSTRA)
                            ? grafo_dijkstra(controller->grafo_tad, controller->vertice_inicio,
                                             controller->vertice_destino)
                            : grafo_bellman_ford(controller->grafo_tad, controller->vertice_inicio,
                                                 controller->vertice_destino);
        if (c.estado == GRAFO_OK && c.existe && c.aristas != NULL) {
            int i;
            int limite_aristas = (int)((c.cantidad > 0) ? c.cantidad - 1 : 0);
            if (limite_aristas > 256) {
                limite_aristas = 256;
            }
            for (i = 0; i < limite_aristas; i++) {
                controller->script_aristas[i] = c.aristas[i];
            }
            controller->script_aristas_count = limite_aristas;

            if (limite_aristas > 0) {
                controller->script_vertices[0] = c.aristas[0].origen;
                for (i = 0; i < limite_aristas && i + 1 < 256; i++) {
                    controller->script_vertices[i + 1] = c.aristas[i].destino;
                }
                controller->script_vertices_count = limite_aristas + 1;
            }
        }
        grafo_liberar_camino(&c);
        break;
    }
    case GRAFO_ALGO_PRIM:
    case GRAFO_ALGO_KRUSKAL: {
        GrafoCamino c = (controller->algoritmo_seleccionado == GRAFO_ALGO_PRIM)
                            ? grafo_prim(controller->grafo_tad, controller->vertice_inicio)
                            : grafo_kruskal(controller->grafo_tad);
        if (c.estado == GRAFO_OK && c.aristas != NULL) {
            int i;
            int limite_aristas = (c.cantidad > 256) ? 256 : (int)c.cantidad;
            for (i = 0; i < limite_aristas; i++) {
                controller->script_aristas[i] = c.aristas[i];
            }
            controller->script_aristas_count = limite_aristas;
        }
        grafo_liberar_camino(&c);
        break;
    }
    default:
        break;
    }

    if (controller->script_vertices_count > 0) {
        controller->total_pasos = controller->script_vertices_count;
    } else if (controller->script_aristas_count > 0) {
        controller->total_pasos = controller->script_aristas_count;
    } else {
        controller->total_pasos = 1;
    }

    if (controller->algoritmo_seleccionado == GRAFO_ALGO_BELLMAN_FORD) {
        controller->total_pasos += 1;
    }
}

/** @brief Busca el índice de un vértice visual por id. */
static int grafo_controller_idx_vertice(const GrafoState *estado, int id) {
    int i;

    if (estado == NULL) {
        return -1;
    }
    for (i = 0; i < estado->cantidad_vertices; i++) {
        if (estado->vertices[i].id == id) {
            return i;
        }
    }
    return -1;
}

/** @brief Aplica resaltado visual según el paso actual del algoritmo. */
static void grafo_controller_aplicar_visual_paso(GrafoController *controller) {
    GrafoState *estado;
    int i;
    int current_idx;
    int paso;
    int dist_acumulada;
    int dist_previa[64];
    int count_dist = 0;

    if (controller == NULL) {
        return;
    }
    estado = &controller->estado_visual;
    grafo_state_reiniciar_visuales(estado);
    estado->algoritmo_activo = controller->algoritmo_seleccionado;
    estado->total_pasos = controller->total_pasos;
    estado->paso_algoritmo = controller->paso_actual;
    controller->paso_tipo_actual = grafo_controller_tipo_paso(controller, controller->paso_actual);
    controller->paso_mejora = false;
    controller->arista_actual_valida = false;

    if (controller->vertice_inicio >= 0) {
        grafo_state_establecer_vertice_estado(estado, controller->vertice_inicio,
                                              GRAFO_VÉRTICE_INICIAL);
    }
    if (controller->vertice_destino >= 0) {
        grafo_state_establecer_vertice_estado(estado, controller->vertice_destino,
                                              GRAFO_VÉRTICE_DESTINO);
    }

    paso = controller->paso_actual;
    if (paso < 0) {
        paso = 0;
    }
    if (controller->total_pasos > 0 && paso >= controller->total_pasos) {
        paso = controller->total_pasos - 1;
    }

    if (controller->script_vertices_count > 0) {
        int visitados = paso + 1;
        if (visitados > controller->script_vertices_count) {
            visitados = controller->script_vertices_count;
        }
        for (i = 0; i < visitados; i++) {
            grafo_state_marcar_vertice_visitado(estado, controller->script_vertices[i], i + 1);
        }
        current_idx = grafo_controller_idx_vertice(estado, controller->script_vertices[visitados - 1]);
        if (current_idx >= 0) {
            estado->vertices[current_idx].estado = GRAFO_VÉRTICE_ACTUAL;
        }
        if (visitados >= 2) {
            grafo_state_establecer_arista_estado(estado, controller->script_vertices[visitados - 2],
                                                 controller->script_vertices[visitados - 1],
                                                 GRAFO_ARISTA_CAMINO_MINIMO);
        }
    }

    if (controller->script_aristas_count > 0) {
        int destacadas = paso + 1;
        if (destacadas > controller->script_aristas_count) {
            destacadas = controller->script_aristas_count;
        }

        dist_acumulada = 0;
        if (controller->vertice_inicio >= 0 &&
            (controller->algoritmo_seleccionado == GRAFO_ALGO_DIJKSTRA ||
             controller->algoritmo_seleccionado == GRAFO_ALGO_BELLMAN_FORD)) {
            grafo_state_actualizar_distancia_vertice(estado, controller->vertice_inicio, 0, -1);
        }

        for (i = 0; i < estado->cantidad_vertices && i < 64; i++) {
            dist_previa[i] = estado->vertices[i].distancia;
            count_dist++;
        }

        for (i = 0; i < destacadas; i++) {
            GrafoAristaEstadoVisual est = GRAFO_ARISTA_RELAJADA;

            if (controller->algoritmo_seleccionado == GRAFO_ALGO_PRIM ||
                controller->algoritmo_seleccionado == GRAFO_ALGO_KRUSKAL) {
                est = GRAFO_ARISTA_MST;
            }

            grafo_state_establecer_arista_estado(estado, controller->script_aristas[i].origen,
                                                 controller->script_aristas[i].destino, est);
            grafo_state_marcar_vertice_visitado(estado, controller->script_aristas[i].origen,
                                                i + 1);
            grafo_state_marcar_vertice_visitado(estado, controller->script_aristas[i].destino,
                                                i + 1);

            if (controller->algoritmo_seleccionado == GRAFO_ALGO_DIJKSTRA ||
                controller->algoritmo_seleccionado == GRAFO_ALGO_BELLMAN_FORD) {
                int destino_idx = grafo_controller_idx_vertice(estado, controller->script_aristas[i].destino);

                dist_acumulada += controller->script_aristas[i].peso;
                if (destino_idx >= 0 && destino_idx < count_dist &&
                    (dist_previa[destino_idx] == 0 || dist_previa[destino_idx] > dist_acumulada)) {
                    grafo_state_establecer_arista_estado(estado, controller->script_aristas[i].origen,
                                                         controller->script_aristas[i].destino,
                                                         GRAFO_ARISTA_CAMINO_MINIMO);
                    controller->paso_mejora = (i == destacadas - 1);
                }
                grafo_state_actualizar_distancia_vertice(
                    estado, controller->script_aristas[i].destino, dist_acumulada,
                    controller->script_aristas[i].origen);
            }

            current_idx = grafo_controller_idx_vertice(estado, controller->script_aristas[i].destino);
            if (current_idx >= 0) {
                estado->vertices[current_idx].estado = GRAFO_VÉRTICE_ACTUAL;
            }
        }

        if (destacadas > 0) {
            controller->arista_actual_origen = controller->script_aristas[destacadas - 1].origen;
            controller->arista_actual_destino = controller->script_aristas[destacadas - 1].destino;
            controller->arista_actual_valida = true;
            grafo_state_establecer_arista_estado(estado, controller->arista_actual_origen,
                                                 controller->arista_actual_destino,
                                                 controller->paso_mejora ? GRAFO_ARISTA_CAMINO_MINIMO
                                                                         : GRAFO_ARISTA_RELAJADA);
        }
    }

    snprintf(estado->mensaje_estado, sizeof(estado->mensaje_estado), "%s - paso %d/%d [%s]",
             controller->algoritmo_seleccionado == GRAFO_ALGO_NINGUNO
                 ? "Edicion"
                 : (controller->algoritmo_seleccionado == GRAFO_ALGO_BFS
                        ? "BFS"
                        : (controller->algoritmo_seleccionado == GRAFO_ALGO_DFS
                               ? "DFS"
                               : (controller->algoritmo_seleccionado == GRAFO_ALGO_DIJKSTRA
                                      ? "Dijkstra"
                                      : (controller->algoritmo_seleccionado ==
                                                 GRAFO_ALGO_BELLMAN_FORD
                                             ? "Bellman-Ford"
                                             : (controller->algoritmo_seleccionado == GRAFO_ALGO_PRIM
                                                    ? "Prim"
                                                    : "Kruskal"))))),
             controller->paso_actual + 1, controller->total_pasos > 0 ? controller->total_pasos : 1,
             grafo_controller_tipo_paso_cadena(controller));
}

/* ============================================================================
 * Inicialización
 * ============================================================================ */

GrafoController grafo_controller_crear(Grafo *grafo, Rectangle area_renderizado) {
    GrafoController controller;
    controller.grafo_tad = grafo;
    controller.estado_visual = grafo_state_init();
    controller.vista = grafo_vista_init(&controller.estado_visual, area_renderizado);
    
    controller.modo = GRAFO_MODO_EDICION;
    controller.algoritmo_seleccionado = GRAFO_ALGO_NINGUNO;
    controller.vertice_inicio = -1;
    controller.vertice_destino = -1;
    
    controller.esta_ejecutando = false;
    controller.paso_actual = 0;
    controller.total_pasos = 0;
    controller.autoplay_activo = false;
    controller.autoplay_intervalo = GRAFO_AUTOPLAY_INTERVALOS[1];
    controller.autoplay_acumulado = 0.0f;
    controller.autoplay_velocidad_idx = 1;
    controller.paso_tipo_actual = GRAFO_PASO_CONSOLIDACION;
    controller.paso_mejora = false;
    controller.arista_actual_valida = false;
    controller.arista_actual_origen = -1;
    controller.arista_actual_destino = -1;
    controller.script_vertices_count = 0;
    controller.script_aristas_count = 0;
    
    strcpy(controller.mensaje_error, "");
    return controller;
}

void grafo_controller_destruir(GrafoController *controller) {
    if (!controller) return;
    
    grafo_state_destruir(&controller->estado_visual);
}

/* ============================================================================
 * Operaciones de Grafo
 * ============================================================================ */

bool grafo_controller_agregar_vertice(GrafoController *controller, int id_vertice) {
    if (!controller || !controller->grafo_tad) return false;
    
    GrafoEstado estado = grafo_insertar_vertice(controller->grafo_tad, id_vertice);
    if (estado != GRAFO_OK) {
        snprintf(controller->mensaje_error, sizeof(controller->mensaje_error),
                "Error al agregar vértice: %s", grafo_estado_cadena(estado));
        return false;
    }
    
    return true;
}

bool grafo_controller_eliminar_vertice(GrafoController *controller, int id_vertice) {
    if (!controller || !controller->grafo_tad) return false;
    
    GrafoEstado estado = grafo_eliminar_vertice(controller->grafo_tad, id_vertice);
    if (estado != GRAFO_OK) {
        snprintf(controller->mensaje_error, sizeof(controller->mensaje_error),
                "Error al eliminar vértice: %s", grafo_estado_cadena(estado));
        return false;
    }
    
    return true;
}

bool grafo_controller_agregar_arista(GrafoController *controller, 
                                    int id_origen, int id_destino, int peso) {
    if (!controller || !controller->grafo_tad) return false;
    
    GrafoEstado estado = grafo_insertar_arista(controller->grafo_tad, 
                                              id_origen, id_destino, peso);
    if (estado != GRAFO_OK) {
        snprintf(controller->mensaje_error, sizeof(controller->mensaje_error),
                "Error al agregar arista: %s", grafo_estado_cadena(estado));
        return false;
    }
    
    return true;
}

bool grafo_controller_eliminar_arista(GrafoController *controller, 
                                     int id_origen, int id_destino) {
    if (!controller || !controller->grafo_tad) return false;
    
    GrafoEstado estado = grafo_eliminar_arista(controller->grafo_tad, 
                                              id_origen, id_destino);
    if (estado != GRAFO_OK) {
        snprintf(controller->mensaje_error, sizeof(controller->mensaje_error),
                "Error al eliminar arista: %s", grafo_estado_cadena(estado));
        return false;
    }
    
    return true;
}

void grafo_controller_limpiar(GrafoController *controller) {
    if (!controller || !controller->grafo_tad) return;
    
    /* Destruir e inicializar nuevo grafo */
    grafo_destruir(&controller->grafo_tad);
    controller->grafo_tad = grafo_crear(false);  /* Grafo no dirigido por defecto */
    
    grafo_state_reiniciar_visuales(&controller->estado_visual);
    strcpy(controller->mensaje_error, "Grafo limpiado");
}

/* ============================================================================
 * Ejecución de Algoritmos
 * ============================================================================ */

bool grafo_controller_seleccionar_algoritmo(GrafoController *controller, 
                                            int codigo_algoritmo, 
                                            int id_inicio, int id_destino) {
    if (!controller) return false;
    
    controller->algoritmo_seleccionado = codigo_algoritmo;
    controller->vertice_inicio = id_inicio;
    controller->vertice_destino = id_destino;
    controller->total_pasos = 1;
    controller->paso_actual = 0;
    grafo_controller_limpiar_script(controller);
    controller->estado_visual.algoritmo_activo = codigo_algoritmo;
    controller->estado_visual.total_pasos = controller->total_pasos;
    controller->estado_visual.paso_algoritmo = controller->paso_actual;
    
    return true;
}

bool grafo_controller_iniciar_algoritmo(GrafoController *controller) {
    if (!controller || !controller->grafo_tad) return false;
    if (controller->algoritmo_seleccionado == GRAFO_ALGO_NINGUNO) {
        strcpy(controller->mensaje_error, "No hay algoritmo seleccionado");
        return false;
    }
    
    controller->esta_ejecutando = true;
    controller->modo = GRAFO_MODO_ALGORITMO;
    controller->paso_actual = 0;
    controller->autoplay_acumulado = 0.0f;
    grafo_controller_construir_script(controller);
    grafo_controller_aplicar_visual_paso(controller);
    
    return true;
}

bool grafo_controller_paso_siguiente(GrafoController *controller) {
    if (!controller || controller->total_pasos <= 0) return false;

    if (!controller->esta_ejecutando) {
        controller->esta_ejecutando = true;
    }
    
    if (controller->paso_actual >= controller->total_pasos - 1) {
        controller->esta_ejecutando = false;
        return false;
    }

    controller->paso_actual++;
    controller->estado_visual.paso_algoritmo = controller->paso_actual;
    grafo_controller_aplicar_visual_paso(controller);
    
    return true;
}

bool grafo_controller_paso_anterior(GrafoController *controller) {
    if (!controller) return false;
    
    if (controller->paso_actual > 0) {
        controller->paso_actual--;
        controller->esta_ejecutando = true;
        controller->estado_visual.paso_algoritmo = controller->paso_actual;
        grafo_controller_aplicar_visual_paso(controller);
        return true;
    }
    return false;
}

void grafo_controller_pausar(GrafoController *controller) {
    if (!controller) return;
    controller->modo = GRAFO_MODO_PAUSA;
}

void grafo_controller_reanudar(GrafoController *controller) {
    if (!controller) return;
    controller->modo = GRAFO_MODO_ALGORITMO;
}

void grafo_controller_reiniciar(GrafoController *controller) {
    if (!controller) return;
    
    controller->esta_ejecutando = false;
    controller->paso_actual = 0;
    controller->autoplay_acumulado = 0.0f;
    grafo_controller_aplicar_visual_paso(controller);
}

void grafo_controller_ir_inicio(GrafoController *controller) {
    grafo_controller_reiniciar(controller);
}

void grafo_controller_ir_final(GrafoController *controller) {
    if (!controller) return;
    if (controller->total_pasos <= 0) {
        controller->paso_actual = 0;
    } else {
        controller->paso_actual = controller->total_pasos - 1;
    }
    controller->esta_ejecutando = false;
    controller->autoplay_activo = false;
    grafo_controller_aplicar_visual_paso(controller);
}

void grafo_controller_toggle_autoplay(GrafoController *controller) {
    if (!controller) return;
    controller->autoplay_activo = !controller->autoplay_activo;
    controller->autoplay_acumulado = 0.0f;
}

void grafo_controller_cambiar_velocidad(GrafoController *controller) {
    if (!controller) return;
    controller->autoplay_velocidad_idx = (controller->autoplay_velocidad_idx + 1) % 3;
    controller->autoplay_intervalo =
        GRAFO_AUTOPLAY_INTERVALOS[controller->autoplay_velocidad_idx];
}

void grafo_controller_actualizar(GrafoController *controller, float delta_time) {
    if (!controller || !controller->autoplay_activo || controller->total_pasos <= 0) {
        return;
    }

    controller->autoplay_acumulado += delta_time;
    if (controller->autoplay_acumulado >= controller->autoplay_intervalo) {
        controller->autoplay_acumulado = 0.0f;
        if (!grafo_controller_paso_siguiente(controller)) {
            controller->autoplay_activo = false;
        }
    }
}

/* ============================================================================
 * Interacción
 * ============================================================================ */

void grafo_controller_procesar_mouse(GrafoController *controller, 
                                    Vector2 mouse_pos, bool boton_pulsado) {
    if (!controller || !boton_pulsado) return;
    if (controller->modo != GRAFO_MODO_EDICION) return;
    
    int id_vertice = grafo_vista_detectar_vertice(&controller->vista, mouse_pos);
    if (id_vertice >= 0) {
        grafo_state_establecer_vertice_estado(&controller->estado_visual, 
                                             id_vertice, 
                                             GRAFO_VÉRTICE_ACTUAL);
    }
}

void grafo_controller_establecer_modo(GrafoController *controller, GrafoControllerModo nuevo_modo) {
    if (!controller) return;
    controller->modo = nuevo_modo;
}

/* ============================================================================
 * Consultas
 * ============================================================================ */

const char* grafo_controller_obtener_error(const GrafoController *controller) {
    if (!controller) return "Controlador nulo";
    return controller->mensaje_error;
}

void grafo_controller_dibujar(GrafoController *controller) {
    if (!controller) return;
    grafo_vista_dibujar(&controller->vista);
}

void grafo_controller_actualizar_area(GrafoController *controller, Rectangle nueva_area) {
    if (!controller) return;
    grafo_vista_actualizar_area(&controller->vista, nueva_area);
}

const char *grafo_controller_tipo_paso_cadena(const GrafoController *controller) {
    if (controller == NULL) {
        return "sin paso";
    }
    switch (controller->paso_tipo_actual) {
    case GRAFO_PASO_VERTICE:
        return "seleccion de vertice";
    case GRAFO_PASO_ARISTA:
        return controller->paso_mejora ? "relajacion con mejora" : "relajacion";
    default:
        return "consolidacion";
    }
}
