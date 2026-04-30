#ifndef GRAFO_PEDAGOGY_H
#define GRAFO_PEDAGOGY_H

#include <stddef.h>

#include <raylib.h>

#include "app_state.h"
#include "grafo_trace.h"

typedef struct {
    int mejoras;
    int sin_cambio;
    int empeora;
} GrafoPasoMetricas;

int grafo_linea_desde_paso(int algoritmo, int paso_actual, int total_pasos);
int grafo_vertice_activo_paso(const AppState *app, int paso_idx);
const char *grafo_tipo_paso_label(const AppState *app);
bool grafo_hay_mejora_paso(const AppState *app, int paso_idx);
GrafoPasoMetricas grafo_obtener_metricas_paso(const AppState *app, int paso_idx);
int grafo_contar_mejoras_paso(const AppState *app, int paso_idx);
int grafo_contar_sin_cambio_paso(const AppState *app, int paso_idx);
int grafo_contar_empeora_paso(const AppState *app, int paso_idx);
void grafo_camino_parcial_paso(const AppState *app, int paso_idx, char *out, size_t out_size);
void grafo_cerrados_paso(const AppState *app, int paso_idx, char *out, size_t out_size);
void grafo_tabla_distancias_paso(const AppState *app, int paso_idx, char *out, size_t out_size);
void grafo_tabla_distancias_multiline(const AppState *app, int paso_idx, char *out,
                                      size_t out_size);
void grafo_dibujar_tabla_distancias(const AppState *app, Rectangle viewport, int paso_idx);
void grafo_exportar_resumen_clipboard(AppState *app);
GrafoTrace grafo_trace_desde_estado(const AppState *app);

#endif