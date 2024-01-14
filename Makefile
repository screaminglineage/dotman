STD = -std=c++20
FLAGS = -Wall -Weffc++ -Wextra -Wconversion -Wsign-conversion
EXECUTABLE = dotman
DB_FILE = db.sqlite

INCLUDE = ./include
SRC = ./src
TARGET = ./target
SQLITE3 = sqlite3

.PHONY: all
all: 
	$(CXX) $(STD) $(FLAGS) -I$(INCLUDE) -l$(SQLITE3) $(SRC)/*  -o $(TARGET)/$(EXECUTABLE)

# run program and show db contents
run: all
	$(TARGET)/$(EXECUTABLE) add kitty rofi
	sqlite3 $(DB_FILE) 'SELECT * FROM config_programs;'

deldb:
	rm $(DB_FILE)

.PHONY: clean
clean:
	rm $(TARGET)/*

