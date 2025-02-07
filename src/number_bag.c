#include "number_bag.h"


int numberBag[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

void number_bag_shuffle(void) {
  for (int i = 8; i > 0; i--) {
    int j = rand() % (i + 1);
    int temp = numberBag[i];
    numberBag[i] = numberBag[j];
    numberBag[j] = temp;
  }
}
