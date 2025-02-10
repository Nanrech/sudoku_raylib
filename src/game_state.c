#include "game_state.h"
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
  gameState.mistakeCount = 0;

  gameState.isAnySquareSelected = false;
  gameState.selectedRow = -1;
  gameState.selectedCol = -1;

  grid_clear();
  grid_load_random_seed();
  grid_randomly_mutate();
  grid_mark_pregenerated();
}


void game_update(void) {
  // get mouse position
  Vector2 mousePos = GetMousePosition();

  // get keyboard input
  int keyPressed = GetKeyPressed();

  // restart
  if (IsKeyReleased(KEY_R)) {
  // TraceLog(LOG_INFO, TextFormat("%d %d %d %d %d %d %d %d %d", numberBag[0], numberBag[1], numberBag[2], numberBag[3], numberBag[4], numberBag[5], numberBag[6], numberBag[7], numberBag[8]));
    gameState.isGameInit = false;
  }

  // if the game is over (either win or lose)
  // reject any grid input
  if (gameState.isGameWon || gameState.isGameLost) {
    return;
  }

  // TODO: remove
  if (IsKeyReleased(KEY_H)) {
    if (gameState.isAnySquareSelected) {
      grid[gameState.selectedRow][gameState.selectedCol].number = gridSolved[gameState.selectedRow][gameState.selectedCol];
      grid[gameState.selectedRow][gameState.selectedCol].isHint = true;
      grid[gameState.selectedRow][gameState.selectedCol].isWrong = false;

      // deselect square
      gameState.isAnySquareSelected = false;
      gameState.selectedRow = -1;
      gameState.selectedCol = -1;
    }
  }

  // TODO: remove
  if (IsKeyReleased(KEY_S)) {
    for (int row = 0; row < GRID_SIZE; row++) {
      for (int col = 0; col < GRID_SIZE; col++) {
      int temp = grid[row][col].number;
      grid[row][col].number = gridSolved[row][col];
      gridSolved[row][col] = temp;
      }
    }
  }

  // TODO: remove
  if (IsKeyReleased(KEY_T)) {
    grid_vertical_flip();
  }

  // select cell
  if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
    // set isAnySquareSelected to false so that clicking outside the grid to deselect works
    gameState.isAnySquareSelected = false;

    // calculate which square was hit (if any)
    Vector2 offset;
    offset.x = (screenWidth / 2) - (GRID_SIZE * SQUARE_SIZE / 2);
    offset.y = (screenHeight / 2) - ((GRID_SIZE - 1) * SQUARE_SIZE / 2) + (SQUARE_SIZE * 2);

    offset.y -= 50;
    int offsetControl = offset.x;

    for (int row = 0; row < GRID_SIZE; row++) {
      offset.x = offsetControl;
      for (int col = 0; col < GRID_SIZE; col++) {
        if (CheckCollisionPointRec(mousePos, (Rectangle){offset.x, offset.y, SQUARE_SIZE, SQUARE_SIZE})) {
          // clicking a cell again deselects it
          if (gameState.selectedRow == row && gameState.selectedCol == col) {
            gameState.isAnySquareSelected = false;
            gameState.selectedRow = -1;
            gameState.selectedCol = -1;
          }
          else {
            gameState.isAnySquareSelected = true;
            gameState.selectedRow = row;
            gameState.selectedCol = col;
          }
        }
        offset.x += SQUARE_SIZE;
      }
      offset.y += SQUARE_SIZE;
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
    int selectedNumber = keyPressed - KEY_ONE + 1;
    grid[gameState.selectedRow][gameState.selectedCol].number = selectedNumber;

    // check if it's wrong
    if (selectedNumber != gridSolved[gameState.selectedRow][gameState.selectedCol] || grid_is_place_valid(gameState.selectedRow, gameState.selectedCol, selectedNumber)) {
      grid[gameState.selectedRow][gameState.selectedCol].isWrong = true;
      gameState.mistakeCount += 1;
    }
    else {
      grid[gameState.selectedRow][gameState.selectedCol].isWrong = false;
    }

    // deselect square
    gameState.isAnySquareSelected = false;
    gameState.selectedRow = -1;
    gameState.selectedCol = -1;
  }

  // check lose condition
  if (gameState.mistakeCount >= MAX_MISTAKES_ALLOWED) {
    gameState.isGameLost = true;
  }

  // check win condition
  if (grid_is_solved()) {
    gameState.isGameWon = true;
  }
}

