TARGET = mudo
SRC = main.c

all:
		gcc -std=c99 -Wall -Wextra -O2 -g $(SRC) -o $(TARGET) -lSDL2 -lSDL2_ttf

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
