#pragma once
#include <gtk/gtk.h>
#include "game.h"
#define TILE_SIZE 32
typedef struct {
 GtkWidget *canvas;
 GdkTexture *theme_texture;
 int tile_size;
 int cursor_x;
 int cursor_y;
 bool show_cursor;
 int *highlight_cells;
 int highlight_count;
 float rotation_phase;
} Renderer;
Renderer *renderer_new(GtkWidget *canvas, const char *theme_path);
void renderer_free(Renderer *r);
void renderer_set_highlight(Renderer *r, int *cells, int count);
void renderer_set_cursor(Renderer *r, int x, int y, bool visible);
void renderer_draw_cairo(Renderer *r, cairo_t *cr, GameState *gs);
void renderer_pixels_to_logical(Renderer *r, double px, double py, int *lx, int *ly);