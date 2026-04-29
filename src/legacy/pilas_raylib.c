#include <raylib.h>
#include <stdio.h>
#include <string.h>

#include "pila.h"

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
            Font fuente = LoadFontEx(candidatas[i], 64, NULL, 0);
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
    DrawTextEx(font, texto, (Vector2){x, y}, size, 1.0f, color);
}

static void dibujar_tarjeta(Rectangle r, Color fondo, Color borde, float radio) {
    Rectangle sombra = {r.x + 2, r.y + 3, r.width, r.height};
    DrawRectangleRounded(sombra, radio, 10, Fade(BLACK, 0.12f));
    DrawRectangleRounded(r, radio, 10, fondo);
    DrawRectangleRoundedLinesEx(r, radio, 10, 2.0f, borde);
}

static int clamp_int(int valor, int minimo, int maximo) {
    if (valor < minimo) return minimo;
    if (valor > maximo) return maximo;
    return valor;
}

static float clamp_float(float valor, float minimo, float maximo) {
    if (valor < minimo) return minimo;
    if (valor > maximo) return maximo;
    return valor;
}

static int calcular_max_scroll_pila(Rectangle area, int total) {
    float topOffset = 48.0f;
    float bottomOffset = 16.0f;
    float contentHeight = area.height - topOffset - bottomOffset;
    float nodeHeight = 40.0f;
    float gap = 8.0f;
    int visibles;

    if (contentHeight < nodeHeight) {
        visibles = 1;
    } else {
        visibles = (int)((contentHeight + gap) / (nodeHeight + gap));
        if (visibles < 1) visibles = 1;
    }
    if (total <= visibles) return 0;
    return total - visibles;
}

static void dibujar_scroll_vertical(Rectangle pista, int total, int visibles, int offset) {
    Rectangle thumb;
    float thumbAlto;
    float t;

    if (total <= visibles || total <= 0) {
        DrawRectangleRounded(pista, 0.35f, 8, Fade((Color){34, 79, 120, 255}, 0.18f));
        return;
    }

    DrawRectangleRounded(pista, 0.35f, 8, Fade((Color){34, 79, 120, 255}, 0.18f));

    thumbAlto = pista.height * ((float)visibles / (float)total);
    if (thumbAlto < 24.0f) thumbAlto = 24.0f;

    t = (float)offset / (float)(total - visibles);
    thumb = (Rectangle){
        pista.x + 2,
        pista.y + 2 + (pista.height - thumbAlto - 4) * t,
        pista.width - 4,
        thumbAlto
    };

    DrawRectangleRounded(thumb, 0.4f, 8, (Color){40, 128, 212, 220});
}

static void dibujar_pila(Font font, const Pila *pila, Rectangle area, int *scrollOffset) {
    int valores[128];
    int total = pila_copiar_valores(pila, valores, 128);
    const int maxVisiblesTop = 128;
    float margenX = 24.0f;
    float topOffset = 48.0f;
    float bottomOffset = 16.0f;
    float contentHeight = area.height - topOffset - bottomOffset;
    float nodeHeight = 40.0f;
    float gap = 8.0f;
    Rectangle listArea = {area.x + margenX, area.y + topOffset, area.width - margenX * 2.0f - 14, contentHeight};
    Rectangle scrollTrack = {area.x + area.width - 20, area.y + topOffset, 10, contentHeight};
    int visibles;
    int inicio;
    int maxScroll;
    int i;

    dibujar_tarjeta(area, (Color){220, 242, 255, 245}, (Color){15, 96, 172, 255}, 0.04f);
    draw_text(font, "Representacion de pila (tope arriba)", area.x + 18, area.y + 10, 30, (Color){17, 82, 145, 255});

    if (total == 0) {
        draw_text(font, "No hay nodos para dibujar", area.x + 24, area.y + 72, 31, (Color){90, 98, 108, 255});
        return;
    }

    visibles = (int)((contentHeight + gap) / (nodeHeight + gap));
    if (visibles < 1) visibles = 1;
    if (visibles > maxVisiblesTop) visibles = maxVisiblesTop;

    maxScroll = total - visibles;
    if (maxScroll < 0) maxScroll = 0;

    if (scrollOffset != NULL) {
        *scrollOffset = clamp_int(*scrollOffset, 0, maxScroll);
        inicio = *scrollOffset;
    } else {
        inicio = 0;
    }

    BeginScissorMode((int)listArea.x, (int)listArea.y, (int)listArea.width, (int)listArea.height);
    for (i = 0; i < visibles && (inicio + i) < total; i++) {
        int index = inicio + i;
        Rectangle bloque = {
            listArea.x,
            listArea.y + (float)i * (nodeHeight + gap),
            listArea.width,
            nodeHeight
        };
        Color colorBorde = (index == 0) ? (Color){250, 145, 25, 255} : (Color){30, 120, 205, 255};
        Color colorFondo = (index == 0) ? (Color){255, 234, 207, 240} : (Color){188, 220, 245, 232};

        DrawRectangleRounded(bloque, 0.15f, 6, colorFondo);
        DrawRectangleRoundedLinesEx(bloque, 0.15f, 6, 2.0f, colorBorde);
        draw_text(font, TextFormat("valor: %d", valores[index]), bloque.x + 14, bloque.y + 7, 28, (Color){18, 72, 130, 255});
        if (index == 0) {
            draw_text(font, "TOP", bloque.x + bloque.width - 66, bloque.y + (bloque.height * 0.22f), 30, (Color){196, 20, 55, 255});
        }
    }
    EndScissorMode();

    dibujar_scroll_vertical(scrollTrack, total, visibles, inicio);

    if (total > visibles) {
        draw_text(font, "Scroll: rueda del mouse sobre este panel", area.x + 22, area.y + area.height - 22, 22, (Color){166, 49, 27, 255});
    }
}

