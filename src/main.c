#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include "game.h"
#include "drawing.h"
#define DEFAULT_THEME "data/themes/planets.png"
typedef struct {
    GameState *game;
    Renderer *renderer;
    GtkWidget *score_label;
    GtkWidget *status_label;
    GtkWidget *window;
} AppContext;
static void update_ui(AppContext *app) {
    char buf[128];
    snprintf(buf, sizeof(buf), "Score: %u", app->game->score);
    gtk_label_set_text(GTK_LABEL(app->score_label), buf);
    if (game_is_over(app->game)) {
        int remaining = game_remaining_balls(app->game);
        if (remaining == 0)
            snprintf(buf, sizeof(buf), "Cleared! Final score: %u", app->game->score);
        else
            snprintf(buf, sizeof(buf), "Game Over — %d balls left", remaining);
        gtk_label_set_text(GTK_LABEL(app->status_label), buf);
    } else {
        gtk_label_set_text(GTK_LABEL(app->status_label), "");
    }
    gtk_widget_queue_draw(app->renderer->canvas);
}
static void on_draw(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer data) {
    AppContext *app = data;
    renderer_draw_cairo(app->renderer, cr, app->game);
}
static void on_click(GtkGestureClick *gesture, int n_press, double x, double y, gpointer data) {
    AppContext *app = data;
    int lx, ly;
    renderer_pixels_to_logical(app->renderer, x, y, &lx, &ly);
    int *cells = NULL;
    int count = game_find_connected(app->game, lx, ly, &cells);
    if (count >= 2) {
        game_destroy_selected(app->game, cells, count);
        free(cells);
        renderer_set_highlight(app->renderer, NULL, 0);
        update_ui(app);
    } else {
        free(cells);
    }
}
static void on_motion(GtkEventControllerMotion *motion, double x, double y, gpointer data) {
    AppContext *app = data;
    int lx, ly;
    renderer_pixels_to_logical(app->renderer, x, y, &lx, &ly);
    int *cells = NULL;
    int count = game_find_connected(app->game, lx, ly, &cells);
    if (count >= 2) {
        renderer_set_highlight(app->renderer, cells, count);
    } else {
        renderer_set_highlight(app->renderer, NULL, 0);
        free(cells);
    }
    gtk_widget_queue_draw(app->renderer->canvas);
}
static gboolean on_timer(gpointer data) {
    AppContext *app = data;
    if (app->renderer) {
        app->renderer->rotation_phase += 0.1f;
        if (app->renderer->highlight_count > 0) {
            gtk_widget_queue_draw(app->renderer->canvas);
        }
    }
    return G_SOURCE_CONTINUE;
}
static void set_game_size(AppContext *app, int w, int h, int c) {
    game_free(app->game);
    app->game = game_new_custom(w, h, c);
    if (!app->game) {
        app->game = game_new(SIZE_SMALL);
    }

    // Calculate dynamic tile size to fit screen
    GdkDisplay *display = gdk_display_get_default();
    GdkMonitor *monitor = gdk_display_get_monitor_at_surface(display,
        gtk_native_get_surface(gtk_widget_get_native(app->window)));
    GdkRectangle geometry;
    gdk_monitor_get_geometry(monitor, &geometry);

    int max_w = (int)(geometry.width * 0.8);   // 80% of screen width
    int max_h = (int)(geometry.height * 0.6);  // 60% of screen height (room for UI)

    int tile_w = max_w / app->game->width;
    int tile_h = max_h / app->game->height;
    int new_tile_size = MIN(tile_w, tile_h);
    if (new_tile_size < 8) new_tile_size = 8;  // Minimum readable size

    app->renderer->tile_size = new_tile_size;

    renderer_set_highlight(app->renderer, NULL, 0);
    int win_w = app->game->width * new_tile_size + 20;
    int win_h = app->game->height * new_tile_size + 80;
    gtk_window_set_default_size(GTK_WINDOW(app->window), win_w, win_h);
    GtkWidget *canvas = app->renderer->canvas;
    gtk_widget_set_size_request(canvas, app->game->width * new_tile_size, app->game->height * new_tile_size);
    update_ui(app);
}
static void on_new_game(GSimpleAction *action, GVariant *parameter, gpointer data) {
    AppContext *app = data;
    game_reset(app->game);
    renderer_set_highlight(app->renderer, NULL, 0);
    update_ui(app);
}
static void on_set_size_small(GSimpleAction *action, GVariant *parameter, gpointer data) {
    AppContext *app = data;
    set_game_size(app, BOARD_SIZES[SIZE_SMALL].width, BOARD_SIZES[SIZE_SMALL].height, BOARD_SIZES[SIZE_SMALL].ncolors);
}
static void on_set_size_medium(GSimpleAction *action, GVariant *parameter, gpointer data) {
    AppContext *app = data;
    set_game_size(app, BOARD_SIZES[SIZE_MEDIUM].width, BOARD_SIZES[SIZE_MEDIUM].height, BOARD_SIZES[SIZE_MEDIUM].ncolors);
}
static void on_set_size_large(GSimpleAction *action, GVariant *parameter, gpointer data) {
    AppContext *app = data;
    set_game_size(app, BOARD_SIZES[SIZE_LARGE].width, BOARD_SIZES[SIZE_LARGE].height, BOARD_SIZES[SIZE_LARGE].ncolors);
}
typedef struct {
    AppContext *app;
    GtkWidget *w_spin;
    GtkWidget *h_spin;
    GtkWidget *c_spin;
    GtkWidget *dialog;
} CustomSizeData;
static void on_custom_size_ok(GtkButton *button, CustomSizeData *csd) {
    int w = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(csd->w_spin));
    int h = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(csd->h_spin));
    int c = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(csd->c_spin));
    set_game_size(csd->app, w, h, c);
    gtk_window_destroy(GTK_WINDOW(csd->dialog));
    free(csd);
}
static void on_custom_size_cancel(GtkButton *button, CustomSizeData *csd) {
    gtk_window_destroy(GTK_WINDOW(csd->dialog));
    free(csd);
}
static void on_custom_size(GSimpleAction *action, GVariant *parameter, gpointer data) {
    AppContext *app = data;
    GtkWidget *dialog = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Custom Size");
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(app->window));
    
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(box, 20);
    gtk_widget_set_margin_end(box, 20);
    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_bottom(box, 20);
    
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    
    GtkWidget *w_label = gtk_label_new("Width (1-256):");
    GtkWidget *h_label = gtk_label_new("Height (1-256):");
    GtkWidget *c_label = gtk_label_new("Colors (2-8):");
    
    GtkWidget *w_spin = gtk_spin_button_new_with_range(1, 256, 1);
    GtkWidget *h_spin = gtk_spin_button_new_with_range(1, 256, 1);
    GtkWidget *c_spin = gtk_spin_button_new_with_range(2, 8, 1);
    
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w_spin), app->game->width);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(h_spin), app->game->height);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(c_spin), app->game->ncolors);
    
    gtk_grid_attach(GTK_GRID(grid), w_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), w_spin, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), h_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), h_spin, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), c_label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), c_spin, 1, 2, 1, 1);
    
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *cancel_btn = gtk_button_new_with_label("Cancel");
    GtkWidget *ok_btn = gtk_button_new_with_label("OK");
    gtk_box_append(GTK_BOX(btn_box), cancel_btn);
    gtk_box_append(GTK_BOX(btn_box), ok_btn);
    
    gtk_box_append(GTK_BOX(box), grid);
    gtk_box_append(GTK_BOX(box), btn_box);
    
    gtk_window_set_child(GTK_WINDOW(dialog), box);
    
    CustomSizeData *csd = malloc(sizeof(CustomSizeData));
    csd->app = app;
    csd->w_spin = w_spin;
    csd->h_spin = h_spin;
    csd->c_spin = c_spin;
    csd->dialog = dialog;
    
    g_signal_connect(ok_btn, "clicked", G_CALLBACK(on_custom_size_ok), csd);
    g_signal_connect(cancel_btn, "clicked", G_CALLBACK(on_custom_size_cancel), csd);
    
    gtk_window_present(GTK_WINDOW(dialog));
}
static const GActionEntry app_actions[] = {
    { "new_game", on_new_game, NULL, NULL, NULL, {0,0,0} },
    { "size_small", on_set_size_small, NULL, NULL, NULL, {0,0,0} },
    { "size_medium", on_set_size_medium, NULL, NULL, NULL, {0,0,0} },
    { "size_large", on_set_size_large, NULL, NULL, NULL, {0,0,0} },
    { "size_custom", on_custom_size, NULL, NULL, NULL, {0,0,0} },
};
static void activate(GtkApplication *gtk_app, gpointer data) {
    AppContext *app = data;
    GtkWidget *window = gtk_application_window_new(gtk_app);
    app->window = window;
    gtk_window_set_title(GTK_WINDOW(window), "Same GNOME");
    gtk_window_set_default_size(GTK_WINDOW(window),
                                app->game->width * TILE_SIZE + 20,
                                app->game->height * TILE_SIZE + 80);
    // Menu bar
    GMenu *menu = g_menu_new();
    g_menu_append(menu, "New Game", "app.new_game");
    
    GMenu *size_menu = g_menu_new();
    g_menu_append(size_menu, "Small", "app.size_small");
    g_menu_append(size_menu, "Medium", "app.size_medium");
    g_menu_append(size_menu, "Large", "app.size_large");
    g_menu_append(size_menu, "Custom...", "app.size_custom");
    
    GMenuItem *size_item = g_menu_item_new_submenu("Size", G_MENU_MODEL(size_menu));
    g_menu_append_item(menu, size_item);
    g_object_unref(size_item);
    g_object_unref(size_menu);
    
    GtkWidget *header = gtk_header_bar_new();
    gtk_header_bar_set_show_title_buttons(GTK_HEADER_BAR(header), TRUE);
    GtkWidget *menu_button = gtk_menu_button_new();
    gtk_menu_button_set_icon_name(GTK_MENU_BUTTON(menu_button), "open-menu-symbolic");
    gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(menu_button), G_MENU_MODEL(menu));
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), menu_button);
    gtk_window_set_titlebar(GTK_WINDOW(window), header);
    g_object_unref(menu);
    
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_margin_top(vbox, 8);
    gtk_widget_set_margin_bottom(vbox, 8);
    gtk_widget_set_margin_start(vbox, 8);
    gtk_widget_set_margin_end(vbox, 8);
    gtk_window_set_child(GTK_WINDOW(window), vbox);
    
    app->score_label = gtk_label_new("Score: 0");
    gtk_widget_set_halign(app->score_label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), app->score_label);
    
    app->status_label = gtk_label_new("");
    gtk_widget_set_halign(app->status_label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), app->status_label);
    
    GtkWidget *canvas = gtk_drawing_area_new();
    gtk_widget_set_size_request(canvas,
                                app->game->width * TILE_SIZE,
                                app->game->height * TILE_SIZE);
    gtk_box_append(GTK_BOX(vbox), canvas);
    
    app->renderer = renderer_new(canvas, DEFAULT_THEME);
    app->renderer->canvas = canvas;
    
    GtkGesture *click = gtk_gesture_click_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click), GDK_BUTTON_PRIMARY);
    g_signal_connect(click, "pressed", G_CALLBACK(on_click), app);
    gtk_widget_add_controller(canvas, GTK_EVENT_CONTROLLER(click));
    
    GtkEventController *motion = gtk_event_controller_motion_new();
    g_signal_connect(motion, "motion", G_CALLBACK(on_motion), app);
    gtk_widget_add_controller(canvas, motion);
    
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(canvas), on_draw, app, NULL);
    
    g_timeout_add(50, on_timer, app);
    
    update_ui(app);
    gtk_window_present(GTK_WINDOW(window));
}
int main(int argc, char **argv) {
    srand(time(NULL));
    AppContext app = {0};
    app.game = game_new(SIZE_SMALL);
    if (!app.game) {
        fprintf(stderr, "Failed to initialize game\n");
        return 1;
    }
    GtkApplication *gtk_app = gtk_application_new("org.gnome.SameGnome", G_APPLICATION_DEFAULT_FLAGS);
    g_action_map_add_action_entries(G_ACTION_MAP(gtk_app), app_actions, G_N_ELEMENTS(app_actions), &app);
    g_signal_connect(gtk_app, "activate", G_CALLBACK(activate), &app);
    int status = g_application_run(G_APPLICATION(gtk_app), argc, argv);
    g_object_unref(gtk_app);
    renderer_free(app.renderer);
    game_free(app.game);
    return status;
}