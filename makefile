# Executable name
EXE = sudoku

# Compiler
CC = gcc

# CFlags
CFLAGS = -Iinclude -std=c99 -MMD -MP -DPLATFORM_DESKTOP -Wall

# Linker flags
LDFLAGS = -Llib -lraylib -lopengl32 -lgdi32 -lwinmm

# Makefile wizardry
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%, obj/%, $(SRC:.c=.o))

# Make targets
$(EXE): $(OBJ)
		$(CC) $^ -o $@ $(LDFLAGS)

obj/%.o: src/%.c
		$(CC) $(CFLAGS) -c $< -o $@

run: $(EXE)
		./$(EXE)

# TODO
# web:

clean:
		@del /q obj
		@del /q $(EXE).exe
