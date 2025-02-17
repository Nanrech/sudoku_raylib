#include "game.h"
#include "grid.h"


// Global variables
GridSquare_t grid[GRID_SIZE][GRID_SIZE];
int gridSolved[GRID_SIZE][GRID_SIZE] = {0};

// Global state
GameState_t gameState;

void game_start(void) {
  gameState.isGameInit = true;
  gameState.isGameWon = false;
  gameState.isGameLost = false;
  gameState.isHintUsed = false;

  gameState.mistakesLeft = MAX_MISTAKES_ALLOWED;

  game_square_deselect();

  grid_clear();
  grid_load_random_seed();
  grid_randomly_mutate();
  grid_mark_pregenerated();
}

void game_reset(void) {
  gameState.isGameInit = false;
}

void game_input(void) {
  // get mouse position
  Vector2 mousePos = GetMousePosition();

  // get keyboard input
  int keyPressed = GetKeyPressed();

  // restart
  if (IsKeyReleased(KEY_R)) {
    game_reset();
    return;
  }

  // if the game is over (either win or lose) reject any grid input
  if (gameState.isGameWon || gameState.isGameLost) {
    return;
  }

  // Hint
  if (IsKeyReleased(KEY_H)) {
    if (gameState.isAnySquareSelected && !gameState.isHintUsed) {
      grid[gameState.selectedRow][gameState.selectedCol].number = gridSolved[gameState.selectedRow][gameState.selectedCol];
      grid[gameState.selectedRow][gameState.selectedCol].isHint = true;
      grid[gameState.selectedRow][gameState.selectedCol].isWrong = false;

      gameState.isHintUsed = true;

      game_square_deselect();
    }
  }

  // Autosolve
  if (IsKeyReleased(KEY_S)) {
    for (int row = 0; row < GRID_SIZE; row++) {
      for (int col = 0; col < GRID_SIZE; col++) {
        grid[row][col].number = gridSolved[row][col];
      }
    }
  }

  // select cell
  if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
    // calculate which square was hit (if any)
    Vector2 gridOffset;
    gridOffset.x = (screenWidth / 2) - (GRID_SIZE * SQUARE_SIZE / 2);
    gridOffset.y = (screenHeight / 2) - ((GRID_SIZE - 1) * SQUARE_SIZE / 2) + (SQUARE_SIZE * 2);

    gridOffset.y -= 50;
    int gridOffsetControl = gridOffset.x;

    for (int row = 0; row < GRID_SIZE; row++) {
      gridOffset.x = gridOffsetControl;
      for (int col = 0; col < GRID_SIZE; col++) {
        if (CheckCollisionPointRec(mousePos, (Rectangle){gridOffset.x, gridOffset.y, SQUARE_SIZE, SQUARE_SIZE})) {
          // clicking a cell again deselects it
          if (gameState.selectedRow == row && gameState.selectedCol == col) {
            game_square_deselect();
          }
          else {
            game_square_select(row, col);
          }
        }
        gridOffset.x += SQUARE_SIZE;
      }
      gridOffset.y += SQUARE_SIZE;
    }
  }

  // insert number
  if (keyPressed >= KEY_ONE && keyPressed <= KEY_NINE) {
    if (!gameState.isAnySquareSelected) {
      return;
    }
    // if number is NOT 0 and NOT wrong, return
    if (grid[gameState.selectedRow][gameState.selectedCol].number != 0 && !grid[gameState.selectedRow][gameState.selectedCol].isWrong) {
      return;
    }

    // set number
    int selectedNumber = keyPressed - KEY_ZERO;
    grid[gameState.selectedRow][gameState.selectedCol].number = selectedNumber;

    // check if it's wrong
    if (selectedNumber != gridSolved[gameState.selectedRow][gameState.selectedCol]) {
      grid[gameState.selectedRow][gameState.selectedCol].isWrong = true;
      gameState.mistakesLeft -= 1;
    }
    else {
      grid[gameState.selectedRow][gameState.selectedCol].isWrong = false;
    }

    // deselect square
    game_square_deselect();
  }

  // check lose condition
  if (gameState.mistakesLeft == 0) {
    gameState.isGameLost = true;
  }

  // check win condition
  if (grid_is_solved()) {
    gameState.isGameWon = true;
  }
}

void game_square_select(int row, int col) {
  gameState.isAnySquareSelected = true;
  gameState.selectedRow = row;
  gameState.selectedCol = col;
}

void game_square_deselect(void) {
  gameState.isAnySquareSelected = false;
  gameState.selectedRow = -1;
  gameState.selectedCol = -1;
}

