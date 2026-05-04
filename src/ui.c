#include "ui.h"

#include <stddef.h>
#include <stdbool.h>
#include <math.h>

/**
 * @file ui.c
 * @brief Implementacion de layout, widgets simples y recursos visuales compartidos.
 */

static const Color COLOR_PRIMARY = {17, 69, 132, 255};
static const Color COLOR_PRIMARY_DEEP = {10, 43, 92, 255};
static const Color COLOR_ACCENT = {198, 165, 102, 255};
static const Color COLOR_TEXT = {38, 48, 61, 255};
static const Color COLOR_TEXT_MUTED = {66, 80, 98, 255};
static const Color COLOR_PANEL = {240, 245, 250, 255};
static const int HEADER_HEIGHT = 114;
static const int FOOTER_HEIGHT = 42;
static const int GAP = 12;

static Font FONT_HEADING = {0};
static Font FONT_BODY = {0};
static bool FONT_HEADING_LOADED = false;
static bool FONT_BODY_LOADED = false;

/** @brief Ajusta coordenadas a rejilla de pixeles para evitar texto borroso por subpixel. */
static float snap_px(float value) {
    return floorf(value + 0.5f);
}

/** @brief Retorna la fuente tipografica principal si esta disponible. */
static const Font *heading_font(void) {
    return FONT_HEADING_LOADED ? &FONT_HEADING : NULL;
}

/** @brief Retorna la fuente base de lectura si esta disponible. */
static const Font *body_font(void) {
    return FONT_BODY_LOADED ? &FONT_BODY : NULL;
}

/** @brief Mide un texto usando fuente externa cuando existe. */
static int measure_ui_text(const Font *font, const char *text, float size, float spacing) {
    float render_size = size < 12.0f ? 12.0f : size; /* Arial 9pt ~= 12px */
    float render_spacing = spacing;

    if (render_size <= 14.0f && render_spacing > 0.10f) {
        render_spacing = 0.10f;
    }
    if (font != NULL) {
        return (int)MeasureTextEx(*font, text, render_size, render_spacing).x;
    }
    return MeasureText(text, (int)render_size);
}

/** @brief Dibuja texto con fallback seguro a la fuente por defecto. */
static void draw_ui_text(const Font *font, const char *text, float x, float y, float size,
                         float spacing, Color color) {
    float render_size = size < 12.0f ? 12.0f : size; /* Arial 9pt ~= 12px */
    float render_spacing = spacing;
    float draw_x = snap_px(x);
    float draw_y = snap_px(y);

    if (render_size <= 14.0f && render_spacing > 0.10f) {
        render_spacing = 0.10f;
    }

    if (font != NULL) {
        DrawTextEx(*font, text, (Vector2){draw_x, draw_y}, render_size, render_spacing, color);
        return;
    }
    DrawText(text, (int)draw_x, (int)draw_y, (int)render_size, color);
}

/** @brief Dibuja texto reutilizando las fuentes compartidas de la UI. */
void ui_draw_text(const char *text, float x, float y, float size, float spacing, Color color,
                  bool heading) {
    draw_ui_text(heading ? heading_font() : body_font(), text, x, y, size, spacing, color);
}

/** @brief Mide texto reutilizando las fuentes compartidas de la UI. */
int ui_measure_text(const char *text, float size, float spacing, bool heading) {
    return measure_ui_text(heading ? heading_font() : body_font(), text, size, spacing);
}

/** @brief Dibuja una textura centrada y contenida dentro de un rectangulo. */
static void draw_texture_fit(Texture2D texture, Rectangle bounds, float padding) {
    Rectangle target;
    float scale;

    if (texture.id == 0) {
        return;
    }

    target = (Rectangle){bounds.x + padding, bounds.y + padding, bounds.width - padding * 2.0f,
                         bounds.height - padding * 2.0f};
    scale = target.width / (float)texture.width;
    if ((float)texture.height * scale > target.height) {
        scale = target.height / (float)texture.height;
    }

    DrawTextureEx(texture,
                  (Vector2){target.x + (target.width - texture.width * scale) * 0.5f,
                            target.y + (target.height - texture.height * scale) * 0.5f},
                  0.0f, scale, WHITE);
}

