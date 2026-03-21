/*
 * MUDO  - A simple GUI todo application written in C using raylib
 * Copyright (C) 2025 mufeedcm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>. 
 */

#include "SDL2/SDL.h"
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static const SDL_Color WHITE      = {255,255,255,255};
static const SDL_Color BLACK      = {30,30,30,255};
static const SDL_Color PURE_BLACK = {0,0,0,255};
static const SDL_Color GRAY1      = {40,40,40,255};
static const SDL_Color GRAY2      = {50,50,50,255};
static const SDL_Color GRAY3      = {60,60,60,255};
static const SDL_Color GRAY4      = {70,70,70,255};
static const SDL_Color RED        = {255,0,0,255};
static const SDL_Color GREEN      = {0,255,0,255};

typedef struct {
  char *text;
  bool done;
}Todo;

typedef struct {
  Todo *todos;
  int count;
  int capacity;

  int selected;

  char input[1024];
  int inputLen; 

  int scrollOffset;
}AppState;

typedef struct {
  TTF_Font *small;
  TTF_Font *normal;
  TTF_Font *large;

}Fonts;

int contentWidth = 400;
int contentHeight =  500;

void saveTodos(AppState *app){
  char *home = getenv("HOME");
  if(!home){return;}
  char path[512];
  snprintf(path, sizeof(path), "%s/.mudo.txt",home);
  FILE* file;
  file = fopen(path,"w");
  if(file!=NULL){
    for(int i=0; i< app->count;i++){
      fprintf(file,"%s %s\n",
          app->todos[i].done? "DONE": "TODO",
          app->todos[i].text);
    } 
    fclose(file);
  }else{
    perror("fopen");
  }
}

void deleteTodo(AppState *app,int item){
  free (app->todos[item].text);
  for(int i= item;i<app->count-1;i++){
    app->todos[i] = app->todos[i+1];
  }
  (app->count)--;
  if(app->count==0){
    item=-1;
  }else if(item>=app->count){
    item=app->count-1;
  }
  saveTodos(app);
}

void addTodo(AppState *app,const char *text){
  if(app->count>=app->capacity){
    int newCapacity = app->capacity==0?4:app->capacity*2;
    Todo *newBlock = realloc(app->todos,newCapacity*sizeof(Todo));
    if(!newBlock){
      printf("memory allocation failed");
      return;
    }
    app->todos = newBlock;
    app->capacity = newCapacity;
  }
  app->todos[app->count].text = malloc(strlen(text)+1);
  strcpy(app->todos[app->count].text,text);
  app->todos[app->count].done = false;
  (app->count)++;

}

void loadTodos(AppState *app){
  char *home = getenv("HOME");
  if(!home){return;}
  char path[512];
  snprintf(path, sizeof(path), "%s/.mudo.txt",home);
  FILE* file;
  file = fopen(path,"r");
  char buffer[1024];

  if(file!=NULL){
    while(fgets(buffer,sizeof(buffer),file)){
      buffer[strcspn(buffer,"\n")] = '\0';
      bool done = false;
      char *textStart = buffer;
      if (strncmp(buffer,"DONE ",5)==0){
        done = true;
        textStart = buffer +5;
      }else if (strncmp(buffer,"TODO ",5)==0){
        done = false;
        textStart = buffer +5;
      }
      addTodo(app,textStart);
      app->todos[app->count-1].done = done;
    }
    fclose(file);
  }
}


