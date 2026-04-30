#include "grafo_pedagogy.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ui.h"

static const char *grafo_algoritmo_nombre_local(int algoritmo) {
    switch (algoritmo) {
    case GRAFO_ALGO_BFS:
        return "BFS";
    case GRAFO_ALGO_DFS:
        return "DFS";
    case GRAFO_ALGO_DIJKSTRA:
        return "Dijkstra";
    case GRAFO_ALGO_BELLMAN_FORD:
        return "Bellman-Ford";
    case GRAFO_ALGO_PRIM:
        return "Prim";
    case GRAFO_ALGO_KRUSKAL:
        return "Kruskal";
    default:
        return "Edicion";
    }
}

static int grafo_idx_id(const int *ids, int count, int id) {
    int i;

    for (i = 0; i < count; i++) {
        if (ids[i] == id) {
            return i;
        }
    }
    return -1;
}

static void grafo_ordenar_ids(int *ids, int count) {
    int i;

    for (i = 1; i < count; i++) {
        int key = ids[i];
        int j = i - 1;

        while (j >= 0 && ids[j] > key) {
            ids[j + 1] = ids[j];
            j--;
        }
        ids[j + 1] = key;
    }
}

static int grafo_recolectar_ids(const AppState *app, int *ids, int max_ids) {
    int *vertices = NULL;
    size_t vertices_count = 0;
    int count = 0;
    int i;

    if (ids == NULL || max_ids <= 0) {
        return 0;
    }

    if (grafo_obtener_vertices(app->grafo, &vertices, &vertices_count) == GRAFO_OK &&
        vertices != NULL) {
        for (i = 0; i < (int)vertices_count && count < max_ids; i++) {
            int id = vertices[i];
            if (grafo_idx_id(ids, count, id) < 0) {
                ids[count++] = id;
            }
        }
        free(vertices);
    }

    for (i = 0; i < app->grafo_controller_state.script_vertices_count && count < max_ids; i++) {
        int id = app->grafo_controller_state.script_vertices[i];
        if (grafo_idx_id(ids, count, id) < 0) {
            ids[count++] = id;
        }
    }

    grafo_ordenar_ids(ids, count);
    return count;
}

static void grafo_dist_pred_hasta_paso(const AppState *app, int paso_idx, const int *ids, int count,
                                       int *dist, int *pred) {
    int i;
    int end_step;

    for (i = 0; i < count; i++) {
        dist[i] = 1000000000;
        pred[i] = -1;
    }

    i = grafo_idx_id(ids, count, app->grafo_vertice_inicio);
    if (i >= 0) {
        dist[i] = 0;
    }

    end_step = paso_idx;
    if (end_step >= app->grafo_controller_state.script_aristas_count) {
        end_step = app->grafo_controller_state.script_aristas_count - 1;
    }

    for (i = 0; i <= end_step; i++) {
        const GrafoArista *a;
        int io;
        int id;

        if (i < 0) {
            break;
        }
        a = &app->grafo_controller_state.script_aristas[i];
        io = grafo_idx_id(ids, count, a->origen);
        id = grafo_idx_id(ids, count, a->destino);
        if (io >= 0 && id >= 0 && dist[io] < 1000000000) {
            int cand = dist[io] + a->peso;
            if (cand < dist[id]) {
                dist[id] = cand;
                pred[id] = a->origen;
            }
        }
    }
}

