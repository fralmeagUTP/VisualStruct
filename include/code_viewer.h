#ifndef CODE_VIEWER_H
#define CODE_VIEWER_H

#include "app_state.h"

/**
 * @file code_viewer.h
 * @brief Proveedor de snippets C asociados a cada operacion.
 */

/** @brief Retorna snippet C para estructura/operacion actual. */
const char *code_viewer_get_snippet(TipoEstructura estructura, TipoOperacion operacion);

#endif