int main(void) {
    const int anchoInicial = 1140;
    const int altoInicial = 760;
    Pila pila;
    Font fuenteUi;
    bool fuentePersonalizada = false;
    int siguienteValor = 10;
    int scrollPila = 0;
    float scrollVistaX = 0.0f;
    char estado[256];
    char vistaPila[1024];

    pila_inicializar(&pila);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(anchoInicial, altoInicial, "Metodos de pilas en C - Visual con raylib");
    SetWindowMinSize(980, 660);
    SetTargetFPS(60);

    fuenteUi = cargar_fuente_ui(&fuentePersonalizada);

    strcpy(estado, "Listo. Usa A:push, D:pop, M:mostrar, X:destruir, R:reiniciar.");
    pila_formatear(&pila, vistaPila, sizeof(vistaPila));

    while (!WindowShouldClose()) {
        int ancho = GetScreenWidth();
        int alto = GetScreenHeight();
        Rectangle rEstado = {30, 178, (float)ancho - 60.0f, 72};
        Rectangle rVista = {30, 262, (float)ancho - 60.0f, 84};
        Rectangle rPila = {30, 360, (float)ancho - 60.0f, (float)alto - 380.0f};
        Rectangle viewportVista = {390, 287, (float)ancho - 430.0f, 38};
        Vector2 mouse = GetMousePosition();
        float wheel = GetMouseWheelMove();
        float vistaWidth;
        float vistaMaxScroll;
        int maxScrollPila;

        if (rPila.height < 220.0f) {
            rPila.height = 220.0f;
        }

        if (CheckCollisionPointRec(mouse, rPila) && wheel != 0.0f) {
            scrollPila -= (int)wheel;
        } else if (CheckCollisionPointRec(mouse, viewportVista) && wheel != 0.0f) {
            scrollVistaX -= wheel * 42.0f;
        }

        if (IsKeyPressed(KEY_UP)) scrollPila--;
        if (IsKeyPressed(KEY_DOWN)) scrollPila++;
        if (IsKeyDown(KEY_LEFT)) scrollVistaX -= 240.0f * GetFrameTime();
        if (IsKeyDown(KEY_RIGHT)) scrollVistaX += 240.0f * GetFrameTime();

        if (IsKeyPressed(KEY_A)) {
            if (pila_push(&pila, siguienteValor)) {
                snprintf(estado, sizeof(estado), "push(%d) ejecutado", siguienteValor);
                siguienteValor += 10;
            } else {
                snprintf(estado, sizeof(estado), "push() fallido: sin memoria");
            }
            pila_formatear(&pila, vistaPila, sizeof(vistaPila));
        }

        if (IsKeyPressed(KEY_D)) {
            int valor = 0;
            if (pila_pop(&pila, &valor)) {
                snprintf(estado, sizeof(estado), "pop() -> %d", valor);
            } else {
                snprintf(estado, sizeof(estado), "pop() no realizado: pila vacia");
            }
            pila_formatear(&pila, vistaPila, sizeof(vistaPila));
        }

        if (IsKeyPressed(KEY_M)) {
            pila_formatear(&pila, vistaPila, sizeof(vistaPila));
            snprintf(estado, sizeof(estado), "mostrar_pila() ejecutado");
        }

        if (IsKeyPressed(KEY_X)) {
            pila_destruir(&pila);
            snprintf(estado, sizeof(estado), "destruir_pila() ejecutado");
            pila_formatear(&pila, vistaPila, sizeof(vistaPila));
        }

        if (IsKeyPressed(KEY_R)) {
            pila_destruir(&pila);
            siguienteValor = 10;
            snprintf(estado, sizeof(estado), "escena reiniciada. contador push vuelve a 10");
            pila_formatear(&pila, vistaPila, sizeof(vistaPila));
        }

        maxScrollPila = calcular_max_scroll_pila(rPila, pila_contar(&pila));
        scrollPila = clamp_int(scrollPila, 0, maxScrollPila);

        vistaWidth = MeasureTextEx(fuenteUi, vistaPila, 33, 1.0f).x;
        vistaMaxScroll = vistaWidth - viewportVista.width;
        if (vistaMaxScroll < 0.0f) vistaMaxScroll = 0.0f;
        scrollVistaX = clamp_float(scrollVistaX, 0.0f, vistaMaxScroll);

        BeginDrawing();
        ClearBackground((Color){239, 246, 255, 255});

        DrawRectangleGradientV(0, 0, ancho, alto, (Color){188, 217, 240, 255}, (Color){238, 246, 252, 255});
        draw_text(fuenteUi, "Visualizador de metodos de PILA", 30, 22, 58, (Color){13, 72, 138, 255});
        draw_text(fuenteUi, "Controles: [A] Push   [D] Pop   [M] Mostrar   [X] Destruir   [R] Reiniciar   [ESC] Salir", 34, 90, 36, (Color){19, 24, 30, 255});
        draw_text(fuenteUi, TextFormat("Cantidad de nodos: %d", pila_contar(&pila)), 34, 136, 34, (Color){57, 67, 78, 255});
        draw_text(fuenteUi, TextFormat("Pila vacia: %s", pila_vacia(&pila) ? "si" : "no"), 360, 136, 34, (Color){57, 67, 78, 255});

        dibujar_tarjeta(rEstado, (Color){227, 233, 239, 245}, (Color){126, 136, 148, 255}, 0.08f);
        draw_text(fuenteUi, "Ultimo metodo:", 48, 198, 33, (Color){170, 17, 55, 255});
        draw_text(fuenteUi, estado, 270, 198, 33, (Color){22, 24, 28, 255});

        dibujar_tarjeta(rVista, (Color){230, 219, 194, 238}, (Color){181, 150, 102, 255}, 0.08f);
        draw_text(fuenteUi, "Vista logica (mostrar_pila):", 48, 287, 33, (Color){10, 124, 66, 255});
        BeginScissorMode((int)viewportVista.x, (int)viewportVista.y, (int)viewportVista.width, (int)viewportVista.height);
        draw_text(fuenteUi, vistaPila, viewportVista.x - scrollVistaX, viewportVista.y, 33, (Color){20, 24, 28, 255});
        EndScissorMode();

        if (vistaMaxScroll > 0.0f) {
            Rectangle track = {viewportVista.x, viewportVista.y + viewportVista.height + 8, viewportVista.width, 8};
            float thumbWidth = track.width * (viewportVista.width / vistaWidth);
            float t = (vistaMaxScroll > 0.0f) ? (scrollVistaX / vistaMaxScroll) : 0.0f;
            Rectangle thumb;

            if (thumbWidth < 40.0f) thumbWidth = 40.0f;
            thumb = (Rectangle){
                track.x + (track.width - thumbWidth) * t,
                track.y,
                thumbWidth,
                track.height
            };

            DrawRectangleRounded(track, 0.45f, 8, Fade((Color){130, 100, 48, 255}, 0.25f));
            DrawRectangleRounded(thumb, 0.45f, 8, (Color){181, 150, 102, 255});
        }

        dibujar_pila(fuenteUi, &pila, rPila, &scrollPila);

        EndDrawing();
    }

    pila_destruir(&pila);
    if (fuentePersonalizada) {
        UnloadFont(fuenteUi);
    }
    CloseWindow();
    return 0;
}