static void grafo_describir_paso(const AppState *app, int paso_idx, int total, char *desc,
                                 size_t desc_size, char *vars, size_t vars_size) {
    const GrafoController *gc = &app->grafo_controller_state;
    int dist_acum;
    GrafoPasoMetricas metricas;
    int j;
    char tabla_dist[512];
    char camino_parcial[256];
    char cerrados[192];

    metricas = grafo_obtener_metricas_paso(app, paso_idx);
    grafo_camino_parcial_paso(app, paso_idx, camino_parcial, sizeof(camino_parcial));
    grafo_cerrados_paso(app, paso_idx, cerrados, sizeof(cerrados));

    if (paso_idx < gc->script_aristas_count) {
        const GrafoArista *a = &gc->script_aristas[paso_idx];
        dist_acum = 0;
        for (j = 0; j <= paso_idx && j < gc->script_aristas_count; j++) {
            dist_acum += gc->script_aristas[j].peso;
        }
        switch (app->grafo_algoritmo_seleccionado) {
        case GRAFO_ALGO_DIJKSTRA:
            snprintf(desc, desc_size, "Relajar arista %d -> %d (peso=%d)", a->origen, a->destino,
                     a->peso);
            grafo_tabla_distancias_paso(app, paso_idx, tabla_dist, sizeof(tabla_dist));
            snprintf(vars, vars_size,
                     "paso=%d/%d\n"
                     "tipo=%s\n"
                     "dist[%d]=%d | pred[%d]=%d\n"
                     "mejoras=%d | sin_cambio=%d | empeora=%d\n"
                     "%s\n"
                     "%s\n"
                     "inicio=%d | destino=%d\n"
                     "%s",
                     paso_idx + 1, total, grafo_tipo_paso_label(app), a->destino, dist_acum,
                     a->destino, a->origen, metricas.mejoras, metricas.sin_cambio,
                     metricas.empeora, camino_parcial, cerrados, app->grafo_vertice_inicio,
                     app->grafo_vertice_destino, tabla_dist);
            break;
        case GRAFO_ALGO_BELLMAN_FORD:
            snprintf(desc, desc_size,
                     "Aplicar relajacion Bellman-Ford en %d -> %d (peso=%d)", a->origen,
                     a->destino, a->peso);
            grafo_tabla_distancias_paso(app, paso_idx, tabla_dist, sizeof(tabla_dist));
            snprintf(vars, vars_size,
                     "paso=%d/%d\n"
                     "tipo=%s\n"
                     "dist[%d]=%d | pred[%d]=%d\n"
                     "mejoras=%d | sin_cambio=%d | empeora=%d\n"
                     "%s\n"
                     "%s\n"
                     "ronda=%d/%d | control ciclos negativos\n"
                     "%s",
                     paso_idx + 1, total, grafo_tipo_paso_label(app), a->destino, dist_acum,
                     a->destino, a->origen, metricas.mejoras, metricas.sin_cambio,
                     metricas.empeora, camino_parcial, cerrados, paso_idx + 1,
                     app->grafo_controller_state.script_aristas_count > 0
                         ? app->grafo_controller_state.script_aristas_count
                         : 1,
                     tabla_dist);
            break;
        case GRAFO_ALGO_PRIM:
            snprintf(desc, desc_size,
                     "Seleccionar arista minima candidata del MST: %d -> %d (peso=%d)",
                     a->origen, a->destino, a->peso);
            snprintf(vars, vars_size,
                     "paso=%d/%d | costo_mst_parcial=%d | aristas_mst=%d", paso_idx + 1, total,
                     dist_acum, paso_idx + 1);
            break;
        case GRAFO_ALGO_KRUSKAL:
            snprintf(desc, desc_size,
                     "Evaluar arista ordenada %d -> %d (peso=%d) y verificar ciclo", a->origen,
                     a->destino, a->peso);
            snprintf(vars, vars_size,
                     "paso=%d/%d | costo_mst_parcial=%d | componente_union=%d", paso_idx + 1,
                     total, dist_acum, paso_idx + 1);
            break;
        default:
            snprintf(desc, desc_size, "Procesar arista %d -> %d (peso=%d)", a->origen,
                     a->destino, a->peso);
            snprintf(vars, vars_size,
                     "paso=%d/%d | inicio=%d destino=%d | aristas_script=%d | vertices_script=%d",
                     paso_idx + 1, total, app->grafo_vertice_inicio, app->grafo_vertice_destino,
                     gc->script_aristas_count, gc->script_vertices_count);
            break;
        }
        return;
    }

    if (paso_idx < gc->script_vertices_count) {
        int v = gc->script_vertices[paso_idx];
        switch (app->grafo_algoritmo_seleccionado) {
        case GRAFO_ALGO_BFS:
            snprintf(desc, desc_size, "Extraer de cola y visitar vertice %d", v);
            break;
        case GRAFO_ALGO_DFS:
            snprintf(desc, desc_size, "Extraer de pila y visitar vertice %d", v);
            break;
        case GRAFO_ALGO_DIJKSTRA:
            snprintf(desc, desc_size, "Seleccionar vertice actual %d con menor distancia", v);
            snprintf(vars, vars_size,
                     "paso=%d/%d\n"
                     "tipo=%s\n"
                     "vertice=%d\n"
                     "mejoras=%d | sin_cambio=%d | empeora=%d\n"
                     "%s\n"
                     "%s\n"
                     "inicio=%d | destino=%d",
                     paso_idx + 1, total, grafo_tipo_paso_label(app), v, metricas.mejoras,
                     metricas.sin_cambio, metricas.empeora, camino_parcial, cerrados,
                     app->grafo_vertice_inicio, app->grafo_vertice_destino);
            break;
        case GRAFO_ALGO_BELLMAN_FORD:
            snprintf(desc, desc_size, "Revisar estado de distancias alrededor de %d", v);
            snprintf(vars, vars_size,
                     "paso=%d/%d\n"
                     "tipo=%s\n"
                     "vertice=%d\n"
                     "mejoras=%d | sin_cambio=%d | empeora=%d\n"
                     "%s\n"
                     "%s\n"
                     "control ciclos negativos",
                     paso_idx + 1, total, grafo_tipo_paso_label(app), v, metricas.mejoras,
                     metricas.sin_cambio, metricas.empeora, camino_parcial, cerrados);
            break;
        case GRAFO_ALGO_PRIM:
        case GRAFO_ALGO_KRUSKAL:
            snprintf(desc, desc_size, "Actualizar conjunto del arbol con vertice %d", v);
            break;
        default:
            snprintf(desc, desc_size, "Visitar vertice %d", v);
            break;
        }
        if (app->grafo_algoritmo_seleccionado != GRAFO_ALGO_DIJKSTRA &&
            app->grafo_algoritmo_seleccionado != GRAFO_ALGO_BELLMAN_FORD) {
            snprintf(vars, vars_size,
                     "paso=%d/%d | inicio=%d destino=%d | aristas_script=%d | vertices_script=%d",
                     paso_idx + 1, total, app->grafo_vertice_inicio, app->grafo_vertice_destino,
                     gc->script_aristas_count, gc->script_vertices_count);
        }
        return;
    }

    snprintf(desc, desc_size, "Consolidar resultados del algoritmo");
    if (app->grafo_algoritmo_seleccionado == GRAFO_ALGO_BELLMAN_FORD) {
        snprintf(desc, desc_size, "Verificar ciclo negativo y consolidar resultado final");
    }
    snprintf(vars, vars_size, "paso=%d/%d\ntipo=%s\nresultado=%s", paso_idx + 1, total,
             grafo_tipo_paso_label(app), app->mensaje_operacion);
}

