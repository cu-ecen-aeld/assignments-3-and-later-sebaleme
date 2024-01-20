# Copyright (c) 2023 Sebastien Lemetter
# makefile for writer.c
# Available commands: make and make clean
# ========================================== 

# For X-Compilation, define the variable as follow before running the make command
export CROSS_COMPILE=aarch64-none-linux-gnu-
CC = $(CROSS_COMPILE)gcc
OBJ = writer

# This is our default rule, so must come first
all: writermake

# Executing make with no argument will execute the first command only
writermake: writer.c
	$(CC) -g -Wall -o $(OBJ) writer.c

.PHONY: clean

clean:
	rm -f *.o writer