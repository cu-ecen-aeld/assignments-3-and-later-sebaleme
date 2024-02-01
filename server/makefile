# Copyright (c) 2024 Sebastien Lemetter
# makefile for aesdsocket.c
# Available commands: make and make clean
# ========================================== 

# For X-Compilation, define the variable as follow before running the make command
export CROSS_COMPILE=aarch64-none-linux-gnu-
CC = $(CROSS_COMPILE)gcc
OBJ = aesdsocket

# This is our default rule, so must come first
all: socketmake

# Executing make with no argument will execute the first command only
socketmake: aesdsocket.c
	$(CC) -g -Wall -o $(OBJ) aesdsocket.c

.PHONY: clean

clean:
	rm -f *.o aesdsocket