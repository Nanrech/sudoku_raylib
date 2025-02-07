#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "constants.h"

typedef struct GridSquare {
  bool isPregenerated;
  bool isHint;
  bool isWrong;
  int number;
} GridSquare_t;

typedef struct {
  bool isGameInit;
  bool isGameWon;
  bool isGameLost;

  int mistakeCount;

  bool isAnySquareSelected;
  int selectedRow;
  int selectedCol;

  GridSquare_t grid[GRID_SIZE][GRID_SIZE];
  int gridSolved[GRID_SIZE][GRID_SIZE];
} GameState_t;

// Declare a global game state
extern GameState_t gameState;

void game_start(void);
void game_update(void);
void game_draw(void);
void game_reset(void);

#endif // GAME_STATE_H