void game_draw(void) {
  BeginDrawing();
  ClearBackground(RAYWHITE);

  // Draw UI
  DrawRectangleLinesEx((Rectangle){(screenWidth / 2) - (GRID_SIZE * SQUARE_SIZE / 2), (screenHeight / 2) - SQUARE_SIZE * 4 - 50, SQUARE_SIZE * GRID_SIZE, SQUARE_SIZE * 1.5}, 2, GRAY);

  // Draw gameplay area
  Vector2 offset;
  Vector2 offsetControl;

  offset.x = (screenWidth / 2) - (GRID_SIZE * SQUARE_SIZE / 2);
  offset.y = (screenHeight / 2) - ((GRID_SIZE - 1) * SQUARE_SIZE / 2) + (SQUARE_SIZE * 2) - 50;

  offsetControl = offset;

  for (int row = 0; row < GRID_SIZE; row++) {
    offset.x = offsetControl.x;
    for (int col = 0; col < GRID_SIZE; col++) {
      // if we have selected a number
      if (gameState.isAnySquareSelected) {
        int selectedNumber = grid[gameState.selectedRow][gameState.selectedCol].number;

        // highlight selected square
        // if number == 0 OR if the current number is wrong
        if (row == gameState.selectedRow && col == gameState.selectedCol && (grid[gameState.selectedRow][gameState.selectedCol].number == 0 || grid[gameState.selectedRow][gameState.selectedCol].isWrong)) {
          DrawRectangle(offset.x, offset.y, SQUARE_SIZE, SQUARE_SIZE, COLOR_SELECTED);
        }

        // if selected square is 0, highlight relevant squares
        else if (selectedNumber == 0) {
          if (row == gameState.selectedRow || col == gameState.selectedCol) {
            DrawRectangle(offset.x, offset.y, SQUARE_SIZE, SQUARE_SIZE, COLOR_HIGHLIGHT);
          }
        }

        // highlight squares with the same (non 0) number
        else if (grid[row][col].number == selectedNumber && selectedNumber != 0) {
          DrawRectangle(offset.x, offset.y, SQUARE_SIZE, SQUARE_SIZE, COLOR_HIGHLIGHT);
        }
      }

      // text color
      Color textColor;

      if (grid[row][col].isPregenerated) {
        textColor = BLACK;
      }
      else if (grid[row][col].isWrong) {
        textColor = MAROON;
      }
      else if (grid[row][col].isHint) {
        textColor = DARKGREEN;
      }
      else {
        textColor = DARKBLUE;
      }

      // draw text, 0 means empty
      if (grid[row][col].number != 0) {
        DrawText(TextFormat("%d", grid[row][col].number), offset.x + (SQUARE_SIZE / 4), offset.y + (SQUARE_SIZE / 5), SQUARE_FONT_SIZE, textColor);
      }

      // draw surrounding square of each cell
      DrawLine(offset.x, offset.y, offset.x + SQUARE_SIZE, offset.y, GRAY);
      DrawLine(offset.x, offset.y, offset.x, offset.y + SQUARE_SIZE, GRAY);
      DrawLine(offset.x + SQUARE_SIZE, offset.y, offset.x + SQUARE_SIZE, offset.y + SQUARE_SIZE, GRAY);
      DrawLine(offset.x, offset.y + SQUARE_SIZE, offset.x + SQUARE_SIZE, offset.y + SQUARE_SIZE, GRAY);

      offset.x += SQUARE_SIZE;
    }
    offset.y += SQUARE_SIZE;
  }

  // draw separators
  DrawLineEx(
    (Vector2){offsetControl.x + (3 * SQUARE_SIZE), offsetControl.y},
    (Vector2){offsetControl.x + (3 * SQUARE_SIZE), offsetControl.y + (9 * SQUARE_SIZE)},
    3, DARKGRAY
  );
  DrawLineEx(
    (Vector2){offsetControl.x + (6 * SQUARE_SIZE), offsetControl.y},
    (Vector2){offsetControl.x + (6 * SQUARE_SIZE), offsetControl.y + (9 * SQUARE_SIZE)},
    3, DARKGRAY
  );
  DrawLineEx(
    (Vector2){offsetControl.x, offsetControl.y + (3 * SQUARE_SIZE)},
    (Vector2){offsetControl.x + (9 * SQUARE_SIZE), offsetControl.y + (3 * SQUARE_SIZE)},
    3, DARKGRAY
  );
  DrawLineEx(
    (Vector2){offsetControl.x, offsetControl.y + (6 * SQUARE_SIZE)},
    (Vector2){offsetControl.x + (9 * SQUARE_SIZE), offsetControl.y + (6 * SQUARE_SIZE)},
    3, DARKGRAY
  );

  EndDrawing();
}

void game_reset(void) {
  gameState.isGameInit = false;
}
