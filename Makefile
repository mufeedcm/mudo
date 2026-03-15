TARGET = mudo
SRC = main.c

all:
		gcc -std=c99 -Wall -Wextra -O2 -g $(SRC) -o $(TARGET) -lraylib -lm -lpthread -ldl -lrt -lX11

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
