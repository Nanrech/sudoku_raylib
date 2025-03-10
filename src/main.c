#include "raylib.h"
#include "game.h"
#include "common.h"


int screenWidth;
int screenHeight;

int main(void) {
  // Start RNG
  srand(0);

  // Set fullscreen
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  screenWidth = GetScreenWidth();
  screenHeight = GetScreenHeight();

  // Create window
  InitWindow(screenWidth, screenHeight, "Shudoku");
  MaximizeWindow();
  SetTargetFPS(60);

  // Game loop
  while (!WindowShouldClose()) {
    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();

    if (!gameState.isGameInit) {
      game_start();
    }

    game_input();
    game_draw();
  }

  CloseWindow();
  return 0;
}
