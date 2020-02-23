# COMP30023 Computer Systems
# Semester 1 2017
# Project 1     : Makefile
# Student Name  : Ivan Ken Weng Chee
# Student ID    : 736901
# Student Email : ichee@student.unimelb.edu.au

## CC     : Compiler
## CFLAGS : Compiler flags
CC     = gcc
CFLAGS = -Wall -Wextra -ansi -std=gnu99 -g -O0 -no-pie

## OBJ : Object files
## SRC : Source files
## EXE : Executable name
SRC = src/server.c src/list.c src/sha256.c
OBJ = src/server.o src/list.o src/sha256.o
EXE = bin/server

## Run:
run:
	$(EXE) 12345

## Compile:
compile:	$(OBJ)
		$(CC) $(CFLAGS) -o $(EXE) $(OBJ) -lm -lpthread

## Clean: Remove object files and core dump files.
clean:
	rm $(OBJ) log.txt

## Clobber: Performs Clean and removes executable file.
clobber: clean
	rm $(EXE)

## Dependencies
src/server.o : src/server.h src/list.h src/sha256.h src/uint256.h
src/sha256.o : src/sha256.h
src/list.o   : src/list.h
