#ifndef ALGORITHM_TRACE_H
#define ALGORITHM_TRACE_H

#include "app_state.h"

/**
 * @file algorithm_trace.h
 * @brief Generador de traza y complejidad para la operacion actual.
 */

typedef struct {
    const char *pasos;
    const char *tiempo;
    const char *espacio;
} AlgorithmInfo;

/** @brief Obtiene explicacion y complejidad por estructura y operacion. */
AlgorithmInfo algorithm_trace_get_info(TipoEstructura estructura, TipoOperacion operacion);

#endif
