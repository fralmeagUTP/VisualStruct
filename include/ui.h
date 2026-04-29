#ifndef UI_H
#define UI_H

#include "raylib.h"

/**
 * @file ui.h
 * @brief Componentes base de interfaz y layout general.
 */

typedef struct {
    Texture2D logoUTP;
    Texture2D logoISC;
    int screenWidth;
    int screenHeight;
} UIContext;

typedef struct {
    Rectangle sidebar;
    Rectangle center;
    Rectangle right;
    Rectangle bottom;
} UILayout;

/** @brief Inicializa recursos visuales comunes. */
void ui_init(UIContext *ui, int width, int height);

/** @brief Sincroniza tamano de pantalla para layout responsivo. */
void ui_set_size(UIContext *ui, int width, int height);

/** @brief Calcula paneles principales de la aplicacion. */
UILayout ui_get_layout(const UIContext *ui);

/** @brief Dibuja encabezado institucional. */
void ui_draw_header(const UIContext *ui);

/** @brief Dibuja pie de pagina base. */
void ui_draw_footer(const UIContext *ui);

/** @brief Dibuja un panel redondeado con titulo. */
void ui_draw_panel(Rectangle panel, const char *title);

/** @brief Dibuja texto con la tipografia compartida y fallback seguro. */
void ui_draw_text(const char *text, float x, float y, float size, float spacing, Color color,
                  bool heading);

/** @brief Mide texto con la tipografia compartida y fallback seguro. */
int ui_measure_text(const char *text, float size, float spacing, bool heading);

/** @brief Dibuja un boton generico y retorna true si fue presionado. */
bool ui_button(Rectangle bounds, const char *label, bool active);

/** @brief Dibuja una caja de entrada simple y retorna true si recibe foco. */
bool ui_input_box(Rectangle bounds, const char *label, const char *value, bool active,
                  bool invalid);

/** @brief Dibuja boton lateral y retorna true si fue presionado. */
bool ui_sidebar_button(Rectangle bounds, const char *label, bool active);

/** @brief Libera texturas cargadas por la UI. */
void ui_unload(UIContext *ui);

#endif
