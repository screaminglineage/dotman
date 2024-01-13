STD = -std=c++20
FLAGS = -Wall -Weffc++ -Wextra -Wconversion -Wsign-conversion
EXECUTABLE = dotman

INCLUDE = ./include
SRC = ./src
TARGET = ./target
SQLITE3_O = $(INCLUDE)/libsqlite3.so.0.8.6

.PHONY: all
all: 
	$(CXX) $(STD) $(FLAGS) -I$(INCLUDE) $(SRC)/* $(SQLITE3_O) -o $(TARGET)/$(EXECUTABLE)


.PHONY: clean
clean:
	rm $(TARGET)/*