int grafo_linea_desde_paso(int algoritmo, int paso_actual, int total_pasos) {
    static const int mapa_bfs[5] = {1, 2, 4, 8, 11};
    static const int mapa_dfs[5] = {1, 2, 4, 8, 11};
    static const int mapa_dijkstra[5] = {1, 2, 6, 9, 11};
    static const int mapa_bf[5] = {1, 2, 5, 6, 10};
    static const int mapa_prim[5] = {1, 2, 4, 5, 7};
    static const int mapa_kruskal[5] = {1, 2, 4, 5, 8};
    int bucket;

    if (total_pasos <= 1) {
        bucket = 0;
    } else {
        if (paso_actual < 0) {
            paso_actual = 0;
        }
        if (paso_actual >= total_pasos) {
            paso_actual = total_pasos - 1;
        }
        bucket = (paso_actual * 4) / (total_pasos - 1);
        if (bucket < 0) {
            bucket = 0;
        }
        if (bucket > 4) {
            bucket = 4;
        }
    }

    switch (algoritmo) {
    case GRAFO_ALGO_BFS:
        return mapa_bfs[bucket];
    case GRAFO_ALGO_DFS:
        return mapa_dfs[bucket];
    case GRAFO_ALGO_DIJKSTRA:
        return mapa_dijkstra[bucket];
    case GRAFO_ALGO_BELLMAN_FORD:
        return mapa_bf[bucket];
    case GRAFO_ALGO_PRIM:
        return mapa_prim[bucket];
    case GRAFO_ALGO_KRUSKAL:
        return mapa_kruskal[bucket];
    default:
        return 1;
    }
}

