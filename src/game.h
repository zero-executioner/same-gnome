#pragma once
#include <stdint.h>
#include <stdbool.h>

#define NONE -1
#define MAX_COLOURS 4

typedef enum {
    SIZE_SMALL = 0,
    SIZE_MEDIUM,
    SIZE_LARGE,
    SIZE_CUSTOM,
    NUM_SIZES
} GameSize;

typedef struct {
    int width;
    int height;
    int ncolors;
} SizeConfig;

extern const SizeConfig BOARD_SIZES[NUM_SIZES];

typedef struct {
    int8_t **board;
    int width;
    int height;
    int ncolors;
    uint32_t score;
    GameSize size;
} GameState;

GameState *game_new(GameSize size);
GameState *game_new_custom(int width, int height, int ncolors);
void       game_free(GameState *gs);
void       game_reset(GameState *gs);
int        game_get_cell(const GameState *gs, int x, int y);
int        game_find_connected(GameState *gs, int x, int y, int **out_cells);
int        game_calculate_score(int nballs);
bool       game_destroy_selected(GameState *gs, int *cells, int count);
bool       game_is_over(const GameState *gs);
int        game_remaining_balls(const GameState *gs);
void       game_apply_gravity(GameState *gs);