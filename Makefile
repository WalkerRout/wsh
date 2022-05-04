CC = gcc
OBJS = src/*.c 
OBJ = bin/wsh
CFLAGS = 
LIBS =

all: build run

build:
	@$(CC) $(OBJS) $(CFLAGS) $(LIBS) -o $(OBJ)

run:
	@./$(OBJ)

clean:
	@rm ./$(OBJ)
	@echo "Cleaned!"