// this one is way too big
void game_draw(void) {
  BeginDrawing();
  ClearBackground(RAYWHITE);

  // Draw error counter
  Vector2 errorCounterOffset;
  errorCounterOffset.x = (screenWidth / 2) - (GRID_SIZE / 2 * SQUARE_SIZE) - SQUARE_SIZE / 2;//; - (GRID_SIZE / 2 * SQUARE_SIZE);
  errorCounterOffset.y = (screenHeight / 2) - (GRID_SIZE / 2 * SQUARE_SIZE);

  if (!gameState.isGameWon) {
    if (gameState.mistakesLeft == 1) {
      DrawText("x", errorCounterOffset.x, errorCounterOffset.y, SQUARE_FONT_SIZE, RED);
    }
    else if (gameState.mistakesLeft == 2) {
      DrawText("xx", errorCounterOffset.x, errorCounterOffset.y, SQUARE_FONT_SIZE, RED);
    }
    else if (gameState.mistakesLeft == 3) {
      DrawText("xxx", errorCounterOffset.x, errorCounterOffset.y, SQUARE_FONT_SIZE, RED);
    }
  }

  // Draw win/lose text
  if (gameState.isGameLost) {
    DrawText("YOU LOSE\t:(", errorCounterOffset.x, errorCounterOffset.y, SQUARE_FONT_SIZE, RED);
  }
  else if (gameState.isGameWon) {
    DrawText("YOU WIN\t:)", errorCounterOffset.x, errorCounterOffset.y, SQUARE_FONT_SIZE, GREEN);
  }

  // Draw info text
  Vector2 infoOffset;
  infoOffset.x = (screenWidth / 2) - (GRID_SIZE / 2 * SQUARE_SIZE) - SQUARE_SIZE / 2;
  infoOffset.y = (screenHeight / 2) + 50 + SQUARE_FONT_SIZE + (GRID_SIZE / 2 * SQUARE_SIZE);

  DrawText("[R] to restart\n[H] to get a hint", infoOffset.x, infoOffset.y, SQUARE_FONT_SIZE, BLACK);

  // Draw gameplay area
  Vector2 gridOffset;
  Vector2 gridOffsetControl;

  gridOffset.x = (screenWidth / 2) - (GRID_SIZE * SQUARE_SIZE / 2);
  gridOffset.y = (screenHeight / 2) - ((GRID_SIZE - 1) * SQUARE_SIZE / 2) + (SQUARE_SIZE * 2) - 50;

  gridOffsetControl = gridOffset;

  for (int row = 0; row < GRID_SIZE; row++) {
    gridOffset.x = gridOffsetControl.x;
    for (int col = 0; col < GRID_SIZE; col++) {
      // if we have selected a number
      if (gameState.isAnySquareSelected) {
        int selectedNumber = grid[gameState.selectedRow][gameState.selectedCol].number;

        // highlight selected square
        // if number == 0 OR if the current number is wrong
        if (row == gameState.selectedRow && col == gameState.selectedCol && (grid[gameState.selectedRow][gameState.selectedCol].number == 0 || grid[gameState.selectedRow][gameState.selectedCol].isWrong)) {
          DrawRectangle(gridOffset.x, gridOffset.y, SQUARE_SIZE, SQUARE_SIZE, COLOR_SELECTED);
        }

        // if selected square is 0, highlight relevant squares
        else if (selectedNumber == 0) {
          if (row == gameState.selectedRow || col == gameState.selectedCol) {
            DrawRectangle(gridOffset.x, gridOffset.y, SQUARE_SIZE, SQUARE_SIZE, COLOR_HIGHLIGHT);
          }
        }

        // highlight squares with the same (non 0) number
        else if (grid[row][col].number == selectedNumber && selectedNumber != 0) {
          DrawRectangle(gridOffset.x, gridOffset.y, SQUARE_SIZE, SQUARE_SIZE, COLOR_MATCHING);
        }
      }

      // text color
      Color textColor;

      if (grid[row][col].isPregenerated) {
        textColor = BLACK;
      }
      else if (grid[row][col].isWrong) {
        textColor = RED;
      }
      else if (grid[row][col].isHint) {
        textColor = DARKGREEN;
      }
      else {
        textColor = DARKBLUE;
      }

      // draw text, 0 means empty
      if (grid[row][col].number != 0) {
        DrawText(
          TextFormat("%d", grid[row][col].number),
          gridOffset.x + (SQUARE_SIZE / 4), gridOffset.y + (SQUARE_SIZE / 5),
          SQUARE_FONT_SIZE,
          textColor
        );
      }

      // draw surrounding square of each cell
      DrawLine(gridOffset.x, gridOffset.y, gridOffset.x + SQUARE_SIZE, gridOffset.y, GRAY);
      DrawLine(gridOffset.x, gridOffset.y, gridOffset.x, gridOffset.y + SQUARE_SIZE, GRAY);
      DrawLine(gridOffset.x + SQUARE_SIZE, gridOffset.y, gridOffset.x + SQUARE_SIZE, gridOffset.y + SQUARE_SIZE, GRAY);
      DrawLine(gridOffset.x, gridOffset.y + SQUARE_SIZE, gridOffset.x + SQUARE_SIZE, gridOffset.y + SQUARE_SIZE, GRAY);

      gridOffset.x += SQUARE_SIZE;
    }
    gridOffset.y += SQUARE_SIZE;
  }

  // draw separators
  DrawLineEx(
    (Vector2){gridOffsetControl.x + (3 * SQUARE_SIZE), gridOffsetControl.y},
    (Vector2){gridOffsetControl.x + (3 * SQUARE_SIZE), gridOffsetControl.y + (9 * SQUARE_SIZE)},
    3, DARKGRAY
  );
  DrawLineEx(
    (Vector2){gridOffsetControl.x + (6 * SQUARE_SIZE), gridOffsetControl.y},
    (Vector2){gridOffsetControl.x + (6 * SQUARE_SIZE), gridOffsetControl.y + (9 * SQUARE_SIZE)},
    3, DARKGRAY
  );
  DrawLineEx(
    (Vector2){gridOffsetControl.x, gridOffsetControl.y + (3 * SQUARE_SIZE)},
    (Vector2){gridOffsetControl.x + (9 * SQUARE_SIZE), gridOffsetControl.y + (3 * SQUARE_SIZE)},
    3, DARKGRAY
  );
  DrawLineEx(
    (Vector2){gridOffsetControl.x, gridOffsetControl.y + (6 * SQUARE_SIZE)},
    (Vector2){gridOffsetControl.x + (9 * SQUARE_SIZE), gridOffsetControl.y + (6 * SQUARE_SIZE)},
    3, DARKGRAY
  );

  EndDrawing();
}