int grafo_vertice_activo_paso(const AppState *app, int paso_idx) {
    const GrafoController *gc = &app->grafo_controller_state;

    if (paso_idx >= 0 && paso_idx < gc->script_vertices_count) {
        return gc->script_vertices[paso_idx];
    }
    if (paso_idx >= 0 && paso_idx < gc->script_aristas_count) {
        return gc->script_aristas[paso_idx].destino;
    }
    return -1;
}

const char *grafo_tipo_paso_label(const AppState *app) {
    return grafo_controller_tipo_paso_cadena(&app->grafo_controller_state);
}

bool grafo_hay_mejora_paso(const AppState *app, int paso_idx) {
    return grafo_contar_mejoras_paso(app, paso_idx) > 0;
}

GrafoPasoMetricas grafo_obtener_metricas_paso(const AppState *app, int paso_idx) {
    GrafoPasoMetricas metricas = {0, 0, 0};
    int ids[64];
    int dist[64];
    int pred[64];
    int dist_prev[64];
    int pred_prev[64];
    int count;
    int i;

    count = grafo_recolectar_ids(app, ids, 64);
    if (count <= 0) {
        return metricas;
    }

    grafo_dist_pred_hasta_paso(app, paso_idx, ids, count, dist, pred);
    grafo_dist_pred_hasta_paso(app, paso_idx - 1, ids, count, dist_prev, pred_prev);

    for (i = 0; i < count; i++) {
        if ((dist_prev[i] >= 1000000000 && dist[i] < 1000000000) ||
            (dist_prev[i] < 1000000000 && dist[i] < dist_prev[i])) {
            metricas.mejoras++;
        } else if ((dist_prev[i] < 1000000000 && dist[i] > dist_prev[i]) ||
                   (dist_prev[i] < 1000000000 && dist[i] >= 1000000000)) {
            metricas.empeora++;
        } else {
            metricas.sin_cambio++;
        }
    }

    return metricas;
}

int grafo_contar_mejoras_paso(const AppState *app, int paso_idx) {
    return grafo_obtener_metricas_paso(app, paso_idx).mejoras;
}

int grafo_contar_sin_cambio_paso(const AppState *app, int paso_idx) {
    return grafo_obtener_metricas_paso(app, paso_idx).sin_cambio;
}

int grafo_contar_empeora_paso(const AppState *app, int paso_idx) {
    return grafo_obtener_metricas_paso(app, paso_idx).empeora;
}

void grafo_camino_parcial_paso(const AppState *app, int paso_idx, char *out, size_t out_size) {
    int ids[64];
    int dist[64];
    int pred[64];
    int cadena[64];
    int count;
    int actual;
    int cadena_count = 0;
    int i;

    if (out == NULL || out_size == 0) {
        return;
    }
    out[0] = '\0';

    count = grafo_recolectar_ids(app, ids, 64);
    if (count <= 0) {
        return;
    }
    grafo_dist_pred_hasta_paso(app, paso_idx, ids, count, dist, pred);
    actual = grafo_vertice_activo_paso(app, paso_idx);
    if (actual < 0) {
        actual = app->grafo_vertice_destino;
    }
    while (actual >= 0 && cadena_count < 64) {
        int idx = grafo_idx_id(ids, count, actual);
        cadena[cadena_count++] = actual;
        if (idx < 0 || pred[idx] < 0) {
            break;
        }
        actual = pred[idx];
    }

    strncat(out, "camino=", out_size - strlen(out) - 1);
    for (i = cadena_count - 1; i >= 0; i--) {
        char tmp[24];
        snprintf(tmp, sizeof(tmp), "%d", cadena[i]);
        strncat(out, tmp, out_size - strlen(out) - 1);
        if (i > 0) {
            strncat(out, "->", out_size - strlen(out) - 1);
        }
    }
}

void grafo_cerrados_paso(const AppState *app, int paso_idx, char *out, size_t out_size) {
    int limite;
    int i;

    if (out == NULL || out_size == 0) {
        return;
    }
    out[0] = '\0';
    strncat(out, "cerrados={", out_size - strlen(out) - 1);

    limite = paso_idx + 1;
    if (limite > app->grafo_controller_state.script_vertices_count) {
        limite = app->grafo_controller_state.script_vertices_count;
    }
    for (i = 0; i < limite; i++) {
        char tmp[24];
        snprintf(tmp, sizeof(tmp), "%d", app->grafo_controller_state.script_vertices[i]);
        strncat(out, tmp, out_size - strlen(out) - 1);
        if (i + 1 < limite) {
            strncat(out, ",", out_size - strlen(out) - 1);
        }
    }
    strncat(out, "}", out_size - strlen(out) - 1);
}

