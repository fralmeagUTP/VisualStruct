# Guia de Contribucion

## Objetivo
Esta guia resume como extender o modificar el proyecto sin romper la separacion entre dominio, aplicacion y presentacion.

## Reglas base
- Mantener un unico punto de entrada en `src/main.c`.
- Evitar acceso directo a nodos desde la UI o desde las vistas.
- No incluir `raylib.h` en los TAD.
- Toda operacion visible al usuario debe pasar por `app_state`.
- Toda mejora docente que dependa de la operacion actual debe reflejarse tambien en `code_viewer` o `algorithm_trace` si aplica.

## Cuando cambies un TAD
1. Actualizar su encabezado en `include/`.
2. Mantener la API publica simple y coherente.
3. Preservar independencia respecto a Raylib.
4. Si hace falta visualizacion nueva, exponer funciones de copia o snapshot antes que acceso estructural directo.
5. Verificar que compile con `-std=c11 -Wall -Wextra -pedantic`.

## Cuando agregues una operacion nueva
1. Extender enums en `include/app_state.h`.
2. Implementar el comportamiento en `src/app_state.c`.
3. Actualizar botones o atajos en `src/main.c` si la operacion es interactiva.
4. Anadir snippet asociado en `src/code_viewer.c`.
5. Anadir traza y complejidad en `src/algorithm_trace.c`.
6. Ajustar la vista correspondiente si necesita nuevo feedback visual.
7. Revisar si la ayuda interna (`SCREEN_HELP` en `src/main.c`) requiere actualizacion del texto.

## Cuando agregues una estructura nueva
1. Crear TAD y header publico.
2. Registrar la nueva estructura en `AppState`.
3. Crear una vista dedicada `src/<estructura>_view.c` y su header en `include/`.
4. Integrar seleccion, controles y render en `src/main.c`.
5. Documentar arquitectura y guia docente si la estructura cambia el alcance del proyecto.

## Cuando toques la UI
- Reutilizar helpers en `src/ui.c` antes de introducir widgets duplicados.
- Mantener mensajes breves y pedagogicos.
- Favorecer claridad visual sobre decoracion excesiva.
- Si agregas feedback visual, validar tambien los estados de error y vacio.
- Si cambias navegacion o atajos, sincronizar `README.md`, `docs/qa-manual.md` y la ayuda interna.

## Documentacion minima esperada
- `@file` y `@brief` en modulos nuevos.
- Actualizacion de `README.md` si cambia el flujo de uso o compilacion.
- Actualizacion de `docs/architecture.md` si cambia una responsabilidad arquitectonica.
- Actualizacion de `docs/guia-docente.md` si cambia la estrategia de demostracion en clase.

## Validacion recomendada antes de cerrar un cambio
1. Compilar la app unificada.
2. Revisar errores estaticos de los archivos tocados.
3. Regenerar Doxygen si cambiaste comentarios o Markdown en `docs/`.
4. Confirmar que `src/legacy` no vuelva a entrar accidentalmente al build principal.
