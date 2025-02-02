#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <stdlib.h>

#define GRID_SIZE 9
#define SEED_LEN 81
#define SEED_AMOUNT 5

#define SQUARE_SIZE 40
#define SQUARE_FONT_SIZE (SQUARE_SIZE - (SQUARE_SIZE / 4))


// typedefs
typedef struct GridSquare {
  bool isPregenerated;
  bool isHint;
  bool isWrong;
  int number;
} GridSquare_t;

// global variables
const int screenWidth = 1600;
const int screenHeight = 900;

static bool isGameInit = false;

static int mistakeCount = 0;

static bool isSquareSelected = false;
static int selectedRow = 0;
static int selectedCol = 0;

static GridSquare_t grid[GRID_SIZE][GRID_SIZE];
static GridSquare_t gridSolved[GRID_SIZE][GRID_SIZE];
static char* gridData;
static int numberBag[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

// function declarations
void game_start(void);
void game_update(void);
void game_draw(void);

void grid_clear(void);
bool grid_is_solved(void);
bool grid_is_place_valid(int row, int col, int n);
// TODO: Muy ineficiente. Demasiados bucles.
void grid_load_random_seed(void);
void grid_rotate_90(void);
void grid_horizontal_flip(void);
void grid_vertical_flip(void);
void grid_randomly_mutate(void);
void grid_mark_pregenerated(void);

void number_bag_shuffle(void);

// entry point
int main(void) {
  srand(0);
  // 81 x 2 x 5
  // TODO: Somehow embed seeds into the code
  gridData = (char*)MemAlloc(((SEED_LEN * 2 * SEED_AMOUNT) + 1) * sizeof(char));
  gridData = LoadFileText("seeds.txt");
  UnloadFileText("seeds.txt");

  InitWindow(screenWidth, screenHeight, "Shudoku");
  SetTargetFPS(60);

  // Main game loop
  while (!WindowShouldClose()) {
    // Start
    if (!isGameInit) {
      game_start();
    }

    // Update
    game_update();

    // Render
    game_draw();
  }

  // Close window and OpenGL context
  CloseWindow();

  return 0;
}

void game_start(void) {
  isSquareSelected = false;
  mistakeCount = 0;

  grid_clear();
  grid_load_random_seed();
  grid_randomly_mutate();
  grid_mark_pregenerated();
}

void game_update(void) {
  // get mouse position
  Vector2 mousePos = GetMousePosition();

  // restart
  if (IsKeyReleased(KEY_R)) {
    TraceLog(LOG_INFO, TextFormat("%d %d %d %d %d %d %d %d %d", numberBag[0], numberBag[1], numberBag[2], numberBag[3], numberBag[4], numberBag[5], numberBag[6], numberBag[7], numberBag[8]));

    isGameInit = false;
  }

  // TODO: remove
  if (IsKeyReleased(KEY_S)) {
    for (int row = 0; row < GRID_SIZE; row++) {
      for (int col = 0; col < GRID_SIZE; col++) {
        GridSquare_t temp = grid[row][col];
        grid[row][col] = gridSolved[row][col];
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
    // set isSquareSelected to false so that clicking outside the grid to deselect works
    isSquareSelected = false;

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
          if (selectedRow == row && selectedCol == col) {
            isSquareSelected = false;
            selectedRow = 10;
            selectedCol = 10;
          }
          else {
            isSquareSelected = true;
            selectedRow = row;
            selectedCol = col;
          }
        }
        offset.x += SQUARE_SIZE;
      }
      offset.y += SQUARE_SIZE;
    }
  }

  // number input
  int keyPressed = GetKeyPressed();

  if (keyPressed >= KEY_ONE && keyPressed <= KEY_NINE) {
    if (isSquareSelected && !grid[selectedRow][selectedCol].isPregenerated && !grid[selectedRow][selectedCol].isHint) {
      // set number
      grid[selectedRow][selectedCol].number = keyPressed - KEY_ONE + 1;

      // check if it's wrong
      if (grid[selectedRow][selectedCol].number != gridSolved[selectedRow][selectedCol].number) {
        grid[selectedRow][selectedCol].isWrong = true;
        mistakeCount += 1;
      }
      else {
        grid[selectedRow][selectedCol].isWrong = false;
      }

      // deselect square
      isSquareSelected = false;
      selectedRow = 10;
      selectedCol = 10;
    }
  }

  return;
}