void grafo_tabla_distancias_paso(const AppState *app, int paso_idx, char *out, size_t out_size) {
    int ids[64];
    int dist[64];
    int pred[64];
    int count;
    int i;

    if (out == NULL || out_size == 0) {
        return;
    }
    out[0] = '\0';

    if (app->grafo_algoritmo_seleccionado != GRAFO_ALGO_DIJKSTRA &&
        app->grafo_algoritmo_seleccionado != GRAFO_ALGO_BELLMAN_FORD) {
        return;
    }

    count = grafo_recolectar_ids(app, ids, 64);
    grafo_dist_pred_hasta_paso(app, paso_idx, ids, count, dist, pred);

    snprintf(out, out_size, "tabla: ");
    for (i = 0; i < count; i++) {
        char row[64];
        if (dist[i] >= 1000000000) {
            snprintf(row, sizeof(row), "v%d=INF(pred=-)", ids[i]);
        } else if (pred[i] < 0) {
            snprintf(row, sizeof(row), "v%d=%d(pred=-)", ids[i], dist[i]);
        } else {
            snprintf(row, sizeof(row), "v%d=%d(pred=%d)", ids[i], dist[i], pred[i]);
        }
        if (i > 0) {
            strncat(out, "; ", out_size - strlen(out) - 1);
        }
        strncat(out, row, out_size - strlen(out) - 1);
    }
}

void grafo_tabla_distancias_multiline(const AppState *app, int paso_idx, char *out,
                                      size_t out_size) {
    char tabla[512];
    size_t i;
    size_t j;
    const char *start;

    if (out == NULL || out_size == 0) {
        return;
    }
    out[0] = '\0';

    grafo_tabla_distancias_paso(app, paso_idx, tabla, sizeof(tabla));
    if (tabla[0] == '\0') {
        return;
    }

    start = tabla;
    if (strncmp(tabla, "tabla: ", 7) == 0) {
        start = tabla + 7;
    }

    j = 0;
    for (i = 0; start[i] != '\0' && j + 1 < out_size; i++) {
        if (start[i] == ';') {
            out[j++] = '\n';
            if (start[i + 1] == ' ') {
                i++;
            }
        } else {
            out[j++] = start[i];
        }
    }
    out[j] = '\0';
}

void grafo_dibujar_tabla_distancias(const AppState *app, Rectangle viewport, int paso_idx) {
    int ids[64];
    int dist[64];
    int pred[64];
    int dist_prev[64];
    int pred_prev[64];
    int count;
    int i;
    int line_height = 18;
    int vertice_activo;

    count = grafo_recolectar_ids(app, ids, 64);
    if (count <= 0) {
        return;
    }

    grafo_dist_pred_hasta_paso(app, paso_idx, ids, count, dist, pred);
    grafo_dist_pred_hasta_paso(app, paso_idx - 1, ids, count, dist_prev, pred_prev);
    vertice_activo = grafo_vertice_activo_paso(app, paso_idx);

    BeginScissorMode((int)viewport.x, (int)viewport.y, (int)viewport.width, (int)viewport.height);
    for (i = 0; i < count; i++) {
        char line[96];
        int y;
        bool fila_activa = false;
        bool mejora = false;

        if (dist[i] >= 1000000000) {
            snprintf(line, sizeof(line), "v%d=INF(pred=-)", ids[i]);
        } else if (pred[i] < 0) {
            snprintf(line, sizeof(line), "v%d=%d(pred=-)", ids[i], dist[i]);
        } else {
            snprintf(line, sizeof(line), "v%d=%d(pred=%d)", ids[i], dist[i], pred[i]);
        }

        if (ids[i] == vertice_activo) {
            fila_activa = true;
        }
        if (dist_prev[i] >= 1000000000 && dist[i] < 1000000000) {
            mejora = true;
        } else if (dist[i] < dist_prev[i]) {
            mejora = true;
        }

        y = (int)viewport.y + i * line_height;
        if (y + line_height <= (int)(viewport.y + viewport.height)) {
            if (fila_activa) {
                DrawRectangleRounded((Rectangle){viewport.x + 1.0f, (float)y + 1.0f,
                                                 viewport.width - 2.0f,
                                                 (float)line_height - 2.0f},
                                     0.22f, 6, Fade((Color){46, 112, 185, 255}, 0.20f));
                ui_draw_text(line, viewport.x + 4.0f, (float)y + 2.0f, 12.0f, 0.10f,
                             (Color){18, 62, 108, 255}, true);
            } else if (mejora) {
                DrawRectangleRounded((Rectangle){viewport.x + 1.0f, (float)y + 1.0f,
                                                 viewport.width - 2.0f,
                                                 (float)line_height - 2.0f},
                                     0.22f, 6, Fade((Color){34, 122, 72, 255}, 0.16f));
                ui_draw_text(line, viewport.x + 4.0f, (float)y + 2.0f, 12.0f, 0.10f,
                             (Color){26, 90, 54, 255}, true);
                ui_draw_text("+", viewport.x + viewport.width - 10.0f, (float)y + 2.0f, 12.0f,
                             0.06f, (Color){26, 90, 54, 255}, true);
            } else {
                ui_draw_text(line, viewport.x + 4.0f, (float)y + 2.0f, 12.0f, 0.10f,
                             (Color){42, 54, 70, 255}, false);
            }
        }
    }
    EndScissorMode();
}

