#include "game.h"
#include <stdlib.h>
#include <string.h>

const SizeConfig BOARD_SIZES[NUM_SIZES] = {
    [SIZE_SMALL]  = { 15, 10, 3 },
    [SIZE_MEDIUM] = { 30, 20, 4 },
    [SIZE_LARGE]  = { 45, 30, 4 },
    [SIZE_CUSTOM] = { 0, 0, 0 }, // Placeholder for custom sizes
};

static GameState *game_init(int width, int height, int ncolors) {
    GameState *gs = calloc(1, sizeof(GameState));
    if (!gs) return NULL;
    gs->width = width;
    gs->height = height;
    gs->ncolors = ncolors;
    gs->board = calloc(gs->height, sizeof(int8_t *));
    if (!gs->board) { free(gs); return NULL; }
    for (int y = 0; y < gs->height; y++) {
        gs->board[y] = calloc(gs->width, sizeof(int8_t));
        if (!gs->board[y]) { game_free(gs); return NULL; }
    }
    game_reset(gs);
    return gs;
}

GameState *game_new(GameSize size) {
    if (size == SIZE_CUSTOM) return NULL; // Use game_new_custom instead
    return game_init(BOARD_SIZES[size].width, BOARD_SIZES[size].height, BOARD_SIZES[size].ncolors);
}

GameState *game_new_custom(int width, int height, int ncolors) {
    if (width <= 0 || height <= 0 || ncolors <= 0) return NULL;
    GameState *gs = game_init(width, height, ncolors);
    if (gs) gs->size = SIZE_CUSTOM;
    return gs;
}

void game_free(GameState *gs) {
    if (!gs) return;
    if (gs->board) {
        for (int y = 0; y < gs->height; y++)
            free(gs->board[y]);
        free(gs->board);
    }
    free(gs);
}

void game_reset(GameState *gs) {
    gs->score = 0;
    for (int y = 0; y < gs->height; y++)
        for (int x = 0; x < gs->width; x++)
            gs->board[y][x] = rand() % gs->ncolors;
}

int game_get_cell(const GameState *gs, int x, int y) {
    if (x < 0 || x >= gs->width || y < 0 || y >= gs->height)
        return NONE;
    return gs->board[y][x];
}

/* Flood-fill using explicit stack to avoid recursion depth issues.
 * Returns count of connected cells; out_cells is caller-freed array of
 * packed (y * width + x) ints. */
int game_find_connected(GameState *gs, int x, int y, int **out_cells) {
    int color = game_get_cell(gs, x, y);
    if (color == NONE) { *out_cells = NULL; return 0; }

    int max = gs->width * gs->height;
    int *stack = malloc(max * sizeof(int));
    bool *visited = calloc(max, sizeof(bool));
    int *result = malloc(max * sizeof(int));
    if (!stack || !visited || !result) {
        free(stack); free(visited); free(result);
        *out_cells = NULL;
        return 0;
    }

    int sp = 0, count = 0;
    stack[sp++] = y * gs->width + x;
    visited[y * gs->width + x] = true;

    while (sp > 0) {
        int idx = stack[--sp];
        int cx = idx % gs->width;
        int cy = idx / gs->width;
        result[count++] = idx;

        const int dx[] = { 1, -1, 0, 0 };
        const int dy[] = { 0, 0, 1, -1 };
        for (int d = 0; d < 4; d++) {
            int nx = cx + dx[d], ny = cy + dy[d];
            if (nx < 0 || nx >= gs->width || ny < 0 || ny >= gs->height)
                continue;
            int ni = ny * gs->width + nx;
            if (visited[ni] || gs->board[ny][nx] != color)
                continue;
            visited[ni] = true;
            stack[sp++] = ni;
        }
    }

    free(stack);
    free(visited);
    *out_cells = result;
    return count;
}

int game_calculate_score(int nballs) {
    if (nballs < 3) return 0;
    return (nballs - 2) * (nballs - 2);
}

bool game_destroy_selected(GameState *gs, int *cells, int count) {
    if (count < 2 || !cells) return false;
    for (int i = 0; i < count; i++) {
        int x = cells[i] % gs->width;
        int y = cells[i] / gs->width;
        gs->board[y][x] = NONE;
    }
    gs->score += game_calculate_score(count);
    game_apply_gravity(gs);
    return true;
}

void game_apply_gravity(GameState *gs) {
    /* Drop balls down within each column */
    for (int x = 0; x < gs->width; x++) {
        int write_y = gs->height - 1;
        for (int y = gs->height - 1; y >= 0; y--) {
            if (gs->board[y][x] != NONE) {
                gs->board[write_y][x] = gs->board[y][x];
                if (write_y != y)
                    gs->board[y][x] = NONE;
                write_y--;
            }
        }
    }
    /* Collapse empty columns to the left */
    int write_x = 0;
    for (int x = 0; x < gs->width; x++) {
        if (gs->board[gs->height - 1][x] != NONE) {
            if (write_x != x) {
                for (int y = 0; y < gs->height; y++) {
                    gs->board[y][write_x] = gs->board[y][x];
                    gs->board[y][x] = NONE;
                }
            }
            write_x++;
        }
    }
}

bool game_is_over(const GameState *gs) {
    bool *checked = calloc(gs->width * gs->height, sizeof(bool));
    if (!checked) return true;
    bool over = true;
    for (int y = 0; y < gs->height && over; y++) {
        for (int x = 0; x < gs->width && over; x++) {
            if (gs->board[y][x] == NONE || checked[y * gs->width + x])
                continue;
            /* Quick neighbor check before full flood fill */
            int color = gs->board[y][x];
            bool has_neighbor = false;
            const int dx[] = { 1, -1, 0, 0 };
            const int dy[] = { 0, 0, 1, -1 };
            for (int d = 0; d < 4; d++) {
                int nx = x + dx[d], ny = y + dy[d];
                if (nx >= 0 && nx < gs->width && ny >= 0 && ny < gs->height
                    && gs->board[ny][nx] == color) {
                    has_neighbor = true;
                    break;
                }
            }
            if (!has_neighbor) {
                checked[y * gs->width + x] = true;
                continue;
            }
            /* Full flood fill to mark all in group */
            int *cells = NULL;
            /* We need a mutable copy for find_connected; cast away const safely
             * since find_connected only reads the board. */
            GameState tmp = *gs;
            int cnt = game_find_connected(&tmp, x, y, &cells);
            for (int i = 0; i < cnt; i++)
                checked[cells[i]] = true;
            free(cells);
            if (cnt >= 2)
                over = false;
        }
    }
    free(checked);
    return over;
}

int game_remaining_balls(const GameState *gs) {
    int count = 0;
    for (int y = 0; y < gs->height; y++)
        for (int x = 0; x < gs->width; x++)
            if (gs->board[y][x] != NONE) count++;
    return count;
}