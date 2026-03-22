TARGET = mudo
SRC = main.c

VERSION = 0.1.1

BUILD_DIR = build
DIST_DIR = dist

CFLAGS = -std=c99 -Wall -Wextra -O2 -g
LDFLAGS = -lSDL2 -lSDL2_ttf

build: linux 

linux:
	mkdir -p $(BUILD_DIR)/linux
	gcc $(CFLAGS) $(SRC) -o $(BUILD_DIR)/linux/$(TARGET) $(LDFLAGS)

dist: build 
	rm -rf $(DIST_DIR)/linux
	mkdir -p $(DIST_DIR)/linux
	cp $(BUILD_DIR)/linux/$(TARGET) $(DIST_DIR)/linux/
	cp -r assets $(DIST_DIR)/linux/

package: dist
	cd $(DIST_DIR) && tar -czf mudo-v$(VERSION)-linux.tar.gz linux

run: dist
	./$(DIST_DIR)/linux/$(TARGET)

clean: 
	rm -rf $(BUILD_DIR) $(DIST_DIR)

.PHONY: build linux dist package run clean