void grafo_exportar_resumen_clipboard(AppState *app) {
    char tabla[768];
    char camino[256];
    char cerrados[192];
    char resumen[1800];
    int paso = app->grafo_controller_state.paso_actual;
    GrafoPasoMetricas metricas = grafo_obtener_metricas_paso(app, paso);

    grafo_tabla_distancias_paso(app, paso, tabla, sizeof(tabla));
    grafo_camino_parcial_paso(app, paso, camino, sizeof(camino));
    grafo_cerrados_paso(app, paso, cerrados, sizeof(cerrados));
    snprintf(resumen, sizeof(resumen),
             "Algoritmo: %s\nPaso: %d/%d\nTipo: %s\nDirigido: %s\nInicio: %d\nDestino: %d\nMejoras: %d\nSin cambio: %d\nEmpeora: %d\n%s\n%s\n%s\nEstado: %s",
             grafo_algoritmo_nombre_local(app->grafo_algoritmo_seleccionado), paso + 1,
             app->grafo_controller_state.total_pasos, grafo_tipo_paso_label(app),
             app->grafo_dirigido ? "si" : "no", app->grafo_vertice_inicio,
             app->grafo_vertice_destino, metricas.mejoras, metricas.sin_cambio,
             metricas.empeora, camino, cerrados, tabla, app->mensaje_operacion);
    SetClipboardText(resumen);
    snprintf(app->mensaje_operacion, sizeof(app->mensaje_operacion),
             "Resumen de grafo copiado al portapapeles");
}

GrafoTrace grafo_trace_desde_estado(const AppState *app) {
    GrafoTrace traza = grafo_trace_crear(grafo_algoritmo_nombre_local(app->grafo_algoritmo_seleccionado));
    char paso_desc[256];
    char paso_vars[512];
    int paso_actual;
    int i;
    int total;

    total = app->grafo_controller_state.total_pasos;
    if (total <= 0) {
        total = 1;
    }

    for (i = 0; i < total; i++) {
        grafo_describir_paso(app, i, total, paso_desc, sizeof(paso_desc), paso_vars,
                             sizeof(paso_vars));
        grafo_trace_agregar_paso(&traza, paso_desc, paso_vars,
                                 grafo_linea_desde_paso(app->grafo_algoritmo_seleccionado, i,
                                                        total));
    }

    grafo_trace_establecer_resultado(&traza, app->mensaje_operacion);
    if (traza.cantidad_pasos > 0) {
        paso_actual = app->grafo_controller_state.paso_actual;
        if (paso_actual < 0) {
            paso_actual = 0;
        }
        if (paso_actual >= traza.cantidad_pasos) {
            paso_actual = traza.cantidad_pasos - 1;
        }
        traza.paso_actual = paso_actual;
    }
    return traza;
}