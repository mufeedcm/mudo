# MUDO  

A simple GUI todo application written in C using SDL2.



## Install dependencies

#### Linux
```bash
sudo pacman -S gcc sdl2 sdl2_ttf 
```

#### Windows
```bash
sudo pacman -S mingw-w64-gcc 
yay -S mingw-w64-sdl2 mingw-w64-sdl2-ttf

# optional wine for testing
sudo pacman -S wine

```

#### Web
```bash
# install emscripten
https://emscripten.org/docs/getting_started/downloads.html

#optional python for running
sudo pacman -S python

```

## Run

#### Linux

```bash
make run-linux
```
#### Windows

```bash
make run-windows
```

#### Web 

```bash
make run-web
```

