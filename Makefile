TARGET = mudo
SRC = main.c

VERSION = 0.1.2

BUILD_DIR = build
DIST_DIR = dist

CFLAGS = -std=c99 -Wall -Wextra -O2 
LDFLAGS = -lSDL2 -lSDL2_ttf

all: dist

build:
ifeq ($(shell uname),Linux)
		$(MAKE) build-linux
else ifeq ($(shell uname),Darwin)
		$(MAKE) build-macos
endif
	
build-linux:
	mkdir -p $(BUILD_DIR)/linux
	gcc $(CFLAGS) $(SRC) -o $(BUILD_DIR)/linux/$(TARGET) $(LDFLAGS)

build-macos:
	mkdir -p $(BUILD_DIR)/macos
	gcc $(CFLAGS) $(SRC) -o $(BUILD_DIR)/macos/$(TARGET) \
		-I/opt/homebrew/include \
		-L/opt/homebrew/lib -lSDL2 -lSDL2_ttf

build-windows:
	mkdir -p $(BUILD_DIR)/windows
	x86_64-w64-mingw32-gcc $(CFLAGS) $(SRC) -o $(BUILD_DIR)/windows/$(TARGET).exe \
		-lmingw32 -lSDL2main $(LDFLAGS) -mwindows

build-web:
	mkdir -p $(BUILD_DIR)/web
	emcc $(SRC) -o $(BUILD_DIR)/web/index.html \
		-s USE_SDL=2 -s USE_SDL_TTF=2 -s USE_FREETYPE=1 \
		--preload-file assets


dist: 
ifeq ($(shell uname),Linux)
	$(MAKE) dist-linux
	$(MAKE) dist-windows
	$(MAKE) dist-web
else ifeq ($(shell uname),Darwin)
	$(MAKE) dist-macos
endif

dist-linux: build-linux
	rm -rf $(DIST_DIR)/mudo-v*-linux
	mkdir -p $(DIST_DIR)/mudo-v$(VERSION)-linux
	cp $(BUILD_DIR)/linux/$(TARGET) $(DIST_DIR)/mudo-v$(VERSION)-linux/
	cp -r assets $(DIST_DIR)/mudo-v$(VERSION)-linux/
	cd $(DIST_DIR) && \
	tar -czf mudo-v$(VERSION)-linux.tar.gz mudo-v$(VERSION)-linux 

dist-windows: build-windows
	rm -rf $(DIST_DIR)/mudo-v*-windows
	mkdir -p $(DIST_DIR)/mudo-v$(VERSION)-windows
	cp $(BUILD_DIR)/windows/$(TARGET).exe $(DIST_DIR)/mudo-v$(VERSION)-windows/
	cp /usr/x86_64-w64-mingw32/bin/SDL2.dll $(DIST_DIR)/mudo-v$(VERSION)-windows/
	cp /usr/x86_64-w64-mingw32/bin/SDL2_ttf.dll $(DIST_DIR)/mudo-v$(VERSION)-windows/
	cp /usr/x86_64-w64-mingw32/bin/libgcc_s_seh-1.dll $(DIST_DIR)/mudo-v$(VERSION)-windows/
	cp /usr/x86_64-w64-mingw32/bin/libwinpthread-1.dll $(DIST_DIR)/mudo-v$(VERSION)-windows/
	cp /usr/x86_64-w64-mingw32/bin/libssp-0.dll $(DIST_DIR)/mudo-v$(VERSION)-windows/
	cp -r assets $(DIST_DIR)/mudo-v$(VERSION)-windows/
	cd $(DIST_DIR) && \
	zip -r mudo-v$(VERSION)-windows.zip mudo-v$(VERSION)-windows 

dist-web: build-web
	rm -rf $(DIST_DIR)/mudo-v*-web
	mkdir -p $(DIST_DIR)/mudo-v$(VERSION)-web
	cp $(BUILD_DIR)/web/* $(DIST_DIR)/mudo-v$(VERSION)-web/
	cd $(DIST_DIR) && \
	zip -r mudo-v$(VERSION)-web.zip mudo-v$(VERSION)-web

dist-macos: build-macos
	rm -rf $(DIST_DIR)/mudo-v*-macos
	mkdir -p $(DIST_DIR)/mudo-v$(VERSION)-macos
	cp $(BUILD_DIR)/macos/$(TARGET) $(DIST_DIR)/mudo-v$(VERSION)-macos/
	cp -r assets $(DIST_DIR)/mudo-v$(VERSION)-macos/
	cd $(DIST_DIR) && \
	zip -r mudo-v$(VERSION)-macos.zip mudo-v$(VERSION)-macos

run: 
ifeq ($(shell uname),Linux)
	$(MAKE)	run-linux
else ifeq ($(shell uname),Darwin)
	$(MAKE)	run-macos
endif
		
run-linux: dist-linux
	./$(DIST_DIR)/mudo-v$(VERSION)-linux/$(TARGET)

run-macos: dist-macos
	./$(DIST_DIR)/mudo-v$(VERSION)-macos/$(TARGET)

run-windows: dist-windows
	cd $(DIST_DIR)/mudo-v$(VERSION)-windows/ && wine $(TARGET).exe

run-web: dist-web
	cd $(DIST_DIR)/mudo-v$(VERSION)-web/ && python -m http.server & \
		xdg-open http://localhost:8000

clean: 
	rm -rf $(BUILD_DIR) $(DIST_DIR)

.PHONY: all build build-linux build-windows build-web build-macos dist dist-linux dist-windows dist-web dist-macos run run-linux run-windows run-web run-macos clean
