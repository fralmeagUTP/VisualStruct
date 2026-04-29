#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "lista.h"

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

        if (chipW < 120.0f) chipW = 120.0f;
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

        if (chipW < 120.0f) chipW = 120.0f;
        if (chipW > maxWidth) chipW = maxWidth;
        if (cx + chipW > x + maxWidth) {
            cx = x;
            cy += chipHeight + 8.0f;
        }
        cx += chipW + gap;
    }

    return cy + chipHeight;
}

static int clamp_int(int v, int min, int max) {
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

static float clamp_float(float v, float min, float max) {
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

static int parse_posicion(const char *texto) {
    int pos = 0;
    if (texto == NULL) {
        return 0;
    }
    if (sscanf(texto, "%d", &pos) != 1) {
        return 0;
    }
    return pos;
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

static void dibujar_lista(
    Font font,
    const Lista *lista,
    Rectangle area,
    float *scrollX
) {
    int valores[256];
    int total = lista_copiar_valores(lista, valores, 256);
    const float paddingX = 18.0f;
    const float topOffset = 44.0f;
    const float bottomOffset = 26.0f;
    const float boxWidth = 126.0f;
    const float boxHeight = 62.0f;
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
    draw_text(font, "Representacion de lista enlazada", area.x + 16, area.y + 10, 30, (Color){12, 75, 138, 255});

    if (total == 0) {
        draw_text(font, "Lista vacia", area.x + 24, area.y + 70, 33, (Color){92, 101, 112, 255});
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
        Rectangle item = {x, viewport.y + 18.0f, boxWidth, boxHeight};
        Color borde = (i == 0) ? (Color){245, 138, 24, 255} : (Color){31, 119, 196, 255};
        Color fondo = (i == 0) ? (Color){255, 234, 203, 245} : (Color){190, 221, 245, 236};

        DrawRectangleRounded(item, 0.15f, 6, fondo);
        DrawRectangleRoundedLinesEx(item, 0.15f, 6, 2.0f, borde);
        draw_text(font, TextFormat("[%d] %d", i + 1, valores[i]), item.x + 16, item.y + 16, 28, (Color){18, 70, 126, 255});

        if (i == 0) {
            draw_text(font, "HEAD", item.x + 20, item.y - 22, 22, (Color){178, 48, 12, 255});
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
            draw_text(font, "NULL", item.x + item.width + 10, item.y + 16, 24, (Color){130, 54, 31, 255});
        }
    }
    EndScissorMode();

    dibujar_scroll_horizontal(scrollTrack, contenidoAncho, viewport.width, scrollX != NULL ? *scrollX : 0.0f);
    if (maxScroll > 0.0f) {
        draw_text(font, "Scroll: rueda del mouse sobre el panel o flechas <- ->", area.x + 18, area.y + area.height - 34, 22, (Color){146, 45, 26, 255});
    }
}

static void dibujar_modal_posicion(Font font, int ancho, int alto, const char *titulo, const char *input, bool blink) {
    Rectangle caja = {(float)ancho * 0.5f - 260.0f, (float)alto * 0.5f - 95.0f, 520.0f, 190.0f};
    Rectangle txt = {caja.x + 22.0f, caja.y + 76.0f, caja.width - 44.0f, 52.0f};
    char valorMostrado[64];
    size_t lenInput = strlen(input);

    DrawRectangle(0, 0, ancho, alto, Fade(BLACK, 0.42f));
    DrawRectangleRounded(caja, 0.08f, 10, (Color){245, 249, 255, 255});
    DrawRectangleRoundedLinesEx(caja, 0.08f, 10, 2.0f, (Color){78, 117, 157, 255});

    draw_text(font, titulo, caja.x + 22.0f, caja.y + 18.0f, 30, (Color){14, 73, 132, 255});

    DrawRectangleRounded(txt, 0.12f, 8, (Color){255, 255, 255, 255});
    DrawRectangleRoundedLinesEx(txt, 0.12f, 8, 2.0f, (Color){64, 124, 182, 255});

    if (lenInput == 0) {
        strcpy(valorMostrado, blink ? "|" : "");
    } else {
        snprintf(valorMostrado, sizeof(valorMostrado), "%s%s", input, blink ? "|" : "");
    }
    draw_text(font, valorMostrado, txt.x + 14.0f, txt.y + 10.0f, 34, (Color){20, 34, 52, 255});

    draw_text(font, "ENTER: confirmar    ESC: cancelar", caja.x + 22.0f, caja.y + 142.0f, 22, (Color){79, 96, 116, 255});
}

int main(void) {
    const int anchoInicial = 1260;
    const int altoInicial = 800;
    Lista lista;
    Font fuenteUi;
    bool fuentePersonalizada = false;
    int valorActual = 10;
    int posicionActual = 1;
    int pasoAuto = 10;
    bool autoIncremento = true;
    bool modoCompacto = false;
    float scrollListaX = 0.0f;
    float scrollVistaX = 0.0f;
    char estado[256];
    char vistaLista[1400];
    char infoBusqueda[256];
    bool modalPosicionActivo = false;
    bool modalInsertarAntes = true;
    char modalPosicionInput[16] = "";
    int modalPosicionLen = 0;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(anchoInicial, altoInicial, "Visualizador de metodos de LISTA con raylib (v2)");
    SetWindowMinSize(1020, 680);
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    lista_inicializar(&lista);
    fuenteUi = cargar_fuente_ui(&fuentePersonalizada);

    strcpy(estado, "Listo. I inicio, F final, B antes(pos), N despues(pos), X eliminar uno, T eliminar todos.");
    strcpy(infoBusqueda, "Busqueda: usa S para buscar el valor actual.");
    lista_formatear(&lista, vistaLista, sizeof(vistaLista));

    while (!WindowShouldClose()) {
        int ancho = GetScreenWidth();
        int alto = GetScreenHeight();
        Rectangle rEstado;
        Rectangle rVista;
        Rectangle rBusqueda;
        Rectangle rLista;
        Rectangle viewportVista;
        Vector2 mouse = GetMousePosition();
        float wheel = GetMouseWheelMove();
        float anchoTextoVista;
        float maxScrollVista;
        int cuenta = lista_contar(&lista);
        float prom;
        int maximo;
        bool ordenAsc = lista_orden_asc(&lista);
        bool tieneProm = lista_promedio(&lista, &prom);
        bool tieneMayor = lista_mayor(&lista, &maximo);
        float layoutBaseY;
        float controlsSize = (ancho < 1150) ? 25.0f : 29.0f;
        float line1Y = 84.0f;
        float line2Y = line1Y + controlsSize + 6.0f;
        float chipsY = line2Y + controlsSize + 8.0f;
        float chipHeight = 38.0f;
        float gapTarjetas = 12.0f;
        float hEstado = 72.0f;
        float hVista = 84.0f;
        float hBusqueda = 52.0f;
        bool mostrarBusqueda = true;
        char mValor[80];
        char mPos[80];
        char mAuto[80];
        char mNodos[80];
        char mProm[80];
        char mMayor[80];
        char mOrden[80];
        char mModo[80];
        const char *chips[8];
        float labelW;

        snprintf(mValor, sizeof(mValor), "Valor actual: %d", valorActual);
        snprintf(mPos, sizeof(mPos), "Posicion actual: %d", posicionActual);
        snprintf(mAuto, sizeof(mAuto), "AutoIncremento: %s", autoIncremento ? "ON" : "OFF");
        snprintf(mNodos, sizeof(mNodos), "Nodos: %d", cuenta);
        snprintf(mProm, sizeof(mProm), "Promedio: %s", tieneProm ? TextFormat("%.2f", prom) : "n/a");
        snprintf(mMayor, sizeof(mMayor), "Mayor: %s", tieneMayor ? TextFormat("%d", maximo) : "n/a");
        snprintf(mOrden, sizeof(mOrden), "Orden ASC: %s", ordenAsc ? "si" : "no");
        chips[0] = mValor;
        chips[1] = mPos;
        chips[2] = mAuto;
        chips[3] = mNodos;
        chips[4] = mProm;
        chips[5] = mMayor;
        chips[6] = mOrden;

        if (modalPosicionActivo) {
            int key = GetCharPressed();
            while (key > 0) {
                if (key >= '0' && key <= '9' && modalPosicionLen < (int)sizeof(modalPosicionInput) - 1) {
                    modalPosicionInput[modalPosicionLen++] = (char)key;
                    modalPosicionInput[modalPosicionLen] = '\0';
                }
                key = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE) && modalPosicionLen > 0) {
                modalPosicionLen--;
                modalPosicionInput[modalPosicionLen] = '\0';
            }

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
                int posIngresada = parse_posicion(modalPosicionInput);
                bool ok = false;

                if (posIngresada >= 1) {
                    if (modalInsertarAntes) {
                        ok = lista_insertar_antes(&lista, valorActual, posIngresada);
                        if (ok) {
                            snprintf(estado, sizeof(estado), "insertar_antes(valor=%d, pos=%d) ejecutado", valorActual, posIngresada);
                        } else {
                            snprintf(estado, sizeof(estado), "insertar_antes() fallo: posicion invalida o sin memoria");
                        }
                    } else {
                        ok = lista_insertar_despues(&lista, valorActual, posIngresada);
                        if (ok) {
                            snprintf(estado, sizeof(estado), "insertar_despues(valor=%d, pos=%d) ejecutado", valorActual, posIngresada);
                        } else {
                            snprintf(estado, sizeof(estado), "insertar_despues() fallo: posicion invalida o sin memoria");
                        }
                    }

                    if (ok) {
                        posicionActual = posIngresada;
                        if (autoIncremento) valorActual += pasoAuto;
                        if (!modalInsertarAntes && posIngresada >= cuenta) {
                            scrollListaX = 1e9f;
                        }
                    }
                    lista_formatear(&lista, vistaLista, sizeof(vistaLista));
                } else {
                    snprintf(estado, sizeof(estado), "Posicion invalida. Ingresa un numero >= 1.");
                }

                modalPosicionActivo = false;
                modalPosicionInput[0] = '\0';
                modalPosicionLen = 0;
            }

            if (IsKeyPressed(KEY_ESCAPE)) {
                modalPosicionActivo = false;
                modalPosicionInput[0] = '\0';
                modalPosicionLen = 0;
                snprintf(estado, sizeof(estado), "Insercion cancelada");
            }
        } else {
            if (IsKeyPressed(KEY_ESCAPE)) {
                break;
            }

            if (IsKeyPressed(KEY_UP)) valorActual++;
            if (IsKeyPressed(KEY_DOWN)) valorActual--;
            if (IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD)) valorActual += pasoAuto;
            if (IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT)) valorActual -= pasoAuto;
            if (IsKeyPressed(KEY_RIGHT)) posicionActual++;
            if (IsKeyPressed(KEY_LEFT)) posicionActual--;
            if (IsKeyPressed(KEY_A)) autoIncremento = !autoIncremento;
            if (IsKeyPressed(KEY_C)) modoCompacto = !modoCompacto;
            if (posicionActual < 1) posicionActual = 1;
            if (cuenta > 0) posicionActual = clamp_int(posicionActual, 1, cuenta);
            if (cuenta == 0) posicionActual = 1;

            if (IsKeyPressed(KEY_I)) {
                if (lista_insertar_inicio(&lista, valorActual)) {
                    snprintf(estado, sizeof(estado), "insertar_inicio(%d) ejecutado", valorActual);
                    if (autoIncremento) valorActual += pasoAuto;
                    scrollListaX = 0.0f;
                } else {
                    snprintf(estado, sizeof(estado), "insertar_inicio() fallido: sin memoria");
                }
                lista_formatear(&lista, vistaLista, sizeof(vistaLista));
            }

            if (IsKeyPressed(KEY_F)) {
                if (lista_insertar_final(&lista, valorActual)) {
                    snprintf(estado, sizeof(estado), "insertar_final(%d) ejecutado", valorActual);
                    if (autoIncremento) valorActual += pasoAuto;
                    scrollListaX = 1e9f;
                } else {
                    snprintf(estado, sizeof(estado), "insertar_final() fallido: sin memoria");
                }
                lista_formatear(&lista, vistaLista, sizeof(vistaLista));
            }

            if (IsKeyPressed(KEY_B)) {
                modalPosicionActivo = true;
                modalInsertarAntes = true;
                snprintf(modalPosicionInput, sizeof(modalPosicionInput), "%d", posicionActual);
                modalPosicionLen = (int)strlen(modalPosicionInput);
                snprintf(estado, sizeof(estado), "Insercion antes: ingresa posicion y ENTER");
            }

            if (IsKeyPressed(KEY_N)) {
                modalPosicionActivo = true;
                modalInsertarAntes = false;
                snprintf(modalPosicionInput, sizeof(modalPosicionInput), "%d", posicionActual);
                modalPosicionLen = (int)strlen(modalPosicionInput);
                snprintf(estado, sizeof(estado), "Insercion despues: ingresa posicion y ENTER");
            }

            if (IsKeyPressed(KEY_X)) {
                if (lista_eliminar_primero(&lista, valorActual)) {
                    snprintf(estado, sizeof(estado), "eliminar_primero(valor=%d) ejecutado", valorActual);
                } else {
                    snprintf(estado, sizeof(estado), "eliminar_primero() no elimino: valor no encontrado");
                }
                lista_formatear(&lista, vistaLista, sizeof(vistaLista));
            }

            if (IsKeyPressed(KEY_T)) {
                int borrados = lista_eliminar_todos(&lista, valorActual);
                snprintf(estado, sizeof(estado), "eliminar_todos(valor=%d) -> %d eliminados", valorActual, borrados);
                lista_formatear(&lista, vistaLista, sizeof(vistaLista));
            }

            if (IsKeyPressed(KEY_V)) {
                lista_invertir(&lista);
                snprintf(estado, sizeof(estado), "invertir() ejecutado");
                lista_formatear(&lista, vistaLista, sizeof(vistaLista));
            }

            if (IsKeyPressed(KEY_S)) {
                int posiciones[16];
                int encontrados = lista_buscar_posiciones(&lista, valorActual, posiciones, 16);
                if (encontrados == 0) {
                    snprintf(infoBusqueda, sizeof(infoBusqueda), "Busqueda valor %d: no encontrado", valorActual);
                } else if (encontrados == 1) {
                    snprintf(infoBusqueda, sizeof(infoBusqueda), "Busqueda valor %d: encontrado en posicion %d", valorActual, posiciones[0]);
                } else {
                    snprintf(infoBusqueda, sizeof(infoBusqueda), "Busqueda valor %d: %d coincidencias (primera pos %d)", valorActual, encontrados, posiciones[0]);
                }
                snprintf(estado, sizeof(estado), "buscar_posiciones(valor=%d) ejecutado", valorActual);
            }

            if (IsKeyPressed(KEY_P)) {
                if (lista_promedio(&lista, &prom)) {
                    snprintf(estado, sizeof(estado), "promedio() = %.2f", prom);
                } else {
                    snprintf(estado, sizeof(estado), "promedio() no disponible: lista vacia");
                }
            }

            if (IsKeyPressed(KEY_G)) {
                if (lista_mayor(&lista, &maximo)) {
                    snprintf(estado, sizeof(estado), "mayor() = %d", maximo);
                } else {
                    snprintf(estado, sizeof(estado), "mayor() no disponible: lista vacia");
                }
            }

            if (IsKeyPressed(KEY_O)) {
                snprintf(estado, sizeof(estado), "orden_asc() = %s", ordenAsc ? "true" : "false");
            }

            if (IsKeyPressed(KEY_R)) {
                lista_destruir(&lista);
                valorActual = 10;
                posicionActual = 1;
                scrollListaX = 0.0f;
                scrollVistaX = 0.0f;
                strcpy(infoBusqueda, "Busqueda: usa S para buscar el valor actual.");
                snprintf(estado, sizeof(estado), "escena reiniciada");
                lista_formatear(&lista, vistaLista, sizeof(vistaLista));
            }

            if (IsKeyPressed(KEY_M)) {
                lista_formatear(&lista, vistaLista, sizeof(vistaLista));
                snprintf(estado, sizeof(estado), "mostrar_lista() ejecutado");
            }
        }

        ordenAsc = lista_orden_asc(&lista);
        cuenta = lista_contar(&lista);
        tieneProm = lista_promedio(&lista, &prom);
        tieneMayor = lista_mayor(&lista, &maximo);

        snprintf(mValor, sizeof(mValor), "Valor actual: %d", valorActual);
        snprintf(mPos, sizeof(mPos), "Posicion actual: %d", posicionActual);
        snprintf(mAuto, sizeof(mAuto), "AutoIncremento: %s", autoIncremento ? "ON" : "OFF");
        snprintf(mNodos, sizeof(mNodos), "Nodos: %d", cuenta);
        snprintf(mProm, sizeof(mProm), "Promedio: %s", tieneProm ? TextFormat("%.2f", prom) : "n/a");
        snprintf(mMayor, sizeof(mMayor), "Mayor: %s", tieneMayor ? TextFormat("%d", maximo) : "n/a");
        snprintf(mOrden, sizeof(mOrden), "Orden ASC: %s", ordenAsc ? "si" : "no");
        snprintf(mModo, sizeof(mModo), "Modo: %s", modoCompacto ? "Compacto" : "Detallado");
        chips[0] = mValor;
        chips[1] = mPos;
        chips[2] = mAuto;
        chips[3] = mNodos;
        chips[4] = mProm;
        chips[5] = mMayor;
        chips[6] = mOrden;
        chips[7] = mModo;

        if (modoCompacto) {
            controlsSize = (ancho < 1150) ? 24.0f : 28.0f;
            line2Y = -1000.0f;
            chipsY = line1Y + controlsSize + 10.0f;
            chipHeight = 34.0f;
            gapTarjetas = 10.0f;
            hEstado = 62.0f;
            hVista = 72.0f;
            hBusqueda = 0.0f;
            mostrarBusqueda = false;
        }

        layoutBaseY = calcular_bottom_chips(fuenteUi, 34.0f, chipsY, (float)ancho - 68.0f, chipHeight, 8.0f, chips, 8) + gapTarjetas;
        rEstado = (Rectangle){30, layoutBaseY, (float)ancho - 60.0f, hEstado};
        rVista = (Rectangle){30, rEstado.y + rEstado.height + gapTarjetas, (float)ancho - 60.0f, hVista};
        rBusqueda = (Rectangle){30, rVista.y + rVista.height + gapTarjetas, (float)ancho - 60.0f, hBusqueda};
        if (mostrarBusqueda) {
            rLista = (Rectangle){30, rBusqueda.y + rBusqueda.height + gapTarjetas, (float)ancho - 60.0f, (float)alto - (rBusqueda.y + rBusqueda.height + 28.0f)};
        } else {
            rLista = (Rectangle){30, rVista.y + rVista.height + gapTarjetas, (float)ancho - 60.0f, (float)alto - (rVista.y + rVista.height + 28.0f)};
        }
        if (rLista.height < 220.0f) rLista.height = 220.0f;

        labelW = MeasureTextEx(fuenteUi, "Vista logica:", 31, 1.0f).x;
        viewportVista = (Rectangle){
            rVista.x + 16.0f + labelW + 16.0f,
            rVista.y + 18.0f,
            rVista.width - (labelW + 48.0f),
            38.0f
        };
        if (viewportVista.width < 120.0f) viewportVista.width = 120.0f;

        anchoTextoVista = MeasureTextEx(fuenteUi, vistaLista, 31, 1.0f).x;
        maxScrollVista = anchoTextoVista - viewportVista.width;
        if (maxScrollVista < 0.0f) maxScrollVista = 0.0f;

        if (CheckCollisionPointRec(mouse, rLista) && wheel != 0.0f) {
            scrollListaX -= wheel * 42.0f;
        } else if (CheckCollisionPointRec(mouse, viewportVista) && wheel != 0.0f) {
            scrollVistaX -= wheel * 42.0f;
        }
        scrollVistaX = clamp_float(scrollVistaX, 0.0f, maxScrollVista);

        BeginDrawing();
        ClearBackground((Color){239, 247, 255, 255});

        DrawRectangleGradientV(0, 0, ancho, alto, (Color){187, 218, 242, 255}, (Color){236, 246, 253, 255});
        draw_text(fuenteUi, "Visualizador de metodos de LISTA", 30, 22, 54, (Color){12, 72, 136, 255});
        if (modoCompacto) {
            draw_text(fuenteUi, "I/F/B/N insertar | X/T eliminar | V invertir | S buscar | P/G/O analisis | M | R | A | C", 34, line1Y, controlsSize, (Color){16, 23, 30, 255});
        } else {
            draw_text(fuenteUi, "I inicio | F final | B antes(pos) | N despues(pos) | X elimina uno | T elimina todos | V invertir", 34, line1Y, controlsSize, (Color){16, 23, 30, 255});
            draw_text(fuenteUi, "S buscar | P promedio | G mayor | O orden asc | M mostrar | R reiniciar | A auto(+10) | C compacto | ESC salir", 34, line2Y, controlsSize, (Color){16, 23, 30, 255});
        }

        layoutBaseY = dibujar_chips(fuenteUi, 34.0f, chipsY, (float)ancho - 68.0f, chipHeight, 8.0f, chips, 8) + gapTarjetas;

        dibujar_tarjeta(rEstado, (Color){226, 233, 240, 245}, (Color){124, 134, 147, 255}, 0.08f);
        draw_text(fuenteUi, "Ultimo metodo:", 46, rEstado.y + 20, 31, (Color){170, 17, 55, 255});
        draw_text(fuenteUi, estado, 260, rEstado.y + 20, 31, (Color){22, 25, 28, 255});

        dibujar_tarjeta(rVista, (Color){231, 220, 196, 238}, (Color){182, 149, 100, 255}, 0.08f);
        draw_text(fuenteUi, "Vista logica:", 46, rVista.y + (modoCompacto ? 16.0f : 20.0f), modoCompacto ? 28 : 31, (Color){10, 124, 66, 255});
        BeginScissorMode((int)viewportVista.x, (int)viewportVista.y, (int)viewportVista.width, (int)viewportVista.height);
        draw_text(fuenteUi, vistaLista, viewportVista.x - scrollVistaX, viewportVista.y, modoCompacto ? 28 : 31, (Color){20, 24, 28, 255});
        EndScissorMode();
        dibujar_scroll_horizontal((Rectangle){viewportVista.x, viewportVista.y + viewportVista.height + 8, viewportVista.width, 8}, anchoTextoVista, viewportVista.width, scrollVistaX);

        if (mostrarBusqueda) {
            dibujar_tarjeta(rBusqueda, (Color){236, 247, 232, 240}, (Color){111, 164, 103, 255}, 0.08f);
            draw_text(fuenteUi, infoBusqueda, 46, rBusqueda.y + 14, 28, (Color){18, 90, 39, 255});
        }

        dibujar_lista(fuenteUi, &lista, rLista, &scrollListaX);

        if (modalPosicionActivo) {
            const char *titulo = modalInsertarAntes
                ? "Insertar ANTES: posicion objetivo"
                : "Insertar DESPUES: posicion objetivo";
            bool blink = ((int)(GetTime() * 2.0) % 2) == 0;
            dibujar_modal_posicion(fuenteUi, ancho, alto, titulo, modalPosicionInput, blink);
        }

        EndDrawing();
    }

    lista_destruir(&lista);
    if (fuentePersonalizada) {
        UnloadFont(fuenteUi);
    }
    CloseWindow();
    return 0;
}
