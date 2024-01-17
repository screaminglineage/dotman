STD = -std=c++20
FLAGS = -Wall -Weffc++ -Wextra -Wconversion -Wsign-conversion
BIN = dotman
DB_FILE = db.sqlite

INCLUDE = ./include
SRC = ./src
BUILD = ./build
SQLITE3 = sqlite3
BIN_PATH = $(BUILD)/$(BIN)

.PHONY: all run deldb clean
all: $(BIN_PATH)

# run program and show db contents
run: $(BIN_PATH)
	$(BIN_PATH) add kitty rofi
	sqlite3 $(DB_FILE) 'SELECT * FROM config_programs;'
	sqlite3 $(DB_FILE) 'SELECT * FROM config_files;'

$(BIN_PATH): $(SRC)/*.cpp
	$(CXX) $(STD) $(FLAGS) -I$(INCLUDE) -l$(SQLITE3) $? -o $@

deldb:
	rm $(DB_FILE)

clean:
	rm $(BIN_PATH)

