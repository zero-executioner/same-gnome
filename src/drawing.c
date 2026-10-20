#include "drawing.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
Renderer *renderer_new(GtkWidget *canvas, const char *theme_path) {
    Renderer *r = calloc(1, sizeof(Renderer));
    if (!r) return NULL;
    r->canvas = canvas;
    r->tile_size = TILE_SIZE;
    r->cursor_x = -1;
    r->cursor_y = -1;
    r->show_cursor = false;
    r->highlight_cells = NULL;
    r->highlight_count = 0;
    r->rotation_phase = 0.0f;
    GFile *file = g_file_new_for_path(theme_path);
    GError *err = NULL;
    r->theme_texture = gdk_texture_new_from_file(file, &err);
    if (err) {
        g_warning("Failed to load theme %s: %s", theme_path, err->message);
        g_error_free(err);
        r->theme_texture = NULL;
    }
    g_object_unref(file);
    return r;
}
void renderer_free(Renderer *r) {
    if (!r) return;
    if (r->theme_texture) g_object_unref(r->theme_texture);
    free(r->highlight_cells);
    free(r);
}
void renderer_set_highlight(Renderer *r, int *cells, int count) {
    free(r->highlight_cells);
    r->highlight_cells = cells;
    r->highlight_count = count;
}
void renderer_set_cursor(Renderer *r, int x, int y, bool visible) {
    r->cursor_x = x;
    r->cursor_y = y;
    r->show_cursor = visible;
}
void renderer_pixels_to_logical(Renderer *r, double px, double py, int *lx, int *ly) {
    int cols = gtk_widget_get_width(r->canvas) / r->tile_size;
    int rows = gtk_widget_get_height(r->canvas) / r->tile_size;
    *lx = CLAMP((int)(px / r->tile_size), 0, cols - 1);
    *ly = CLAMP((int)(py / r->tile_size), 0, rows - 1);
}
static void draw_fallback_ball(cairo_t *cr, int x, int y, int ts, int color, float rotation) {
    static const double colors[][3] = {
        {0.91, 0.30, 0.24}, // Red
        {0.18, 0.80, 0.44}, // Green
        {0.20, 0.60, 0.86}, // Blue
        {0.95, 0.61, 0.07}, // Yellow
    };
    const double *c = colors[color % 4];
    double cx = x + ts / 2.0;
    double cy = y + ts / 2.0;
    double radius = ts / 2.0 - 2;

    /* Base sphere with 3D shading — light from upper-left */
    cairo_pattern_t *base = cairo_pattern_create_radial(
        cx - radius * 0.35, cy - radius * 0.35, radius * 0.05,
        cx, cy, radius);
    cairo_pattern_add_color_stop_rgb(base, 0.0,
        fmin(c[0] * 1.4, 1.0), fmin(c[1] * 1.4, 1.0), fmin(c[2] * 1.4, 1.0));
    cairo_pattern_add_color_stop_rgb(base, 0.5, c[0], c[1], c[2]);
    cairo_pattern_add_color_stop_rgb(base, 1.0,
        c[0] * 0.25, c[1] * 0.25, c[2] * 0.25);
    cairo_set_source(cr, base);
    cairo_arc(cr, cx, cy, radius, 0, 2 * M_PI);
    cairo_fill(cr);
    cairo_pattern_destroy(base);

    /* Horizontal rotation band — moves continuously rightward */
    if (rotation != 0.0f) {
        double offset = fmod(rotation, 2.0 * M_PI) / (2.0 * M_PI) * (radius * 2.0);
        double band_width = radius * 0.35;

        cairo_save(cr);
        cairo_arc(cr, cx, cy, radius, 0, 2 * M_PI);
        cairo_clip(cr);

        cairo_pattern_t *band = cairo_pattern_create_linear(
            cx + offset - band_width, cy,
            cx + offset + band_width, cy);
        cairo_pattern_add_color_stop_rgba(band, 0.0, 1, 1, 1, 0.0);
        cairo_pattern_add_color_stop_rgba(band, 0.3, 1, 1, 1, 0.18);
        cairo_pattern_add_color_stop_rgba(band, 0.5, 1, 1, 1, 0.25);
        cairo_pattern_add_color_stop_rgba(band, 0.7, 1, 1, 1, 0.18);
        cairo_pattern_add_color_stop_rgba(band, 1.0, 1, 1, 1, 0.0);
        cairo_set_source(cr, band);
        cairo_rectangle(cr, cx - radius, cy - radius, radius * 2, radius * 2);
        cairo_fill(cr);
        cairo_pattern_destroy(band);
        cairo_restore(cr);
    }

    /* Specular highlight that shifts continuously rightward for 3D spin effect */
    double spec_x = cx - radius * 0.3 + fmod(rotation, 2.0 * M_PI) / (2.0 * M_PI) * (radius * 1.5);
    double spec_y = cy - radius * 0.35;
    double spec_r = radius * 0.22;
    cairo_pattern_t *spec = cairo_pattern_create_radial(
        spec_x, spec_y, 0, spec_x, spec_y, spec_r);
    cairo_pattern_add_color_stop_rgba(spec, 0.0, 1, 1, 1, 0.7);
    cairo_pattern_add_color_stop_rgba(spec, 1.0, 1, 1, 1, 0.0);
    cairo_set_source(cr, spec);
    cairo_arc(cr, cx, cy, radius, 0, 2 * M_PI);
    cairo_fill(cr);
    cairo_pattern_destroy(spec);
}
void renderer_draw_cairo(Renderer *r, cairo_t *cr, GameState *gs) {
    int ts = r->tile_size;
    int w = gs->width * ts;
    int h = gs->height * ts;
    /* Background */
    cairo_set_source_rgb(cr, 0.063, 0.063, 0.063);
    cairo_rectangle(cr, 0, 0, w, h);
    cairo_fill(cr);
    /* Grid lines */
    cairo_set_source_rgb(cr, 0.314, 0.314, 0.314);
    cairo_set_line_width(cr, 1.0);
    for (int x = 0; x <= gs->width; x++) {
        cairo_move_to(cr, x * ts + 0.5, 0);
        cairo_line_to(cr, x * ts + 0.5, h);
    }
    for (int y = 0; y <= gs->height; y++) {
        cairo_move_to(cr, 0, y * ts + 0.5);
        cairo_line_to(cr, w, y * ts + 0.5);
    }
    cairo_stroke(cr);
    /* Balls */
    for (int y = 0; y < gs->height; y++) {
        for (int x = 0; x < gs->width; x++) {
            int color = gs->board[y][x];
            if (color == NONE) continue;
            
            // Check if this cell is highlighted
            bool is_highlighted = false;
            float rot = 0.0f;
            if (r->highlight_count > 0 && r->highlight_cells) {
                int idx = y * gs->width + x;
                for (int i = 0; i < r->highlight_count; i++) {
                    if (r->highlight_cells[i] == idx) {
                        is_highlighted = true;
                        // Calculate rotation based on phase and position for a wave effect
                        rot = r->rotation_phase + (x + y) * 0.1f;
                        break;
                    }
                }
            }
            
            if (is_highlighted) {
                draw_fallback_ball(cr, x * ts, y * ts, ts, color, rot);
            } else {
                draw_fallback_ball(cr, x * ts, y * ts, ts, color, 0.0f);
            }
        }
    }
    /* Keyboard cursor */
    if (r->show_cursor && r->cursor_x >= 0 && r->cursor_y >= 0) {
        cairo_set_source_rgb(cr, 1.0, 1.0, 0.72);
        cairo_set_line_width(cr, 2.0);
        cairo_rectangle(cr, r->cursor_x * ts + 1, r->cursor_y * ts + 1, ts - 2, ts - 2);
        cairo_stroke(cr);
    }
}