/** @brief Dibuja una insignia simple cuando un logo institucional no esta disponible. */
static void draw_logo_fallback(Rectangle bounds, const char *text) {
    int width = measure_ui_text(heading_font(), text, 28.0f, 1.0f);

    DrawRectangleRounded(bounds, 0.22f, 10, Fade((Color){225, 235, 247, 255}, 0.95f));
    DrawRectangleRoundedLinesEx(bounds, 0.22f, 10, 1.8f, Fade(COLOR_PRIMARY, 0.28f));
    DrawCircleGradient((int)(bounds.x + bounds.width * 0.5f),
                       (int)(bounds.y + bounds.height * 0.5f), 24.0f,
                       Fade(COLOR_PRIMARY, 0.16f), Fade(COLOR_PRIMARY, 0.03f));
    draw_ui_text(heading_font(), text, bounds.x + (bounds.width - width) * 0.5f,
                 bounds.y + bounds.height * 0.5f - 13.0f, 28.0f, 1.0f, COLOR_PRIMARY_DEEP);
}

/** @brief Inicializa logos y dimensiones base de la interfaz. */
void ui_init(UIContext *ui, int width, int height) {
    const char *heading_candidates[] = {
        "C:/Windows/Fonts/arialbd.ttf",
        "C:/Windows/Fonts/tahomabd.ttf",
        "C:/Windows/Fonts/calibrib.ttf",
        "C:/Windows/Fonts/timesbd.ttf",
        "assets/fons/AtkinsonHyperlegible-Bold.ttf"
    };
    const char *body_candidates[] = {
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/tahoma.ttf",
        "C:/Windows/Fonts/calibri.ttf",
        "C:/Windows/Fonts/times.ttf",
        "assets/fons/AtkinsonHyperlegible-Regular.ttf",
        "assets/fons/SourceSans3.ttf"
    };
    int i;

    if (ui == NULL) {
        return;
    }

    ui->screenWidth = width;
    ui->screenHeight = height;

    ui->logoUTP = LoadTexture("assets/logo UTP.png");
    ui->logoISC = LoadTexture("assets/Logo_ISC.jpg");
    if (ui->logoISC.id == 0) {
        ui->logoISC = LoadTexture("assets/Logo_ISC.png");
    }

    for (i = 0; i < (int)(sizeof(heading_candidates) / sizeof(heading_candidates[0])); i++) {
        FONT_HEADING = LoadFontEx(heading_candidates[i], 72, NULL, 0);
        if (FONT_HEADING.texture.id != 0) {
            break;
        }
    }
    for (i = 0; i < (int)(sizeof(body_candidates) / sizeof(body_candidates[0])); i++) {
        FONT_BODY = LoadFontEx(body_candidates[i], 72, NULL, 0);
        if (FONT_BODY.texture.id != 0) {
            break;
        }
    }
    FONT_HEADING_LOADED = FONT_HEADING.texture.id != 0;
    FONT_BODY_LOADED = FONT_BODY.texture.id != 0;

    if (FONT_HEADING_LOADED) {
        SetTextureFilter(FONT_HEADING.texture, TEXTURE_FILTER_POINT);
    }
    if (FONT_BODY_LOADED) {
        SetTextureFilter(FONT_BODY.texture, TEXTURE_FILTER_POINT);
    }
}

/** @brief Actualiza las dimensiones almacenadas del viewport actual. */
void ui_set_size(UIContext *ui, int width, int height) {
    if (ui == NULL) {
        return;
    }

    ui->screenWidth = width;
    ui->screenHeight = height;
}

