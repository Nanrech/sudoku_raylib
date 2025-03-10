#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "common.h"

typedef struct GridSquare {
  bool isPregenerated;
  bool isHint;
  bool isWrong;
  int number;
} GridSquare_t;

typedef struct {
  // Game state
  bool isGameInit;
  bool isGameWon;
  bool isGameLost;
  bool isHintUsed;
  int mistakesLeft;

  // Input state
  bool isAnySquareSelected;
  int selectedRow;
  int selectedCol;
} GameState_t;

// Global grid(s)
extern GridSquare_t grid[GRID_SIZE][GRID_SIZE];
extern int gridSolved[GRID_SIZE][GRID_SIZE];

// Global game state
extern GameState_t gameState;

void game_start(void);
void game_reset(void);
void game_input(void);
void game_square_select(int row, int col);
void game_square_deselect(void);
void game_draw(void);

#endif // GAME_STATE_H
