#include "grid.h"
#include "seed.h"


int numberBag[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

void number_bag_shuffle(void) {
  for (int i = 8; i > 0; i--) {
    int j = rand() % (i + 1);
    int temp = numberBag[i];
    numberBag[i] = numberBag[j];
    numberBag[j] = temp;
  }
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

  gameState.isGameInit = true;
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

// TODO: Simplify
void grid_rotate(int times) {}

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

// TODO: Unify
void grid_flip(int mode) {}

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