/** @brief Calcula los paneles principales segun el tamano actual de ventana. */
UILayout ui_get_layout(const UIContext *ui) {
    UILayout layout;
    float contentTop = (float)(HEADER_HEIGHT + GAP);
    float contentBottom = (float)(ui->screenHeight - FOOTER_HEIGHT - GAP);
    float contentHeight = contentBottom - contentTop;
    float sidebarW = 196.0f;
    float rightW = 308.0f;
    float bottomH = 172.0f;
    float centerH = contentHeight - bottomH - GAP;

    if (ui->screenWidth <= 1366) {
        sidebarW = 184.0f;
        rightW = 286.0f;
    }
    if (ui->screenWidth <= 1280) {
        sidebarW = 176.0f;
        rightW = 270.0f;
    }
    if (ui->screenHeight <= 760) {
        bottomH = 160.0f;
    }
    if (ui->screenHeight <= 720) {
        bottomH = 148.0f;
    }

    centerH = contentHeight - bottomH - GAP;
    if (centerH < 200.0f) {
        centerH = 200.0f;
    }

    layout.sidebar = (Rectangle){
        (float)GAP,
        contentTop,
        sidebarW,
        contentHeight
    };
    layout.center = (Rectangle){
        layout.sidebar.x + layout.sidebar.width + GAP,
        contentTop,
        (float)ui->screenWidth - sidebarW - rightW - GAP * 4.0f,
        centerH
    };
    layout.right = (Rectangle){
        layout.center.x + layout.center.width + GAP,
        contentTop,
        rightW,
        centerH
    };
    layout.bottom = (Rectangle){
        layout.center.x,
        layout.center.y + layout.center.height + GAP,
        layout.center.width + GAP + layout.right.width,
        contentBottom - (layout.center.y + layout.center.height + GAP)
    };

    if (layout.center.width < 320.0f) {
        layout.center.width = 320.0f;
    }
    if (layout.bottom.height < 100.0f) {
        layout.bottom.height = 100.0f;
    }

    return layout;
}

/** @brief Dibuja el encabezado institucional con logos y titulos. */
void ui_draw_header(const UIContext *ui) {
    Rectangle left_logo_box;
    Rectangle right_logo_box;
    Rectangle title_band;
    const Font *heading = heading_font();
    const Font *body = body_font();
    float title_size = 33.0f;
    float subtitle_size = 17.0f;
    float chip_size = 12.0f;
    int titleWidth;
    int subtitleWidth;
    int chipWidth;

    DrawRectangleGradientV(0, 0, ui->screenWidth, HEADER_HEIGHT, (Color){252, 253, 255, 255},
                           (Color){235, 241, 248, 255});
    DrawRectangle(0, HEADER_HEIGHT - 6, ui->screenWidth, 6, COLOR_PRIMARY_DEEP);
    DrawLine(0, HEADER_HEIGHT - 8, ui->screenWidth, HEADER_HEIGHT - 8,
             Fade(COLOR_ACCENT, 0.70f));

    left_logo_box = (Rectangle){14.0f, 10.0f, 162.0f, 82.0f};
    right_logo_box = (Rectangle){(float)ui->screenWidth - 152.0f, 10.0f, 138.0f, 82.0f};
    title_band = (Rectangle){188.0f, 12.0f, (float)ui->screenWidth - 376.0f, 78.0f};

    DrawRectangleRounded(left_logo_box, 0.18f, 10, Fade(WHITE, 0.88f));
    DrawRectangleRounded(right_logo_box, 0.18f, 10, Fade(WHITE, 0.88f));
    DrawRectangleRounded(title_band, 0.20f, 12, Fade(WHITE, 0.44f));
    DrawRectangleRoundedLinesEx(left_logo_box, 0.18f, 10, 1.5f, Fade(COLOR_PRIMARY, 0.18f));
    DrawRectangleRoundedLinesEx(right_logo_box, 0.18f, 10, 1.5f, Fade(COLOR_PRIMARY, 0.18f));

    draw_texture_fit(ui->logoUTP, left_logo_box, 8.0f);
    if (ui->logoISC.id != 0) {
        draw_texture_fit(ui->logoISC, right_logo_box, 10.0f);
    } else {
        draw_logo_fallback(right_logo_box, "ISC");
    }

    titleWidth = measure_ui_text(heading, "UNIVERSIDAD TECNOLOGICA DE PEREIRA", title_size, 1.0f);
    subtitleWidth = measure_ui_text(body, "Ingenieria de Sistemas y Computacion", subtitle_size, 0.5f);
    chipWidth = measure_ui_text(body, "VISUALSTRUCT", chip_size, 1.8f);
    draw_ui_text(body, "VISUALSTRUCT", (ui->screenWidth - chipWidth) * 0.5f, 14.0f, chip_size,
                 1.8f, Fade(COLOR_PRIMARY_DEEP, 0.72f));
    DrawLine((int)(ui->screenWidth * 0.38f), 23, (int)(ui->screenWidth * 0.46f), 23,
             Fade(COLOR_ACCENT, 0.9f));
    DrawLine((int)(ui->screenWidth * 0.54f), 23, (int)(ui->screenWidth * 0.62f), 23,
             Fade(COLOR_ACCENT, 0.9f));
    draw_ui_text(heading, "UNIVERSIDAD TECNOLOGICA DE PEREIRA",
                 (float)(ui->screenWidth - titleWidth) * 0.5f, 27.0f, title_size, 1.0f,
                 COLOR_PRIMARY_DEEP);
    draw_ui_text(body, "Ingenieria de Sistemas y Computacion",
                 (float)(ui->screenWidth - subtitleWidth) * 0.5f, 65.0f, subtitle_size, 0.5f,
                 COLOR_TEXT);
    DrawLine((int)(ui->screenWidth * 0.33f), 90, (int)(ui->screenWidth * 0.67f), 90,
             Fade(COLOR_PRIMARY, 0.16f));

    DrawCircleGradient((int)(title_band.x + 22.0f), (int)(title_band.y + 39.0f), 7.0f,
                       Fade(COLOR_ACCENT, 0.95f), Fade(COLOR_ACCENT, 0.15f));
    DrawCircleGradient((int)(title_band.x + title_band.width - 22.0f), (int)(title_band.y + 39.0f),
                       7.0f, Fade(COLOR_ACCENT, 0.95f), Fade(COLOR_ACCENT, 0.15f));
}

