# Executable name
EXE = sudoku

# CFlags
CFLAGS = -Iinclude -std=c99 -MMD -MP # -Wall -Wfatal-errors -Wextra -Werror

# Linker flags
LDFLAGS = -Llib -lraylib -lopengl32 -lgdi32 -lwinmm

# Makefile wizardry
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%, obj/%, $(SRC:.c=.o))

# Make targets
$(EXE): $(OBJ)
		gcc $^ -o $@ $(LDFLAGS)

obj/%.o: src/%.c
		gcc $(CFLAGS) -c $< -o $@

run: $(EXE)
		./$(EXE)

clean:
		@del /q obj
		@del /q $(EXE).exe
