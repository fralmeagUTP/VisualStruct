#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "cola.h"

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
    DrawTextEx(font, texto, (Vector2){x, y}, size, 1.0f, color);
}

static void dibujar_tarjeta(Rectangle r, Color fondo, Color borde, float radio) {
    Rectangle sombra = {r.x + 2, r.y + 3, r.width, r.height};
    DrawRectangleRounded(sombra, radio, 10, Fade(BLACK, 0.10f));
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

static void dibujar_cola(
    Font font,
    const Cola *cola,
    Rectangle area,
    float *scrollX
) {
    int valores[256];
    int total = cola_copiar_valores(cola, valores, 256);
    const float paddingX = 18.0f;
    const float topOffset = 44.0f;
    const float bottomOffset = 26.0f;
    const float boxWidth = 138.0f;
    const float boxHeight = 68.0f;
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
    float contenidoAncho = 0.0f;
    float maxScroll = 0.0f;
    int i;

    dibujar_tarjeta(area, (Color){219, 240, 255, 248}, (Color){22, 95, 165, 255}, 0.04f);
    draw_text(font, "Representacion de cola (frente -> final)", area.x + 16, area.y + 10, 30, (Color){12, 75, 138, 255});

    if (total == 0) {
        draw_text(font, "Cola vacia", area.x + 24, area.y + 70, 33, (Color){92, 101, 112, 255});
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
        Rectangle item = {x, viewport.y + 22.0f, boxWidth, boxHeight};
        Color borde = (Color){31, 119, 196, 255};
        Color fondo = (Color){190, 221, 245, 236};

        if (i == 0) {
            borde = (Color){245, 138, 24, 255};
            fondo = (Color){255, 234, 203, 245};
        } else if (i == total - 1) {
            borde = (Color){24, 148, 88, 255};
            fondo = (Color){206, 242, 221, 245};
        }

        DrawRectangleRounded(item, 0.15f, 6, fondo);
        DrawRectangleRoundedLinesEx(item, 0.15f, 6, 2.0f, borde);
        draw_text(font, TextFormat("%d", valores[i]), item.x + 52, item.y + 18, 34, (Color){18, 70, 126, 255});

        if (i == 0) {
            draw_text(font, "FRONT", item.x + 20, item.y - 24, 22, (Color){178, 48, 12, 255});
        }
        if (i == total - 1) {
            draw_text(font, "BACK", item.x + 28, item.y + item.height + 5, 22, (Color){14, 114, 64, 255});
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
        }
    }
    EndScissorMode();

    dibujar_scroll_horizontal(scrollTrack, contenidoAncho, viewport.width, scrollX != NULL ? *scrollX : 0.0f);
    if (maxScroll > 0.0f) {
        draw_text(font, "Scroll horizontal: rueda del mouse o flechas <- ->", area.x + 18, area.y + area.height - 34, 22, (Color){146, 45, 26, 255});
    }
}

int main(void) {
    const int anchoInicial = 1180;
    const int altoInicial = 760;
    Cola cola;
    Font fuenteUi;
    bool fuentePersonalizada = false;
    int siguienteValor = 10;
    float scrollColaX = 0.0f;
    char estado[256];
    char vistaCola[1024];

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(anchoInicial, altoInicial, "Visualizador de metodos de COLA con raylib");
    SetWindowMinSize(980, 640);
    SetTargetFPS(60);

    cola_inicializar(&cola);
    fuenteUi = cargar_fuente_ui(&fuentePersonalizada);
    strcpy(estado, "Listo. Usa E:encolar, D:desencolar, M:mostrar, V:vaciar, R:reiniciar.");
    cola_formatear(&cola, vistaCola, sizeof(vistaCola));

    while (!WindowShouldClose()) {
        int ancho = GetScreenWidth();
        int alto = GetScreenHeight();
        Rectangle rEstado = {30, 174, (float)ancho - 60.0f, 72};
        Rectangle rVista = {30, 258, (float)ancho - 60.0f, 84};
        Rectangle rCola = {30, 356, (float)ancho - 60.0f, (float)alto - 376.0f};
        Rectangle viewportVista = {362, 283, (float)ancho - 400.0f, 38};
        Vector2 mouse = GetMousePosition();
        float wheel = GetMouseWheelMove();
        float anchoTextoVista;
        float maxScrollVista = 0.0f;
        static float scrollVista = 0.0f;

        if (rCola.height < 230.0f) {
            rCola.height = 230.0f;
        }

        if (IsKeyPressed(KEY_E)) {
            if (cola_encolar(&cola, siguienteValor)) {
                snprintf(estado, sizeof(estado), "encolar(%d) ejecutado", siguienteValor);
                siguienteValor += 10;
            } else {
                snprintf(estado, sizeof(estado), "encolar() fallido: sin memoria");
            }
            cola_formatear(&cola, vistaCola, sizeof(vistaCola));
        }

        if (IsKeyPressed(KEY_D)) {
            int valor = 0;
            if (cola_desencolar(&cola, &valor)) {
                snprintf(estado, sizeof(estado), "desencolar() -> %d", valor);
            } else {
                snprintf(estado, sizeof(estado), "desencolar() no realizado: cola vacia");
            }
            cola_formatear(&cola, vistaCola, sizeof(vistaCola));
        }

        if (IsKeyPressed(KEY_M)) {
            cola_formatear(&cola, vistaCola, sizeof(vistaCola));
            snprintf(estado, sizeof(estado), "mostrar_cola() ejecutado");
        }

        if (IsKeyPressed(KEY_V)) {
            cola_vaciar(&cola);
            snprintf(estado, sizeof(estado), "vaciar_cola() ejecutado");
            cola_formatear(&cola, vistaCola, sizeof(vistaCola));
            scrollColaX = 0.0f;
            scrollVista = 0.0f;
        }

        if (IsKeyPressed(KEY_R)) {
            cola_vaciar(&cola);
            siguienteValor = 10;
            snprintf(estado, sizeof(estado), "escena reiniciada. contador encolar vuelve a 10");
            cola_formatear(&cola, vistaCola, sizeof(vistaCola));
            scrollColaX = 0.0f;
            scrollVista = 0.0f;
        }

        anchoTextoVista = MeasureTextEx(fuenteUi, vistaCola, 33, 1.0f).x;
        maxScrollVista = anchoTextoVista - viewportVista.width;
        if (maxScrollVista < 0.0f) maxScrollVista = 0.0f;

        if (CheckCollisionPointRec(mouse, rCola) && wheel != 0.0f) {
            scrollColaX -= wheel * 42.0f;
        } else if (CheckCollisionPointRec(mouse, viewportVista) && wheel != 0.0f) {
            scrollVista -= wheel * 42.0f;
        }

        if (IsKeyDown(KEY_LEFT)) {
            scrollColaX -= 280.0f * GetFrameTime();
            scrollVista -= 280.0f * GetFrameTime();
        }
        if (IsKeyDown(KEY_RIGHT)) {
            scrollColaX += 280.0f * GetFrameTime();
            scrollVista += 280.0f * GetFrameTime();
        }

        scrollVista = clamp_float(scrollVista, 0.0f, maxScrollVista);

        BeginDrawing();
        ClearBackground((Color){239, 247, 255, 255});

        DrawRectangleGradientV(0, 0, ancho, alto, (Color){187, 218, 242, 255}, (Color){236, 246, 253, 255});
        draw_text(fuenteUi, "Visualizador de metodos de COLA", 30, 22, 56, (Color){12, 72, 136, 255});
        draw_text(fuenteUi, "Controles: [E] Encolar   [D] Desencolar   [M] Mostrar   [V] Vaciar   [R] Reiniciar   [ESC] Salir", 34, 88, 34, (Color){16, 23, 30, 255});
        draw_text(fuenteUi, TextFormat("Cantidad de nodos: %d", cola_contar(&cola)), 34, 134, 33, (Color){54, 65, 77, 255});
        draw_text(fuenteUi, TextFormat("Cola vacia: %s", cola_vacia(&cola) ? "si" : "no"), 348, 134, 33, (Color){54, 65, 77, 255});

        dibujar_tarjeta(rEstado, (Color){226, 233, 240, 245}, (Color){124, 134, 147, 255}, 0.08f);
        draw_text(fuenteUi, "Ultimo metodo:", 46, 194, 32, (Color){170, 17, 55, 255});
        draw_text(fuenteUi, estado, 272, 194, 32, (Color){22, 25, 28, 255});

        dibujar_tarjeta(rVista, (Color){231, 220, 196, 238}, (Color){182, 149, 100, 255}, 0.08f);
        draw_text(fuenteUi, "Vista logica:", 46, 282, 32, (Color){10, 124, 66, 255});
        BeginScissorMode((int)viewportVista.x, (int)viewportVista.y, (int)viewportVista.width, (int)viewportVista.height);
        draw_text(fuenteUi, vistaCola, viewportVista.x - scrollVista, viewportVista.y, 33, (Color){20, 24, 28, 255});
        EndScissorMode();
        dibujar_scroll_horizontal((Rectangle){viewportVista.x, viewportVista.y + viewportVista.height + 8, viewportVista.width, 8}, anchoTextoVista, viewportVista.width, scrollVista);

        dibujar_cola(fuenteUi, &cola, rCola, &scrollColaX);

        EndDrawing();
    }

    cola_vaciar(&cola);
    if (fuentePersonalizada) {
        UnloadFont(fuenteUi);
    }
    CloseWindow();
    return 0;
}