/** @brief Dibuja el pie de pagina informativo. */
void ui_draw_footer(const UIContext *ui) {
    int y = ui->screenHeight - FOOTER_HEIGHT;
    const Font *heading = heading_font();
    const Font *body = body_font();
    int leftW;
    int centerW;
    int rightW;

    DrawRectangleGradientH(0, y, ui->screenWidth, FOOTER_HEIGHT, (Color){248, 250, 253, 255},
                           (Color){238, 243, 249, 255});
    DrawLine(0, y, ui->screenWidth, y, Fade(COLOR_PRIMARY_DEEP, 0.35f));
    DrawRectangle(0, y, ui->screenWidth, 3, Fade(COLOR_PRIMARY, 0.88f));

    leftW = measure_ui_text(heading, "VISUALSTRUCT", 14.0f, 1.0f);
    centerW = measure_ui_text(body, "Proyecto Academico · Estructuras de Datos", 14.0f, 0.3f);
    rightW = measure_ui_text(body, "UTP · 2026", 14.0f, 0.3f);

    draw_ui_text(heading, "VISUALSTRUCT", 16.0f, (float)y + 12.0f, 14.0f, 1.0f,
                 COLOR_PRIMARY_DEEP);
    DrawCircle(16 + leftW + 18, y + 20, 2.5f, Fade(COLOR_ACCENT, 0.95f));
    draw_ui_text(body, "Proyecto Academico · Estructuras de Datos",
                 (float)(ui->screenWidth - centerW) * 0.5f, (float)y + 12.0f, 14.0f, 0.3f,
                 COLOR_TEXT_MUTED);
    draw_ui_text(body, "UTP · 2026", (float)(ui->screenWidth - rightW - 16), (float)y + 12.0f,
                 14.0f, 0.3f, COLOR_PRIMARY);
}

/** @brief Dibuja un panel base con borde y titulo. */
void ui_draw_panel(Rectangle panel, const char *title) {
    DrawRectangleRounded(panel, 0.035f, 10, COLOR_PANEL);
    DrawRectangleRoundedLinesEx(panel, 0.035f, 10, 2.0f, Fade(COLOR_PRIMARY, 0.34f));
    DrawRectangleRounded((Rectangle){panel.x + 10.0f, panel.y + 8.0f, panel.width - 20.0f, 26.0f},
                         0.30f, 10, Fade(WHITE, 0.55f));
    draw_ui_text(body_font(), title, panel.x + 14.0f, panel.y + 12.0f, 18.0f, 0.12f,
                 COLOR_PRIMARY_DEEP);
}

