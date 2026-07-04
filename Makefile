CC = gcc 
FLAGS = -Wall -Wextra -g
SRC = filetr.c
Target = server

all : 
	$(CC) $(FLAGS) $(SRC) -o $(Target)

run : all
	./$(Target)

clear :
	rm -rf $(Target)
