#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define GRID_SIZE 9
#define MAX_MISTAKES_ALLOWED 3
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
static bool isGameWon = false;
static bool isGameLost = false;

static int mistakeCount = 0;

static bool isAnySquareSelected = false;
static int selectedRow = 0;
static int selectedCol = 0;

static GridSquare_t grid[GRID_SIZE][GRID_SIZE];
static int gridSolved[GRID_SIZE][GRID_SIZE];
static char seedData[810] = "g.ca..fi.ida.f........ch.a.dgf....bhb..f.i..da......e.hi..d..f.c.g.ife...b..ea...ghcabdfieidagfebhcefbichdagdgfeacibhbehfgiacdacidhbgefhiebdgcfacaghifedbfbdceahgi..b.ci.h......eg..a..hg..b..a.g.b..d....hc..afhi.dacg..bg.a....hdf.e..a....cf..d.gfbdciahedihabegcfacehgfdbieacgibhfdbgdfhceiafhiedacgbcbgiadfehhdfbegiacieacfhbdg.g.da...f...e...c.ef.h..ba..c...da..b.e....dcf..ich.bgd....g..ec.f..i...ib..def..hgcdabiefadbeifgchefihgcbadgchbedafibiegfahdcfadichebgdhafbgciecefahidgbibgcdefhaf..bi..c..c.a.h..b..becf..h..cgb.ae.a....d...g....a.idif.hgb..cc......b..eg......fahbigdceeciadhfgbdgbecfiahhdcgbiaefaifcedbhggbefhacidifahgbedcchdifegbabegdachfi.fd..b...b.ced...ig.....f..d....h.f...hi....a.i.f..hebic.d....he...a.d..ad....egchfdgibacebacedfghigeichafbddbeachifgfghibecdaciafgdhebicfdegbahehgbacdifadbhfiegc";
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
  isGameWon = false;
  isGameLost = false;

  isAnySquareSelected = false;
  mistakeCount = 0;

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
    TraceLog(LOG_INFO, TextFormat("%d %d %d %d %d %d %d %d %d", numberBag[0], numberBag[1], numberBag[2], numberBag[3], numberBag[4], numberBag[5], numberBag[6], numberBag[7], numberBag[8]));

    isGameInit = false;
  }

  // if the game is over (either win or lose)
  // reject any grid input
  if (isGameWon || isGameLost) {
    return;
  }

  // TODO: remove
  if (IsKeyReleased(KEY_H)) {
    if (isAnySquareSelected) {
      grid[selectedRow][selectedCol].number = gridSolved[selectedRow][selectedCol];
      grid[selectedRow][selectedCol].isHint = true;
      grid[selectedRow][selectedCol].isWrong = false;

      // deselect square
      isAnySquareSelected = false;
      selectedRow = 10;
      selectedCol = 10;
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
    isAnySquareSelected = false;

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
            isAnySquareSelected = false;
            selectedRow = 10;
            selectedCol = 10;
          }
          else {
            isAnySquareSelected = true;
            selectedRow = row;
            selectedCol = col;
          }
        }
        offset.x += SQUARE_SIZE;
      }
      offset.y += SQUARE_SIZE;
    }
  }

  // insert number
  if (keyPressed >= KEY_ONE && keyPressed <= KEY_NINE) {
    if (!isAnySquareSelected) {
      return;
    }
    // if number is NOT 0 and NOT wrong, return
    if (grid[selectedRow][selectedCol].number != 0 && !grid[selectedRow][selectedCol].isWrong) {
      return;
    }

    // set number
    int selectedNumber = keyPressed - KEY_ONE + 1;
    grid[selectedRow][selectedCol].number = selectedNumber;

    // check if it's wrong
    if (selectedNumber != gridSolved[selectedRow][selectedCol] || grid_is_place_valid(selectedRow, selectedCol, selectedNumber)) {
      grid[selectedRow][selectedCol].isWrong = true;
      mistakeCount += 1;
    }
    else {
      grid[selectedRow][selectedCol].isWrong = false;
    }

    // deselect square
    isAnySquareSelected = false;
    selectedRow = 10;
    selectedCol = 10;
  }

  // check lose condition
  if (mistakeCount >= MAX_MISTAKES_ALLOWED) {
    isGameLost = true;
  }

  // check win condition
  if (grid_is_solved()) {
    isGameWon = true;
  }
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
        if (isAnySquareSelected) {
          int selectedNumber = grid[selectedRow][selectedCol].number;

          // highlight selected square
          // if number == 0 OR if the current number is wrong
          if (row == selectedRow && col == selectedCol && (grid[selectedRow][selectedCol].number == 0 || grid[selectedRow][selectedCol].isWrong)) {
            DrawRectangle(offset.x, offset.y, SQUARE_SIZE, SQUARE_SIZE, (Color){150, 150, 200, 255});
          }

          // if selected square is 0, highlight relevant squares
          else if (selectedNumber == 0) {
            if (row == selectedRow || col == selectedCol) {
              DrawRectangle(offset.x, offset.y, SQUARE_SIZE, SQUARE_SIZE, (Color){213, 213, 242, 255});
            }
          }

          // highlight squares with the same (non 0) number
          else if (grid[row][col].number == selectedNumber && selectedNumber != 0) {
            DrawRectangle(offset.x, offset.y, SQUARE_SIZE, SQUARE_SIZE, (Color){213, 213, 242, 255});
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

void grid_clear(void) {
  for (int row = 0; row < GRID_SIZE; row++) {
    for (int col = 0; col < GRID_SIZE; col++) {
      grid[row][col].isPregenerated = false;
      grid[row][col].isHint = false;
      grid[row][col].isWrong = false;
      grid[row][col].number = 0;
    }
  }

  isGameInit = true;
}

bool grid_is_solved(void) {
  for (int row = 0; row < GRID_SIZE; row++) {
    for (int col = 0; col < GRID_SIZE; col++) {
      if (grid[row][col].number != gridSolved[row][col]) {
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
      char seedChar = seedData[seedStart + row * GRID_SIZE + col];
      int seedNumber;

      if (seedChar == '.') {
        seedNumber = 0;
      }
      else {
        seedNumber = numberBag[(int)(seedChar - 'a')];
      }

      grid[row][col].number = seedNumber;

      // fetch solution number
      char solutionChar = seedData[solutionStart + row * GRID_SIZE + col];
      int solutionNumber = numberBag[(int)(solutionChar - 'a')];

      gridSolved[row][col] = solutionNumber;
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
      temp = gridSolved[row][col];
      gridSolved[row][col] = gridSolved[col][row];
      gridSolved[col][row] = temp;
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
      temp = gridSolved[row][col];
      gridSolved[row][col] = gridSolved[row][GRID_SIZE - col - 1];
      gridSolved[row][GRID_SIZE - col - 1] = temp;
    }
  }
}

void grid_horizontal_flip(void) {
  int aux[GRID_SIZE][GRID_SIZE] = {0};
  int auxSolved[GRID_SIZE][GRID_SIZE] = {0};

  for (int col = 0; col < (GRID_SIZE / 2); col++) {
    for (int row = 0; row < GRID_SIZE; row++) {
      aux[row][GRID_SIZE - 1 - col] = grid[row][col].number;
      auxSolved[row][GRID_SIZE - 1 - col] = gridSolved[row][col];
    }
  }

  for (int col = (GRID_SIZE / 2); col < GRID_SIZE; col++) {
    for (int row = 0; row < GRID_SIZE; row++) {
      aux[row][GRID_SIZE - 1 - col] = grid[row][col].number;
      auxSolved[row][GRID_SIZE - 1 - col] = gridSolved[row][col];
    }
  }

  for (int row = 0; row < GRID_SIZE; row++) {
    for (int col = 0; col < GRID_SIZE; col++) {
      grid[row][col].number = aux[row][col];
      gridSolved[row][col] = auxSolved[row][col];
    }
  }
}

void grid_vertical_flip(void) {
  int aux[GRID_SIZE][GRID_SIZE] = {0};
  int auxSolved[GRID_SIZE][GRID_SIZE] = {0};

  for (int row = 0; row < (GRID_SIZE / 2); row++) {
    for (int col = 0; col < GRID_SIZE; col++) {
      aux[GRID_SIZE - 1 - row][col] = grid[row][col].number;
      auxSolved[GRID_SIZE - 1 - row][col] = gridSolved[row][col];
    }
  }

  for (int row = (GRID_SIZE / 2); row < GRID_SIZE; row++) {
    for (int col = 0; col < GRID_SIZE; col++) {
      aux[GRID_SIZE - 1 - row][col] = grid[row][col].number;
      auxSolved[GRID_SIZE - 1 - row][col] = gridSolved[row][col];
    }
  }

  for (int row = 0; row < GRID_SIZE; row++) {
    for (int col = 0; col < GRID_SIZE; col++) {
      grid[row][col].number = aux[row][col];
      gridSolved[row][col] = auxSolved[row][col];
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
