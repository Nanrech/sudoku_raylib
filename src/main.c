#include "raylib.h"
#include "game_state.h"
#include "constants.h"


int main(void) {
  srand(0);

  InitWindow(screenWidth, screenHeight, "Shudoku");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    if (!gameState.isGameInit) {
      game_start();
    }

    game_update();
    game_draw();
  }

  CloseWindow();
  return 0;
}