void handleInput(AppState *app,SDL_Window *window, SDL_Event *event){

  if(event->type == SDL_MOUSEBUTTONDOWN){
    SDL_Point mouse; 
    SDL_GetMouseState(&mouse.x, &mouse.y);

    int w,h;
    SDL_GetWindowSize(window, &w, &h);
    int offsetX, offsetY;
    offsetX = w>contentWidth ? (w - contentWidth)/2 : 0;
    offsetY = h>contentHeight ? (h - contentHeight)/2 : 0;

    mouse.x -= offsetX;
    mouse.y -= offsetY;

    for(int i = 0; i<app->count;i++){
      int screenY= (i*40)+50 - app->scrollOffset;
      SDL_Rect state_btn = {10,screenY,65,30};
      SDL_Rect item = {75,screenY,contentWidth-115,30};
      SDL_Rect del_btn = {360,screenY,30,30};

      if(SDL_PointInRect(&mouse,&state_btn)){
        app->todos[i].done = !app->todos[i].done;
        saveTodos(app);
        break;
      }
      if(SDL_PointInRect(&mouse,&item)){
        app->selected = i;
      }
      if(SDL_PointInRect(&mouse,&del_btn)){
        deleteTodo(app,i);
        if(app->count ==0){
          app->selected = -1;
        }else if (i>=app->count) {
          app->selected = app->count -1;
        }else{
          app->selected = i;
        }
        break;
      }
    }

    SDL_Rect done_btn = {340,contentHeight -60,62,60};
    if(SDL_PointInRect(&mouse,&done_btn)){
      if(app->inputLen >0){
        addTodo(app,app->input);
        saveTodos(app);
        app->inputLen=0;
        app->input[0] = '\0';
      }
    }
  }
  if(event->type==SDL_MOUSEWHEEL){
    app->scrollOffset -= event->wheel.y*20;
    if(app->scrollOffset < 0){
      app->scrollOffset =0;
    }
    int maxScroll = (app->count *40+50)-(contentHeight-110);
    if(maxScroll<0){
      maxScroll =0;
    }
    if(app->scrollOffset > maxScroll){
      app->scrollOffset = maxScroll;
    }
  }


  if(event->type == SDL_TEXTINPUT){
    int len = strlen(event->text.text);
    if(app->inputLen + len <1024){
      memcpy(app->input + app->inputLen, event->text.text, len);
      app->inputLen +=len;
      app->input[app->inputLen] = '\0';
    }
  }
  if(event->type ==SDL_KEYDOWN){
    switch(event->key.keysym.sym){
      case SDLK_BACKSPACE:
        if(app->inputLen>0){
          app->input[--app->inputLen] = '\0' ;
        }
        break;
      case SDLK_RETURN:
      case SDLK_KP_ENTER:
        if(app->inputLen >0){
          addTodo(app,app->input);
          saveTodos(app);
          app->inputLen=0;
          app->input[0] = '\0';
        }
        break;
      case SDLK_DELETE:
        if(app->selected>=0){
          deleteTodo(app,app->selected);

          if(app->count ==0){
            app->selected = -1;
          }else if ( app->selected>=app->count) {
            app->selected = app->count -1;
          }        }
        break;
      case SDLK_DOWN:
        if(app->selected<app->count-1){
          app->selected++;
        }
        break;
      case SDLK_UP:
        if(app->selected>0){
          app->selected--;
        }
        break;
      case SDLK_TAB:
        if(event->key.keysym.mod & KMOD_SHIFT){
          if(app->selected>0) app->selected--;
        }else{
          if(app->selected<app->count-1) app->selected++;
        }
        break;
    }
  }
}

void drawText(SDL_Renderer *renderer,TTF_Font *font, const char *text,int x, int y, SDL_Color color){
  SDL_Surface *surface = TTF_RenderUTF8_Blended(font, text, color);
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_Rect dst = {x,y,surface->w,surface->h};
  SDL_FreeSurface(surface);
  SDL_RenderCopy(renderer, texture, NULL, &dst);
  SDL_DestroyTexture(texture);
}

