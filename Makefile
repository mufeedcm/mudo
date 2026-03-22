TARGET = mudo
SRC = main.c

VERSION = 0.1.1

BUILD_DIR = build
DIST_DIR = dist

CFLAGS = -std=c99 -Wall -Wextra -O2 -g
LDFLAGS = -lSDL2 -lSDL2_ttf

all: package

build:
	mkdir -p $(BUILD_DIR)/linux
	gcc $(CFLAGS) $(SRC) -o $(BUILD_DIR)/linux/$(TARGET) $(LDFLAGS)

	mkdir -p $(BUILD_DIR)/windows
	x86_64-w64-mingw32-gcc $(CFLAGS) $(SRC) -o $(BUILD_DIR)/windows/$(TARGET).exe \
		-lmingw32 -lSDL2main $(LDFLAGS) -mwindows

dist: build 
	rm -rf $(DIST_DIR)/mudo-v$(VERSION)-linux
	mkdir -p $(DIST_DIR)/mudo-v$(VERSION)-linux
	cp $(BUILD_DIR)/linux/$(TARGET) $(DIST_DIR)/mudo-v$(VERSION)-linux/
	cp -r assets $(DIST_DIR)/mudo-v$(VERSION)-linux/

	rm -rf $(DIST_DIR)/mudo-v$(VERSION)-windows
	mkdir -p $(DIST_DIR)/mudo-v$(VERSION)-windows
	cp $(BUILD_DIR)/windows/$(TARGET).exe $(DIST_DIR)/mudo-v$(VERSION)-windows/
	cp /usr/x86_64-w64-mingw32/bin/SDL2.dll $(DIST_DIR)/mudo-v$(VERSION)-windows/
	cp /usr/x86_64-w64-mingw32/bin/SDL2_ttf.dll $(DIST_DIR)/mudo-v$(VERSION)-windows/
	cp /usr/x86_64-w64-mingw32/bin/libgcc_s_seh-1.dll $(DIST_DIR)/mudo-v$(VERSION)-windows/
	cp /usr/x86_64-w64-mingw32/bin/libwinpthread-1.dll $(DIST_DIR)/mudo-v$(VERSION)-windows/
	cp /usr/x86_64-w64-mingw32/bin/libssp-0.dll $(DIST_DIR)/mudo-v$(VERSION)-windows/
	cp -r assets $(DIST_DIR)/mudo-v$(VERSION)-windows/

package: dist
	cd $(DIST_DIR) && \
		tar -czf mudo-v$(VERSION)-linux.tar.gz mudo-v$(VERSION)-linux && \
		zip -r mudo-v$(VERSION)-windows.zip mudo-v$(VERSION)-windows

run: dist
	./$(DIST_DIR)/mudo-v$(VERSION)-linux/$(TARGET)

clean: 
	rm -rf $(BUILD_DIR) $(DIST_DIR)

.PHONY: all build dist package run clean
