#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "cola_prioridad.h"

static Font cargar_fuente_ui(bool *personalizada) {
    const char *candidatas[] = {
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf"
    };
    int i;

    if (personalizada != NULL) {
        *personalizada = false;
    }

    for (i = 0; i < 2; i++) {
        if (FileExists(candidatas[i])) {
            Font fuente = LoadFontEx(candidatas[i], 42, NULL, 0);
            if (fuente.texture.id > 0) {
                if (personalizada != NULL) {
                    *personalizada = true;
                }
                return fuente;
            }
        }
    }
    return GetFontDefault();
}

static void draw_text(Font font, const char *texto, float x, float y, float size, Color color) {
    DrawTextEx(font, (const char *)texto, (Vector2){x, y}, size, 1.0f, color);
}

static void dibujar_tarjeta(Rectangle r, Color fondo, Color borde, float radio) {
    Rectangle sombra = {r.x + 2, r.y + 3, r.width, r.height};
    DrawRectangleRounded(sombra, radio, 10, Fade(BLACK, 0.10f));
    DrawRectangleRounded(r, radio, 10, fondo);
    DrawRectangleRoundedLinesEx(r, radio, 10, 2.0f, borde);
}

static float clamp_float(float v, float min, float max) {
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

static int clamp_int(int v, int min, int max) {
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

static float dibujar_chips(
    Font font,
    float x,
    float y,
    float maxWidth,
    float chipHeight,
    float gap,
    const char *items[],
    int count
) {
    float cx = x;
    float cy = y;
    int i;

    for (i = 0; i < count; i++) {
        float textW = MeasureTextEx(font, items[i], 24, 1.0f).x;
        float chipW = textW + 26.0f;
        Rectangle chip;

        if (chipW < 140.0f) chipW = 140.0f;
        if (chipW > maxWidth) chipW = maxWidth;
        if (cx + chipW > x + maxWidth) {
            cx = x;
            cy += chipHeight + 8.0f;
        }

        chip = (Rectangle){cx, cy, chipW, chipHeight};
        DrawRectangleRounded(chip, 0.3f, 8, (Color){219, 232, 245, 250});
        DrawRectangleRoundedLinesEx(chip, 0.3f, 8, 1.5f, (Color){122, 149, 176, 255});
        draw_text(font, items[i], chip.x + 13, chip.y + 7, 24, (Color){33, 56, 78, 255});

        cx += chipW + gap;
    }

    return cy + chipHeight;
}

static float calcular_bottom_chips(
    Font font,
    float x,
    float y,
    float maxWidth,
    float chipHeight,
    float gap,
    const char *items[],
    int count
) {
    float cx = x;
    float cy = y;
    int i;

    for (i = 0; i < count; i++) {
        float textW = MeasureTextEx(font, items[i], 24, 1.0f).x;
        float chipW = textW + 26.0f;

        if (chipW < 140.0f) chipW = 140.0f;
        if (chipW > maxWidth) chipW = maxWidth;
        if (cx + chipW > x + maxWidth) {
            cx = x;
            cy += chipHeight + 8.0f;
        }
        cx += chipW + gap;
    }
    return cy + chipHeight;
}

static void dibujar_scroll_horizontal(Rectangle pista, float contenidoAncho, float vistaAncho, float offsetX) {
    Rectangle thumb;
    float thumbAncho;
    float maxScroll = contenidoAncho - vistaAncho;
    float t = 0.0f;

    DrawRectangleRounded(pista, 0.45f, 8, Fade((Color){23, 94, 157, 255}, 0.20f));
    if (maxScroll <= 0.0f) {
        return;
    }

    thumbAncho = pista.width * (vistaAncho / contenidoAncho);
    if (thumbAncho < 44.0f) thumbAncho = 44.0f;

    t = offsetX / maxScroll;
    thumb = (Rectangle){
        pista.x + (pista.width - thumbAncho) * t,
        pista.y,
        thumbAncho,
        pista.height
    };
    DrawRectangleRounded(thumb, 0.45f, 8, (Color){26, 122, 196, 232});
}

static int indice_salida(const int *prioridades, int total) {
    int i;
    int idx = -1;
    int mejor = 0;

    if (total <= 0 || prioridades == NULL) {
        return -1;
    }

    idx = 0;
    mejor = prioridades[0];
    for (i = 1; i < total; i++) {
        if (prioridades[i] < mejor) {
            mejor = prioridades[i];
            idx = i;
        }
    }
    return idx;
}

static void dibujar_cola_prioridad(
    Font font,
    const ColaPrioridad *cola,
    Rectangle area,
    float *scrollX
) {
    int valores[256];
    int prioridades[256];
    int total = cp_copiar_items(cola, valores, prioridades, 256);
    int idxSalida = indice_salida(prioridades, total);
    const float paddingX = 18.0f;
    const float topOffset = 70.0f;
    const float bottomOffset = 26.0f;
    const float boxWidth = 152.0f;
    const float boxHeight = 84.0f;
    const float gap = 14.0f;
    Rectangle viewport = {
        area.x + paddingX,
        area.y + topOffset,
        area.width - paddingX * 2.0f,
        area.height - topOffset - bottomOffset
    };
    Rectangle scrollTrack = {
        viewport.x,
        area.y + area.height - 14.0f,
        viewport.width,
        8.0f
    };
    float contenidoAncho;
    float maxScroll;
    int i;

    dibujar_tarjeta(area, (Color){219, 240, 255, 248}, (Color){22, 95, 165, 255}, 0.04f);
    draw_text(font, "Representacion de cola con prioridad", area.x + 16, area.y + 10, 30, (Color){12, 75, 138, 255});
    draw_text(font, "Regla: menor numero de prioridad sale primero", area.x + 16, area.y + 38, 22, (Color){106, 59, 26, 255});

    if (total == 0) {
        draw_text(font, "Cola vacia", area.x + 24, area.y + 80, 33, (Color){92, 101, 112, 255});
        return;
    }

    contenidoAncho = total * boxWidth + (total - 1) * gap;
    maxScroll = contenidoAncho - viewport.width;
    if (maxScroll < 0.0f) maxScroll = 0.0f;
    if (scrollX != NULL) {
        *scrollX = clamp_float(*scrollX, 0.0f, maxScroll);
    }

    BeginScissorMode((int)viewport.x, (int)viewport.y, (int)viewport.width, (int)viewport.height);
    for (i = 0; i < total; i++) {
        float x = viewport.x + i * (boxWidth + gap) - (scrollX != NULL ? *scrollX : 0.0f);
        Rectangle item = {x, viewport.y + 30.0f, boxWidth, boxHeight};
        Color borde = (Color){31, 119, 196, 255};
        Color fondo = (Color){190, 221, 245, 236};

        if (i == idxSalida) {
            borde = (Color){244, 126, 20, 255};
            fondo = (Color){255, 232, 198, 246};
        }

        DrawRectangleRounded(item, 0.14f, 6, fondo);
        DrawRectangleRoundedLinesEx(item, 0.14f, 6, 2.0f, borde);
        draw_text(font, TextFormat("V:%d", valores[i]), item.x + 14, item.y + 28, 29, (Color){18, 70, 126, 255});
        draw_text(font, TextFormat("P:%d", prioridades[i]), item.x + 14, item.y + 56, 24, (Color){44, 58, 81, 255});

        if (i == 0) {
            draw_text(font, "FRONT", item.x + 8, item.y + 4, 18, (Color){178, 48, 12, 255});
        }
        if (i == idxSalida) {
            draw_text(font, "OUT", item.x + boxWidth - 40, item.y + 4, 18, (Color){178, 48, 12, 255});
        }

        if (i < total - 1) {
            float ax = item.x + item.width + 4.0f;
            float ay = item.y + item.height * 0.5f;
            DrawLineEx((Vector2){ax, ay}, (Vector2){ax + 14.0f, ay}, 2.5f, (Color){56, 97, 141, 255});
            DrawTriangle(
                (Vector2){ax + 14.0f, ay},
                (Vector2){ax + 8.0f, ay - 4.5f},
                (Vector2){ax + 8.0f, ay + 4.5f},
                (Color){56, 97, 141, 255}
            );
        } else {
            draw_text(font, "BACK", item.x + item.width - 52, item.y + item.height - 22, 19, (Color){130, 54, 31, 255});
        }
    }
    EndScissorMode();

    dibujar_scroll_horizontal(scrollTrack, contenidoAncho, viewport.width, scrollX != NULL ? *scrollX : 0.0f);
    if (maxScroll > 0.0f) {
        draw_text(font, "Scroll: rueda del mouse en panel o flechas <- ->", area.x + 18, area.y + area.height - 34, 22, (Color){146, 45, 26, 255});
    }
}

int main(void) {
    const int anchoInicial = 1260;
    const int altoInicial = 800;
    ColaPrioridad cola;
    Font fuenteUi;
    bool fuentePersonalizada = false;
    int valorActual = 10;
    int prioridadActual = 3;
    int pasoAuto = 10;
    bool autoIncremento = true;
    bool modoCompacto = false;
    float scrollColaX = 0.0f;
    float scrollVistaX = 0.0f;
    char estado[256];
    char vista[1400];

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(anchoInicial, altoInicial, "Visualizador de ColaPrioridad con raylib");
    SetWindowMinSize(1020, 680);
    SetTargetFPS(60);

    cp_inicializar(&cola);
    fuenteUi = cargar_fuente_ui(&fuentePersonalizada);
    strcpy(estado, "Listo. E encolar, Q desencolar, V vaciar, M mostrar.");
    cp_formatear(&cola, vista, sizeof(vista));

    while (!WindowShouldClose()) {
        int ancho = GetScreenWidth();
        int alto = GetScreenHeight();
        Rectangle rEstado;
        Rectangle rVista;
        Rectangle rCola;
        Rectangle viewportVista;
        Vector2 mouse = GetMousePosition();
        float wheel = GetMouseWheelMove();
        float anchoTextoVista;
        float maxScrollVista;
        int cuenta = cp_contar(&cola);
        int valores[256];
        int prioridades[256];
        int total = cp_copiar_items(&cola, valores, prioridades, 256);
        int idxSalida = indice_salida(prioridades, total);
        float controlsSize = (ancho < 1150) ? 25.0f : 29.0f;
        float line1Y = 84.0f;
        float line2Y = 118.0f;
        float headerBottom;
        float estadoLabelY;
        float chipsY;
        float chipHeight = 38.0f;
        float gapTarjetas = 12.0f;
        float hEstado = 72.0f;
        float hVista = 84.0f;
        float layoutBaseY;
        char mValor[80];
        char mPrio[80];
        char mAuto[80];
        char mNodos[80];
        char mModo[80];
        char mNext[100];
        const char *chips[6];
        float labelW;

        if (IsKeyPressed(KEY_ESCAPE)) {
            break;
        }

        if (IsKeyPressed(KEY_UP)) valorActual++;
        if (IsKeyPressed(KEY_DOWN)) valorActual--;
        if (IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD)) valorActual += pasoAuto;
        if (IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT)) valorActual -= pasoAuto;
        if (IsKeyPressed(KEY_RIGHT)) prioridadActual++;
        if (IsKeyPressed(KEY_LEFT)) prioridadActual--;
        prioridadActual = clamp_int(prioridadActual, 1, 9);
        if (IsKeyPressed(KEY_A)) autoIncremento = !autoIncremento;
        if (IsKeyPressed(KEY_C)) modoCompacto = !modoCompacto;

        if (IsKeyPressed(KEY_E)) {
            if (cp_encolar(&cola, valorActual, prioridadActual)) {
                snprintf(estado, sizeof(estado), "encolar(valor=%d, prioridad=%d) ejecutado", valorActual, prioridadActual);
                if (autoIncremento) valorActual += pasoAuto;
                scrollColaX = 1e9f;
            } else {
                snprintf(estado, sizeof(estado), "encolar() fallido: sin memoria");
            }
            cp_formatear(&cola, vista, sizeof(vista));
        }

        if (IsKeyPressed(KEY_Q)) {
            int valOut = 0;
            int priOut = 0;
            if (cp_desencolar(&cola, &valOut, &priOut)) {
                snprintf(estado, sizeof(estado), "desencolar() -> valor=%d, prioridad=%d", valOut, priOut);
            } else {
                snprintf(estado, sizeof(estado), "desencolar() no realizado: cola vacia");
            }
            cp_formatear(&cola, vista, sizeof(vista));
        }

        if (IsKeyPressed(KEY_V)) {
            cp_vaciar(&cola);
            snprintf(estado, sizeof(estado), "vaciar() ejecutado");
            cp_formatear(&cola, vista, sizeof(vista));
            scrollColaX = 0.0f;
            scrollVistaX = 0.0f;
        }

        if (IsKeyPressed(KEY_R)) {
            cp_vaciar(&cola);
            valorActual = 10;
            prioridadActual = 3;
            scrollColaX = 0.0f;
            scrollVistaX = 0.0f;
            snprintf(estado, sizeof(estado), "escena reiniciada");
            cp_formatear(&cola, vista, sizeof(vista));
        }

        if (IsKeyPressed(KEY_M)) {
            cp_formatear(&cola, vista, sizeof(vista));
            snprintf(estado, sizeof(estado), "mostrar() ejecutado");
        }

        cuenta = cp_contar(&cola);
        total = cp_copiar_items(&cola, valores, prioridades, 256);
        idxSalida = indice_salida(prioridades, total);

        snprintf(mValor, sizeof(mValor), "Valor actual: %d", valorActual);
        snprintf(mPrio, sizeof(mPrio), "Prioridad actual: %d", prioridadActual);
        snprintf(mAuto, sizeof(mAuto), "AutoIncremento: %s", autoIncremento ? "ON" : "OFF");
        snprintf(mNodos, sizeof(mNodos), "Nodos: %d", cuenta);
        snprintf(mModo, sizeof(mModo), "Modo: %s", modoCompacto ? "Compacto" : "Detallado");
        if (idxSalida >= 0) {
            snprintf(mNext, sizeof(mNext), "Proximo OUT: %d (p=%d)", valores[idxSalida], prioridades[idxSalida]);
        } else {
            snprintf(mNext, sizeof(mNext), "Proximo OUT: n/a");
        }
        chips[0] = mValor;
        chips[1] = mPrio;
        chips[2] = mAuto;
        chips[3] = mNodos;
        chips[4] = mNext;
        chips[5] = mModo;

        if (modoCompacto) {
            controlsSize = (ancho < 1150) ? 24.0f : 28.0f;
            line2Y = -1000.0f;
            chipHeight = 34.0f;
            gapTarjetas = 10.0f;
            hEstado = 62.0f;
            hVista = 72.0f;
        }

        if (modoCompacto) {
            headerBottom = line1Y + controlsSize;
        } else {
            headerBottom = line2Y + controlsSize;
        }
        estadoLabelY = headerBottom + 8.0f;
        chipsY = estadoLabelY + 22.0f;

        layoutBaseY = calcular_bottom_chips(fuenteUi, 34.0f, chipsY, (float)ancho - 68.0f, chipHeight, 8.0f, chips, 6) + gapTarjetas;
        rEstado = (Rectangle){30, layoutBaseY, (float)ancho - 60.0f, hEstado};
        rVista = (Rectangle){30, rEstado.y + rEstado.height + gapTarjetas, (float)ancho - 60.0f, hVista};
        rCola = (Rectangle){30, rVista.y + rVista.height + gapTarjetas, (float)ancho - 60.0f, (float)alto - (rVista.y + rVista.height + 30.0f)};
        if (rCola.height < 220.0f) rCola.height = 220.0f;

        labelW = MeasureTextEx(fuenteUi, "Vista logica:", 31, 1.0f).x;
        viewportVista = (Rectangle){
            rVista.x + 16.0f + labelW + 16.0f,
            rVista.y + 18.0f,
            rVista.width - (labelW + 48.0f),
            38.0f
        };
        if (viewportVista.width < 120.0f) viewportVista.width = 120.0f;

        anchoTextoVista = MeasureTextEx(fuenteUi, vista, modoCompacto ? 28 : 31, 1.0f).x;
        maxScrollVista = anchoTextoVista - viewportVista.width;
        if (maxScrollVista < 0.0f) maxScrollVista = 0.0f;

        if (CheckCollisionPointRec(mouse, rCola) && wheel != 0.0f) {
            scrollColaX -= wheel * 42.0f;
        } else if (CheckCollisionPointRec(mouse, viewportVista) && wheel != 0.0f) {
            scrollVistaX -= wheel * 42.0f;
        }
        scrollVistaX = clamp_float(scrollVistaX, 0.0f, maxScrollVista);

        BeginDrawing();
        ClearBackground((Color){239, 247, 255, 255});

        DrawRectangleGradientV(0, 0, ancho, alto, (Color){187, 218, 242, 255}, (Color){236, 246, 253, 255});
        draw_text(fuenteUi, "Visualizador de COLA DE PRIORIDAD", 30, 22, 52, (Color){12, 72, 136, 255});
        if (modoCompacto) {
            draw_text(fuenteUi, "E encolar | Q desencolar | V vaciar | M mostrar | R reiniciar | A auto | C", 34, line1Y, controlsSize, (Color){16, 23, 30, 255});
        } else {
            draw_text(fuenteUi, "E encolar | Q desencolar(prioridad) | V vaciar | M mostrar", 34, line1Y, controlsSize, (Color){16, 23, 30, 255});
            draw_text(fuenteUi, "UP/DOWN valor | LEFT/RIGHT prioridad | +/- salto | A auto(+10) | C compacto | ESC salir", 34, line2Y, controlsSize, (Color){16, 23, 30, 255});
        }
        draw_text(fuenteUi, "Estado actual", 34, estadoLabelY, 24, (Color){55, 77, 98, 255});
        layoutBaseY = dibujar_chips(fuenteUi, 34.0f, chipsY, (float)ancho - 68.0f, chipHeight, 8.0f, chips, 6) + gapTarjetas;

        dibujar_tarjeta(rEstado, (Color){226, 233, 240, 245}, (Color){124, 134, 147, 255}, 0.08f);
        draw_text(fuenteUi, "Ultimo metodo:", 46, rEstado.y + 20, 31, (Color){170, 17, 55, 255});
        draw_text(fuenteUi, estado, 260, rEstado.y + 20, 31, (Color){22, 25, 28, 255});

        dibujar_tarjeta(rVista, (Color){231, 220, 196, 238}, (Color){182, 149, 100, 255}, 0.08f);
        draw_text(fuenteUi, "Vista logica:", 46, rVista.y + (modoCompacto ? 16.0f : 20.0f), modoCompacto ? 28 : 31, (Color){10, 124, 66, 255});
        BeginScissorMode((int)viewportVista.x, (int)viewportVista.y, (int)viewportVista.width, (int)viewportVista.height);
        draw_text(fuenteUi, vista, viewportVista.x - scrollVistaX, viewportVista.y, modoCompacto ? 28 : 31, (Color){20, 24, 28, 255});
        EndScissorMode();
        dibujar_scroll_horizontal((Rectangle){viewportVista.x, viewportVista.y + viewportVista.height + 8, viewportVista.width, 8}, anchoTextoVista, viewportVista.width, scrollVistaX);

        dibujar_cola_prioridad(fuenteUi, &cola, rCola, &scrollColaX);

        EndDrawing();
    }

    cp_vaciar(&cola);
    if (fuentePersonalizada) {
        UnloadFont(fuenteUi);
    }
    CloseWindow();
    return 0;
}