void render(AppState *app,SDL_Window *window,SDL_Renderer *renderer,Fonts *fonts){
  int w,h;
  SDL_GetWindowSize(window, &w, &h);

  int offsetX, offsetY;
  offsetX = w>contentWidth ? (w - contentWidth)/2 : 0;
  offsetY = h>contentHeight ? (h - contentHeight)/2 : 0;

  SDL_Point mouse; 
  SDL_GetMouseState(&mouse.x, &mouse.y);

  mouse.x -= offsetX;
  mouse.y -= offsetY;

  SDL_SetRenderDrawColor(renderer, PURE_BLACK.r, PURE_BLACK.g, PURE_BLACK.b, PURE_BLACK.a);
  SDL_RenderClear(renderer);

  SDL_Rect content_bg = {offsetX,offsetY,contentWidth,contentHeight};
  SDL_SetRenderDrawColor(renderer, BLACK.r,BLACK.g,BLACK.b,BLACK.a);
  SDL_RenderFillRect(renderer, &content_bg);

  drawText(renderer, fonts->large, "MUDO", offsetX+170,offsetY+15, WHITE);

  SDL_RenderSetClipRect(renderer, &(SDL_Rect){offsetX,offsetY+50,contentWidth,contentHeight-110});

  for(int i = 0; i<app->count;i++){
    int screenY= (i*40)+50 - app->scrollOffset;

    SDL_Rect state_btn = {10,screenY,65,30};
    SDL_Rect state_btn_draw = {offsetX+state_btn.x,offsetY+state_btn.y,state_btn.w,state_btn.h};

    if(SDL_PointInRect(&mouse,&state_btn)){
      SDL_SetRenderDrawColor(renderer, GRAY2.r,GRAY2.g,GRAY2.b,GRAY2.a);
    }else{
      SDL_SetRenderDrawColor(renderer, GRAY1.r,GRAY1.g,GRAY1.b,GRAY1.a);
    }
    SDL_RenderFillRect(renderer, &state_btn_draw);
    drawText(renderer, fonts->normal, app->todos[i].done? "DONE" : "TODO", offsetX+20, offsetY+screenY+8,app->todos[i].done? RED :GREEN );

    SDL_Rect item = {75,screenY,contentWidth-115,30};
    SDL_Rect item_draw = {offsetX+item.x,offsetY+item.y,item.w,item.h};
    if((i==app->selected)||SDL_PointInRect(&mouse,&item)){
      SDL_SetRenderDrawColor(renderer, GRAY4.r,GRAY4.g,GRAY4.b,GRAY4.a);
    }else{
      SDL_SetRenderDrawColor(renderer, GRAY3.r,GRAY3.g,GRAY3.b,GRAY3.a);
    }
    SDL_RenderFillRect(renderer, &item_draw);
    drawText(renderer, fonts->normal, app->todos[i].text, offsetX+80, offsetY+(screenY+8), WHITE);

    if( app->todos[i].done){
      SDL_Rect strike_line = {offsetX+15,offsetY+(screenY+16), contentWidth-60, 1};
      SDL_SetRenderDrawColor(renderer, RED.r,RED.g,RED.b,RED.a);
      SDL_RenderFillRect(renderer, &strike_line);
    }

    SDL_Rect del_btn = {360,screenY,30,30};
    SDL_Rect del_btn_draw = {offsetX+del_btn.x,offsetY+del_btn.y,del_btn.w,del_btn.h};
    if(SDL_PointInRect(&mouse,&del_btn)){
      SDL_SetRenderDrawColor(renderer, GRAY2.r,GRAY2.g,GRAY2.b,GRAY2.a);
    }else{
      SDL_SetRenderDrawColor(renderer, GRAY1.r,GRAY1.g,GRAY1.b,GRAY1.a);
    }
    SDL_RenderFillRect(renderer, &del_btn_draw);
    drawText(renderer, fonts->normal, "X", offsetX+370, offsetY+(screenY+8), RED);
  }
  SDL_RenderSetClipRect(renderer, NULL);

  SDL_Rect text_box = {offsetX,offsetY+(contentHeight-60),contentWidth,60};
  SDL_SetRenderDrawColor(renderer, GRAY1.r,GRAY1.g,GRAY1.b,GRAY1.a);
  SDL_RenderFillRect(renderer, &text_box);

  if (app->inputLen<=0) {
    drawText(renderer, fonts->normal, "enter todo...", offsetX+30, offsetY+(contentHeight-40), WHITE);
  }else{
    drawText(renderer, fonts->normal, app->input, offsetX+30, offsetY+(contentHeight-40), WHITE);
  }

  SDL_Rect done_btn = {340,(contentHeight -60),60,60};
  SDL_Rect done_btn_draw = {offsetX+done_btn.x,offsetY+done_btn.y,done_btn.w,done_btn.h};
  if(SDL_PointInRect(&mouse,&done_btn)){
    SDL_SetRenderDrawColor(renderer, GRAY4.r,GRAY4.g,GRAY4.b,GRAY4.a);
  }else{
    SDL_SetRenderDrawColor(renderer, GRAY3.r,GRAY3.g,GRAY3.b,GRAY3.a);
  }
  SDL_RenderFillRect(renderer, &done_btn_draw);
  drawText(renderer, fonts->normal, "done", offsetX+350, offsetY+(contentHeight-40), WHITE);

  SDL_RenderPresent(renderer);
}

int main() {
  if(SDL_Init(SDL_INIT_VIDEO) !=0) printf("SDL Init Failed: %s \n",SDL_GetError());
  TTF_Init();

  Fonts fonts;
  fonts.small = TTF_OpenFont("assets/fonts/Inter-Regular.ttf", 12);
  fonts.normal = TTF_OpenFont("assets/fonts/Inter-Regular.ttf", 14);
  fonts.large = TTF_OpenFont("assets/fonts/Inter-Regular.ttf", 18);

  SDL_Window *window = SDL_CreateWindow("MUDO", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 400, 500, 0);
  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  SDL_SetWindowResizable(window, SDL_TRUE);

  AppState app = {0};
  app.selected = -1;
  app.scrollOffset = 0;

  loadTodos(&app);
  SDL_StartTextInput();

  int running = 1;
  while(running){
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
      handleInput(&app,window,&event);
      if (event.type == SDL_QUIT) {
        running = 0;
      }
    }
    render(&app,window, renderer,&fonts);
  }

  for(int i=0;i< app.count;i++){
    free(app.todos[i].text);
  }
  free(app.todos);

  TTF_CloseFont(fonts.small);
  TTF_CloseFont(fonts.normal);
  TTF_CloseFont(fonts.large);
  TTF_Quit();

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