void game_draw(void) {
  BeginDrawing();
    ClearBackground(RAYWHITE);

    // Draw UI
    DrawRectangleLinesEx((Rectangle){(screenWidth / 2) - (GRID_SIZE * SQUARE_SIZE / 2), (screenHeight / 2) - SQUARE_SIZE * 4 - 50, SQUARE_SIZE * GRID_SIZE, SQUARE_SIZE * 1.5}, 2, GRAY);
    GuiLabel((Rectangle){(screenWidth / 2) - (GRID_SIZE * SQUARE_SIZE / 2), (screenHeight / 2) - SQUARE_SIZE * 4 - 50, SQUARE_SIZE * GRID_SIZE, SQUARE_SIZE * 1.5}, GuiIconText(ICON_CLOCK, "15:00"));

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
        if (isSquareSelected) {
          // highlight squares with the same number
          if (grid[row][col].number == grid[selectedRow][selectedCol].number && grid[selectedRow][selectedCol].number != 0) {
            DrawRectangle(offset.x, offset.y, SQUARE_SIZE, SQUARE_SIZE, (Color){200, 200, 225, 255});
          }
          // highlight selected square
          if (row == selectedRow && col == selectedCol) {
            DrawRectangle(offset.x, offset.y, SQUARE_SIZE, SQUARE_SIZE, (Color){150, 150, 200, 255});
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

        // draw surrounding square
        DrawRectangleLines(offset.x, offset.y, SQUARE_SIZE, SQUARE_SIZE, GRAY);

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

void grid_clear(void) {
  for (int row = 0; row < GRID_SIZE; row++) {
    for (int col = 0; col < GRID_SIZE; col++) {
      grid[row][col].isPregenerated = false;
      grid[row][col].isHint = false;
      grid[row][col].number = 0;

      gridSolved[row][col].isPregenerated = true;
      gridSolved[row][col].isHint = true;
      gridSolved[row][col].number = 0;
    }
  }

  isGameInit = true;
}

bool grid_is_solved(void) {
  for (int row = 0; row < GRID_SIZE; row++) {
    for (int col = 0; col < GRID_SIZE; col++) {
      if (grid[row][col].number == 0) {
        return false;
      }
    }
  }
  // else
  return true;
}

bool grid_is_place_valid(int row, int col, int n) {
  // check if n is already in row
  for (int i = 0; i < GRID_SIZE; i++) {
    if (grid[row][i].number == n) {
      return false;
    }
  }

  // check if n is already in column
  for (int i = 0; i < GRID_SIZE; i++) {
    if (grid[i][col].number == n) {
      return false;
    }
  }

  // check 3x3 submatrix
  int startRow = row - (row % 3);
  int startCol = col - (col % 3);

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      if (grid[startRow + i][startCol + j].number == n) {
        return false;
      }
    }
  }

  // if everything checks out, it's safe to place
  return true;
}

void grid_load_random_seed(void) {
  int seedNumber = rand() % SEED_AMOUNT;
  int seedStart = seedNumber * (SEED_LEN * 2);
  int solutionStart = seedStart + SEED_LEN;

  TraceLog(LOG_WARNING, TextFormat("seedNumber %d", seedNumber));
  TraceLog(LOG_WARNING, TextFormat("seedStart %d", seedStart));

  // numbers are randomised each time
  number_bag_shuffle();

  for (int row = 0; row < GRID_SIZE; row++) {
    for (int col = 0; col < GRID_SIZE; col++) {
      // fetch seed number
      char seedChar = gridData[seedStart + row * GRID_SIZE + col];
      int seedNumber;

      if (seedChar == '.') {
        seedNumber = 0;
      }
      else {
        seedNumber = numberBag[(int)(seedChar - 'a')];
      }

      grid[row][col].number = seedNumber;

      // fetch solution number
      char solutionChar = gridData[solutionStart + row * GRID_SIZE + col];
      int solutionNumber = numberBag[(int)(solutionChar - 'a')];

      gridSolved[row][col].number = solutionNumber;
    }
  }
}

void grid_randomly_mutate(void) {
  /*
    - shuffle each block's rows (same rotations for each block)
    - shuffle each block's cols (same rotations for each block)
    - rotate the whole thing by 90 / 180 / 270 degrees
    - flip horizontally
    - flip vertically
  */

  // 0 -> no rotation
  // 1 -> 90º
  // 2 -> 180º
  // 3 -> 270º
  int rngRotations = rand() % 4;

  TraceLog(LOG_WARNING, TextFormat("rngRotations %d", rngRotations));
  while (rngRotations != 0) {
    grid_rotate_90();
    rngRotations -= 1;
  }

  //

  // 0 -> no flip
  // 1 -> horizontal flip
  // 2 -> vertical flip
  // 3 -> horizontal, then vertical flip
  // 4 -> vertical, then horizontal flip
  int rngFlips = rand() % 5;

  if (rngFlips == 1) {
    grid_horizontal_flip();
  }
  else if (rngFlips == 2) {
    grid_vertical_flip();
  }
  else if (rngFlips == 3) {
    grid_horizontal_flip();
    grid_vertical_flip();
  }
  else if (rngFlips == 4) {
    grid_vertical_flip();
    grid_horizontal_flip();
  }

  TraceLog(LOG_WARNING, TextFormat("rngFlips %d", rngFlips));
}

void grid_rotate_90(void) {
  // Transpose
  for (int row = 0; row < GRID_SIZE; row++) {
    for (int col = row + 1; col < GRID_SIZE; col++) {
      // swap
      int temp = grid[row][col].number;
      grid[row][col].number = grid[col][row].number;
      grid[col][row].number = temp;

      // do the same to the solution
      temp = gridSolved[row][col].number;
      gridSolved[row][col].number = gridSolved[col][row].number;
      gridSolved[col][row].number = temp;
    }
  }

  // Reverse the rows
  for (int row = 0; row < GRID_SIZE; row++) {
    for (int col = 0; col < (GRID_SIZE / 2); col++) {
      // swap
      int temp = grid[row][col].number;
      grid[row][col].number = grid[row][GRID_SIZE - col - 1].number;
      grid[row][GRID_SIZE - col - 1].number = temp;

      // do the same to the solution
      temp = gridSolved[row][col].number;
      gridSolved[row][col].number = gridSolved[row][GRID_SIZE - col - 1].number;
      gridSolved[row][GRID_SIZE - col - 1].number = temp;
    }
  }
}

void grid_horizontal_flip(void) {
  int aux[GRID_SIZE][GRID_SIZE] = {0};
  int auxSolved[GRID_SIZE][GRID_SIZE] = {0};

  for (int col = 0; col < (GRID_SIZE / 2); col++) {
    for (int row = 0; row < GRID_SIZE; row++) {
      aux[row][GRID_SIZE - 1 - col] = grid[row][col].number;
      auxSolved[row][GRID_SIZE - 1 - col] = gridSolved[row][col].number;
    }
  }

  for (int col = (GRID_SIZE / 2); col < GRID_SIZE; col++) {
    for (int row = 0; row < GRID_SIZE; row++) {
      aux[row][GRID_SIZE - 1 - col] = grid[row][col].number;
      auxSolved[row][GRID_SIZE - 1 - col] = gridSolved[row][col].number;
    }
  }

  for (int row = 0; row < GRID_SIZE; row++) {
    for (int col = 0; col < GRID_SIZE; col++) {
      grid[row][col].number = aux[row][col];
      gridSolved[row][col].number = auxSolved[row][col];
    }
  }
}

void grid_vertical_flip(void) {
  int aux[GRID_SIZE][GRID_SIZE] = {0};
  int auxSolved[GRID_SIZE][GRID_SIZE] = {0};

  for (int row = 0; row < (GRID_SIZE / 2); row++) {
    for (int col = 0; col < GRID_SIZE; col++) {
      aux[GRID_SIZE - 1 - row][col] = grid[row][col].number;
      auxSolved[GRID_SIZE - 1 - row][col] = gridSolved[row][col].number;
    }
  }

  for (int row = (GRID_SIZE / 2); row < GRID_SIZE; row++) {
    for (int col = 0; col < GRID_SIZE; col++) {
      aux[GRID_SIZE - 1 - row][col] = grid[row][col].number;
      auxSolved[GRID_SIZE - 1 - row][col] = gridSolved[row][col].number;
    }
  }

  for (int row = 0; row < GRID_SIZE; row++) {
    for (int col = 0; col < GRID_SIZE; col++) {
      grid[row][col].number = aux[row][col];
      gridSolved[row][col].number = auxSolved[row][col];
    }
  }
}

void grid_mark_pregenerated(void) {
  for (int row = 0; row < GRID_SIZE; row++) {
    for (int col = 0; col < GRID_SIZE; col++) {
      if (grid[row][col].number != 0) {
        grid[row][col].isPregenerated = true;
      }
    }
  }
}

void number_bag_shuffle(void) {
  for (int i = 0; i < 9; i++) {
    int j = i + rand() / (RAND_MAX / (9 - i) + 1);
    int t = numberBag[j];
    numberBag[j] = numberBag[i];
    numberBag[i] = t;
  }
}
