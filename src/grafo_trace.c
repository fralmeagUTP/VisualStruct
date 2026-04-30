/**
 * @file grafo_trace.c
 * @brief Implementación de la traza pedagógica
 */

#include "grafo_trace.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * Inicialización
 * ============================================================================ */

GrafoTrace grafo_trace_crear(const char *nombre_algoritmo) {
    GrafoTrace traza;
    if (nombre_algoritmo) {
        strncpy(traza.nombre_algoritmo, nombre_algoritmo, sizeof(traza.nombre_algoritmo) - 1);
    } else {
        strcpy(traza.nombre_algoritmo, "Algoritmo sin nombre");
    }
    
    traza.cantidad_pasos = 0;
    traza.paso_actual = -1;
    strcpy(traza.resultado_final, "");
    
    return traza;
}

bool grafo_trace_agregar_paso(GrafoTrace *traza, const char *descripcion, 
                             const char *variables, int linea_codigo) {
    if (!traza || !descripcion) return false;
    if (traza->cantidad_pasos >= 200) return false;
    
    GrafoTracePaso *paso = &traza->pasos[traza->cantidad_pasos];
    paso->numero_paso = traza->cantidad_pasos + 1;
    
    strncpy(paso->descripcion, descripcion, sizeof(paso->descripcion) - 1);
    paso->descripcion[sizeof(paso->descripcion) - 1] = '\0';
    
    if (variables) {
        strncpy(paso->variables, variables, sizeof(paso->variables) - 1);
    } else {
        strcpy(paso->variables, "");
    }
    paso->variables[sizeof(paso->variables) - 1] = '\0';
    
    paso->linea_codigo_actual = linea_codigo;
    strcpy(paso->vertices_procesados, "");
    strcpy(paso->aristas_examinadas, "");
    
    traza->cantidad_pasos++;
    
    /* Si es el primer paso, posicionarse en él */
    if (traza->paso_actual < 0) {
        traza->paso_actual = 0;
    }
    
    return true;
}

void grafo_trace_establecer_resultado(GrafoTrace *traza, const char *resultado) {
    if (!traza || !resultado) return;
    strncpy(traza->resultado_final, resultado, sizeof(traza->resultado_final) - 1);
    traza->resultado_final[sizeof(traza->resultado_final) - 1] = '\0';
}

/* ============================================================================
 * Navegación
 * ============================================================================ */

bool grafo_trace_paso_siguiente(GrafoTrace *traza) {
    if (!traza || traza->paso_actual < 0) return false;
    
    if (traza->paso_actual < traza->cantidad_pasos - 1) {
        traza->paso_actual++;
        return true;
    }
    return false;
}

bool grafo_trace_paso_anterior(GrafoTrace *traza) {
    if (!traza || traza->paso_actual <= 0) return false;
    
    traza->paso_actual--;
    return true;
}

bool grafo_trace_saltar_paso(GrafoTrace *traza, int numero_paso) {
    if (!traza || numero_paso < 1 || numero_paso > traza->cantidad_pasos) {
        return false;
    }
    
    traza->paso_actual = numero_paso - 1;
    return true;
}

/* ============================================================================
 * Renderizado
 * ============================================================================ */

void grafo_trace_dibujar(const GrafoTrace *traza, Rectangle area_destino) {
    if (!traza) return;
    
    /* Encabezado */
    DrawRectangleRec(area_destino, (Color){240, 240, 240, 255});
    DrawRectangleLinesEx(area_destino, 2.0f, (Color){100, 100, 100, 255});
    
    /* Título */
    char titulo[128];
    snprintf(titulo, sizeof(titulo), "Traza: %s", traza->nombre_algoritmo);
    DrawText(titulo, (int)area_destino.x + 10, (int)area_destino.y + 5, 14, BLACK);
    
    /* Contenido */
    int y_offset = (int)area_destino.y + 30;
    
    if (traza->paso_actual >= 0 && traza->paso_actual < traza->cantidad_pasos) {
        const GrafoTracePaso *paso = &traza->pasos[traza->paso_actual];
        
        /* Número de paso */
        DrawText("Paso:", (int)area_destino.x + 10, y_offset, 12, BLUE);
        char paso_str[32];
        snprintf(paso_str, sizeof(paso_str), "%d / %d", paso->numero_paso, traza->cantidad_pasos);
        DrawText(paso_str, (int)area_destino.x + 100, y_offset, 12, BLACK);
        y_offset += 20;
        
        /* Descripción */
        DrawText("Acción:", (int)area_destino.x + 10, y_offset, 12, BLUE);
        y_offset += 15;
        DrawText(paso->descripcion, (int)area_destino.x + 20, y_offset, 11, BLACK);
        y_offset += 30;
        
        /* Variables */
        if (strlen(paso->variables) > 0) {
            DrawText("Variables:", (int)area_destino.x + 10, y_offset, 12, BLUE);
            y_offset += 15;
            DrawText(paso->variables, (int)area_destino.x + 20, y_offset, 10, (Color){80, 80, 80, 255});
            y_offset += 20;
        }
        
        /* Resultado final si es el último paso */
        if (traza->paso_actual == traza->cantidad_pasos - 1 && strlen(traza->resultado_final) > 0) {
            DrawText("Resultado:", (int)area_destino.x + 10, y_offset, 12, GREEN);
            y_offset += 15;
            DrawText(traza->resultado_final, (int)area_destino.x + 20, y_offset, 11, GREEN);
        }
    }
}

void grafo_trace_dibujar_barra_progreso(const GrafoTrace *traza, Rectangle area_barra) {
    if (!traza) return;
    
    /* Fondo */
    DrawRectangleRec(area_barra, (Color){200, 200, 200, 255});
    
    if (traza->cantidad_pasos > 0) {
        /* Barra de progreso */
        float progreso = (float)(traza->paso_actual + 1) / traza->cantidad_pasos;
        float ancho_progreso = area_barra.width * progreso;
        
        DrawRectangle((int)area_barra.x, (int)area_barra.y, 
                     (int)ancho_progreso, (int)area_barra.height, BLUE);
        
        /* Porcentaje */
        char porcentaje[16];
        snprintf(porcentaje, sizeof(porcentaje), "%d%%", (int)(progreso * 100));
        DrawText(porcentaje, (int)(area_barra.x + area_barra.width / 2 - 15), 
                (int)(area_barra.y + area_barra.height / 2 - 6), 10, BLACK);
    }
    
    DrawRectangleLinesEx(area_barra, 1.0f, BLACK);
}

/* ============================================================================
 * Consultas
 * ============================================================================ */

const GrafoTracePaso* grafo_trace_obtener_paso_actual(const GrafoTrace *traza) {
    if (!traza || traza->paso_actual < 0 || traza->paso_actual >= traza->cantidad_pasos) {
        return NULL;
    }
    return &traza->pasos[traza->paso_actual];
}

const char* grafo_trace_obtener_progreso(const GrafoTrace *traza) {
    static char buffer[32];
    if (!traza) {
        strcpy(buffer, "0 / 0");
        return buffer;
    }
    
    if (traza->paso_actual < 0) {
        snprintf(buffer, sizeof(buffer), "0 / %d", traza->cantidad_pasos);
    } else {
        snprintf(buffer, sizeof(buffer), "%d / %d", 
                traza->paso_actual + 1, traza->cantidad_pasos);
    }
    return buffer;
}
