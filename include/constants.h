#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "raylib.h"
#include <stdlib.h>

#define GRID_SIZE 9
#define MAX_MISTAKES_ALLOWED 3
#define SEED_LEN 81
#define SEED_AMOUNT 5
#define COLOR_HIGHLIGHT CLITERAL(Color){ 213, 213, 242, 255 }
#define COLOR_SELECTED CLITERAL(Color){ 176, 176, 209, 255 }
#define SQUARE_SIZE 40
#define SQUARE_FONT_SIZE (SQUARE_SIZE - (SQUARE_SIZE / 4))

extern int screenWidth;
extern int screenHeight;

#endif // CONSTANTS_H
