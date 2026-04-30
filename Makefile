CC = gcc
CSTD = -std=c11
CWARN = -Wall -Wextra -pedantic
CFLAGS = $(CSTD) $(CWARN) -Iinclude

SRC = \
	src/main.c \
	src/ui.c \
	src/app_state.c \
	src/code_viewer.c \
	src/algorithm_trace.c \
	src/pila_view.c \
	src/cola_view.c \
	src/cola_prioridad_view.c \
	src/lista_view.c \
	src/lista_circular_view.c \
	src/sublista_view.c \
	src/grafo_view.c \
	src/pila.c \
	src/cola.c \
	src/cola_prioridad.c \
	src/lista.c \
	src/lista_circular.c \
	src/sublista.c \
	src/grafo.c \
	src/grafo_state.c \
	src/grafo_layout.c \
	src/grafo_controller.c \
	src/grafo_pedagogy.c \
	src/grafo_code_viewer.c \
	src/grafo_trace.c

TARGET = visualstruct

# Si pkg-config esta disponible, usarlo. Si no, usar fallback para Windows.
RAYLIB_CFLAGS ?= $(shell pkg-config --cflags raylib 2>NUL)
RAYLIB_LIBS ?= $(shell pkg-config --libs raylib 2>NUL)

ifeq ($(strip $(RAYLIB_LIBS)),)
	RAYLIB_LIBS = -lraylib -lopengl32 -lgdi32 -lwinmm
endif

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(RAYLIB_CFLAGS) $(SRC) -o $(TARGET) $(RAYLIB_LIBS)

clean:
	-del /Q $(TARGET).exe 2>NUL || true
	-rm -f $(TARGET)

.PHONY: all clean
