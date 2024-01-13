STD = -std=c++20
FLAGS = -Wall -Weffc++ -Wextra -Wconversion -Wsign-conversion
EXECUTABLE = dotman

INCLUDE = ./include
SRC = ./src
TARGET = ./target
SQLITE3 = sqlite3

.PHONY: all
all: 
	$(CXX) $(STD) $(FLAGS) -I$(INCLUDE) -l$(SQLITE3) $(SRC)/*  -o $(TARGET)/$(EXECUTABLE)


.PHONY: clean
clean:
	rm $(TARGET)/*

