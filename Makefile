STD = -std=c++20
FLAGS = -Wall -Weffc++ -Wextra -Wconversion -Wsign-conversion
EXECUTABLE = dotman

INCLUDE = ./include
SQLITE3_O = $(INCLUDE)/libsqlite3.so.0.8.6
SOURCES = main.cpp database.cpp

.PHONY: all
all: 
	$(CXX) $(STD) $(FLAGS) -I$(INCLUDE) $(SOURCES) $(SQLITE3_O) -o ./target/$(EXECUTABLE)


.PHONY: clean
clean:
	rm target/*

