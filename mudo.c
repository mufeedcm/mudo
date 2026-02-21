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

#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

int windowHeight =  500;
int windowWidth = 400;

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

void handleInput(AppState *app){
  Vector2 mouse = GetMousePosition();

  float wheel = GetMouseWheelMove();
  app->scrollOffset -= wheel*20;
  if(app->scrollOffset < 0){ app->scrollOffset = 0;}
  int listHeight = app->count*40;
  int visibleHeight = windowHeight- 110;
  int maxScroll = listHeight - visibleHeight;
  if(maxScroll<0){ maxScroll = 0;}
  if(app->scrollOffset > maxScroll){app->scrollOffset = maxScroll;}

  // state button
  for(int i = 0; i<app->count;i++){
    int y= (i*40)+50 - app->scrollOffset;
    if(CheckCollisionPointRec(mouse, (Rectangle){10,y,65,30}) &&  IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
      app->todos[i].done = !app->todos[i].done;
      saveTodos(app);
      break;
    }
  }

  // lists
  for(int i = 0; i<app->count;i++){
    int y= (i*40)+50 - app->scrollOffset;
    if(CheckCollisionPointRec(mouse,(Rectangle){75,y,windowWidth-115,30})){ app->selected = -1;}
  }

  // delete button
  for(int i = 0; i<app->count;i++){
    int y= (i*40)+50 - app->scrollOffset;
    if(CheckCollisionPointRec(mouse, (Rectangle){360,y,30,30}) &&  IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
      app->selected =i;
      deleteTodo(app,app->selected);
      app->selected = -1;
      break;
    }
  }
  int key = GetCharPressed();
  while ( key >0){
    if(key>=32&&key<=126 && app->inputLen<1023){
      if(app->inputLen<1023){
        app->input[app->inputLen++] = (char)key;
        app->input[app->inputLen] = '\0';
      }
    }
    key = GetCharPressed();
  }
  if(IsKeyPressed(KEY_BACKSPACE)){
    if(app->inputLen>0){
      app->input[--app->inputLen] = '\0' ;
    }
  }

  if(IsKeyPressed(KEY_ENTER)){
    if(app->inputLen >0){
      addTodo(app,app->input);
      saveTodos(app);
      app->inputLen=0;
      app->input[0] = '\0';
    }

  }
  if(IsKeyPressed(KEY_DELETE) && app->selected>=0){
    deleteTodo(app,app->selected);
  }
  if(IsKeyPressed(KEY_DOWN)&&app->selected<app->count-1){
    app->selected++;
  }

  if(IsKeyPressed(KEY_UP)&&app->selected>0){
    app->selected--;
  }

  bool shifPress = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
  if(IsKeyPressed(KEY_TAB)){
    if (!shifPress && app->selected<app->count-1) {
      app->selected++;
    }
    if (shifPress && app->selected>0) {
      app->selected--;
    }
  }

  //done button
  if(CheckCollisionPointRec(mouse, (Rectangle){340,windowHeight -60,62,60}) &&  IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
    if(app->inputLen >0){
      addTodo(app,app->input);
      saveTodos(app);
      app->inputLen=0;
      app->input[0] = '\0';
    }
  }

}

void render(AppState *app,Font font){
  Vector2 mouse = GetMousePosition();
  DrawTextEx(font, "MUDO", (Vector2){170,15}, 18, 1, WHITE);

  BeginScissorMode(0, 50, windowWidth, windowHeight-110);
  for(int i = 0; i<app->count;i++){
    int y= (i*40)+50 - app->scrollOffset;

    // state button
    DrawRectangleRounded((Rectangle){10,y,65,30}, 0.4, 16, CheckCollisionPointRec(mouse, (Rectangle){10,y,65,30})? GRAY : DARKGRAY);
    DrawRectangle(70, y, 5, 30, CheckCollisionPointRec(mouse, (Rectangle){10,y,65,30})?GRAY:DARKGRAY);
    DrawTextEx(font,app->todos[i].done? "DONE" : "TODO",(Vector2){20,y+6}, 18,1, app->todos[i].done? ORANGE : WHITE);

    // lists
    DrawRectangle(75, y, windowWidth-115, 30,(CheckCollisionPointRec(mouse,(Rectangle){75,y,windowWidth-115,30})||(i==app->selected)) ? DARKGRAY :(Color){25,25,25,255});
    DrawTextEx(font,app->todos[i].text,(Vector2){80,y+6}, 18, 1, WHITE);

    // state strike
    if( app->todos[i].done){
      DrawRectangle(15,y+16, windowWidth-60, 1, MAROON);
    }

    // delete button
    DrawRectangleRounded((Rectangle){360,y,30,30}, 0.4, 16, CheckCollisionPointRec(mouse, (Rectangle){360,y,30,30})? GRAY : DARKGRAY);
    DrawRectangle(360, y, 5, 30, CheckCollisionPointRec(mouse, (Rectangle){360,y,30,30})?GRAY:DARKGRAY);
    DrawTextEx(font,"X",(Vector2){(Rectangle){360,y,30,30}.x+10, (Rectangle){360,y,30,30}.y+6}, 18, 1, WHITE);

  }
  EndScissorMode();

  DrawRectangle(0, windowHeight-60, windowWidth, 60, (Color){25,25,25,255});
  if (app->inputLen<=0) {
    DrawTextEx(font,"enter todo...", (Vector2){30, windowHeight-40}, 18, 1, GRAY);
  }

  // done button
  DrawTextEx(font,app->input,(Vector2){30,windowHeight-40}, 18, 1, WHITE);
  DrawRectangle(0,windowHeight-61,windowWidth,1, GRAY);
  DrawRectangle(339,windowHeight-61,1,60, GRAY);
  DrawRectangle(340,windowHeight-60,60,60, CheckCollisionPointRec(mouse, (Rectangle){340,windowHeight -60,62,60})? DARKGRAY : BLACK);
  DrawTextEx(font,"done", (Vector2){(Rectangle){340,windowHeight -60,62,60}.x+10, (Rectangle){340,windowHeight -60,62,60}.y+18}, 18, 1, WHITE);

}

int main() {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(windowWidth, windowHeight, "MUDO");
  SetTargetFPS(60);

  Font font = LoadFontEx("assets/fonts/Inter-Regular.ttf", 18,0,0);

  AppState app = {0};
  app.selected = -1;
  app.scrollOffset = 0;


  loadTodos(&app);

  while (!WindowShouldClose()) {
    handleInput(&app);
    BeginDrawing();
    ClearBackground(BLACK);
    render(&app,font);
    EndDrawing();
  }
  for(int i=0;i< app.count;i++){
    free(app.todos[i].text);
  }
  free(app.todos);
  UnloadFont(font);
  CloseWindow();
}
