FLAGS = -std=c++20 -Werror -Wall -Weffc++ -Wextra -Wconversion -Wsign-conversion
EXECUTABLE = dotman

.PHONY: all
all:
	$(CXX) $(FLAGS) main.cpp -o $(EXECUTABLE)

.PHONY: clean
clean:
	rm $(EXECUTABLE)

