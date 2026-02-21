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
  int listHeight = app->count*30;
  int visibleHeight = windowHeight- 110;
  int maxScroll = listHeight - visibleHeight;
  if(maxScroll<0){ maxScroll = 0;}
  if(app->scrollOffset > maxScroll){app->scrollOffset = maxScroll;}

  for(int i = 0; i<app->count;i++){
    int y= (i*30)+50 - app->scrollOffset;
    Rectangle stateBtn = {20,y,50,20};
    bool statehover = CheckCollisionPointRec(mouse, stateBtn);
    if(statehover &&  IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
      app->todos[i].done = !app->todos[i].done;
      saveTodos(app);
      break;
    }
  }
  for(int i = 0; i<app->count;i++){
    int y= (i*30)+50 - app->scrollOffset;
    Rectangle DelBtn = {365,y,20,20};
    bool delhover = CheckCollisionPointRec(mouse, DelBtn);
    if(delhover &&  IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
      app->selected =i;
      deleteTodo(app,app->selected);
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

  Rectangle doneBtn = {340,windowHeight -40,50,20};
  bool donehover = CheckCollisionPointRec(mouse, doneBtn);

  if(donehover &&  IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
    if(app->inputLen >0){
      addTodo(app,app->input);
      saveTodos(app);
      app->inputLen=0;
      app->input[0] = '\0';
    }
  }

}

void render(AppState *app){
  Vector2 mouse = GetMousePosition();
  DrawText("MUDO", 170, 15, 20, WHITE);

  BeginScissorMode(0, 50, windowWidth, windowHeight-110);
  for(int i = 0; i<app->count;i++){
    int y= (i*30)+50 - app->scrollOffset;
    Color c = (i==app->selected) ? PINK :BLACK;
    DrawRectangle(10,y-2, windowWidth-20, 25, WHITE);
    DrawText(app->todos[i].done? "DONE" : "TODO", 20, y+2, 18, c);
    DrawText(app->todos[i].text, 80, y+2, 18, c);
    if( app->todos[i].done){
      DrawRectangle(80,y+8, windowWidth-130, 2, RED);
    }
    Rectangle DelBtn = {365,y+2,20,18};
    bool delhover = CheckCollisionPointRec(mouse, DelBtn);
    DrawRectangleRec(DelBtn, delhover? GRAY : BLACK);
    DrawText("X", DelBtn.x+4, DelBtn.y+2, 17,  WHITE);

  }
  EndScissorMode();

  DrawRectangle(0, windowHeight-60, windowWidth, 80, WHITE);
  if (app->inputLen<=0) {
    DrawText("enter todo...", 30, windowHeight-40, 18, GRAY);
  }

  DrawText(app->input, 30, windowHeight-40, 18, BLACK);

  Rectangle doneBtn = {340,windowHeight -40,50,20};
  bool donehover = CheckCollisionPointRec(mouse, doneBtn);
  DrawRectangleRec(doneBtn, donehover? GRAY : BLACK);
  DrawText("done", doneBtn.x+4, doneBtn.y+2, 18,WHITE);
}

int main() {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(windowWidth, windowHeight, "MUDO");
  SetTargetFPS(60);

  AppState app = {0};
  app.selected = -1;
  app.scrollOffset = 0;

  loadTodos(&app);

  while (!WindowShouldClose()) {
    handleInput(&app);
    BeginDrawing();
    ClearBackground(BLACK);
    render(&app);
    EndDrawing();
  }
  for(int i=0;i< app.count;i++){
    free(app.todos[i].text);
  }
  free(app.todos);
  CloseWindow();
}
