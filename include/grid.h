#ifndef GRID_H
#define GRID_H

#include "common.h"
#include "game_state.h"

extern int numberBag[9];
void number_bag_shuffle(void);

void grid_clear(void);
bool grid_is_solved(void);
bool grid_is_place_valid(int row, int col, int n);
void grid_load_random_seed(void);
void grid_randomly_mutate(void);
void grid_mark_pregenerated(void);
void grid_rotate_90(void);
void grid_horizontal_flip(void);
void grid_vertical_flip(void);

#endif // GRID_H