/** @brief Dibuja un boton clicable de uso general. */
bool ui_button(Rectangle bounds, const char *label, bool active) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, bounds);
    float label_size = bounds.height >= 40.0f ? 17.0f : 15.0f;
    float label_spacing = label_size >= 17.0f ? 0.12f : 0.10f;
    int labelWidth = measure_ui_text(body_font(), label, label_size, label_spacing);
    Color bg = active ? (Color){216, 231, 246, 255} : (Color){248, 251, 254, 255};
    Color border = active ? COLOR_PRIMARY_DEEP : (Color){143, 163, 186, 255};
    Color text = active ? COLOR_PRIMARY_DEEP : COLOR_TEXT;

    if (hover) {
        bg = (Color){231, 240, 249, 255};
    }

    DrawRectangleRounded(bounds, 0.20f, 10, bg);
    DrawRectangleRoundedLinesEx(bounds, 0.20f, 10, 2.0f, border);
    if (active || hover) {
        DrawRectangleRounded((Rectangle){bounds.x + 1.0f, bounds.y + 1.0f, 4.0f, bounds.height - 2.0f},
                             0.40f, 8, Fade(COLOR_ACCENT, 0.95f));
    }
    draw_ui_text(body_font(), label, bounds.x + (bounds.width - labelWidth) * 0.5f,
                 bounds.y + (bounds.height - label_size) * 0.5f - 1.0f, label_size, label_spacing,
                 text);

    return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

/** @brief Dibuja una caja de texto simple para entradas numéricas. */
bool ui_input_box(Rectangle bounds, const char *label, const char *value, bool active,
                  bool invalid) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, bounds);
    float value_size = bounds.height >= 38.0f ? 17.0f : 15.0f;
    int valueWidth = measure_ui_text(body_font(), value, value_size, 0.10f);
    Color bg = active ? (Color){255, 255, 255, 255} : (Color){247, 250, 253, 255};
    Color border = active ? COLOR_PRIMARY : (Color){123, 150, 175, 255};

    draw_ui_text(body_font(), label, bounds.x + 1.0f, bounds.y - 14.0f, 11.0f, 0.08f,
                 COLOR_TEXT_MUTED);
    if (hover && !active) {
        bg = (Color){251, 253, 255, 255};
    }
    if (invalid) {
        bg = (Color){255, 245, 244, 255};
        border = (Color){187, 65, 54, 255};
    }

    DrawRectangleRounded(bounds, 0.18f, 10, bg);
    DrawRectangleRoundedLinesEx(bounds, 0.18f, 10, 2.0f, border);
    draw_ui_text(body_font(), value, bounds.x + bounds.width - valueWidth - 12.0f,
                 bounds.y + (bounds.height - value_size) * 0.5f - 1.0f, value_size, 0.10f,
                 COLOR_TEXT);

    return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

/** @brief Reutiliza el estilo de boton general para el menu lateral. */
bool ui_sidebar_button(Rectangle bounds, const char *label, bool active) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, bounds);
    float label_size = bounds.height >= 40.0f ? 16.0f : 14.0f;
    float label_spacing = 0.10f;
    int labelWidth = measure_ui_text(body_font(), label, label_size, label_spacing);
    Color bg = active ? (Color){223, 234, 246, 255} : (Color){248, 250, 253, 255};
    Color border = active ? COLOR_PRIMARY_DEEP : (Color){160, 177, 196, 255};

    if (hover) {
        bg = (Color){233, 241, 249, 255};
    }

    DrawRectangleRounded(bounds, 0.22f, 10, bg);
    DrawRectangleRoundedLinesEx(bounds, 0.22f, 10, 2.0f, border);
    DrawRectangleRounded((Rectangle){bounds.x + 1.5f, bounds.y + 1.5f, 6.0f, bounds.height - 3.0f},
                         0.50f, 8, active ? COLOR_ACCENT : Fade(COLOR_PRIMARY, hover ? 0.45f : 0.18f));
    draw_ui_text(body_font(), label, bounds.x + (bounds.width - labelWidth) * 0.5f,
                 bounds.y + (bounds.height - label_size) * 0.5f - 1.0f, label_size, label_spacing,
                 active ? COLOR_PRIMARY_DEEP : COLOR_TEXT);

    return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

/** @brief Libera las texturas cargadas por la UI. */
void ui_unload(UIContext *ui) {
    if (ui == NULL) {
        return;
    }

    if (ui->logoUTP.id != 0) {
        UnloadTexture(ui->logoUTP);
    }
    if (ui->logoISC.id != 0) {
        UnloadTexture(ui->logoISC);
    }
    if (FONT_HEADING_LOADED) {
        UnloadFont(FONT_HEADING);
        FONT_HEADING_LOADED = false;
    }
    if (FONT_BODY_LOADED) {
        UnloadFont(FONT_BODY);
        FONT_BODY_LOADED = false;
    }
}
