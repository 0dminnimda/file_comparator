
.PHONY: compile
compile:
		clang main.c memmap.c -o main.out -Ofast -Wall -Wextra -pedantic

