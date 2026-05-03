#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include "sdl_fb.h"
#ifndef XQ
#define XQ 200
#endif
#ifndef YQ
#define YQ 200
#endif
#ifndef SIZE
#define SIZE 4
#endif
#define FB 10 //до размножения рыб
#define SB 55 //до размножения акул
#define SH 40 //сколько акула живет без еды
#define GQ 5000 //максимальное количество поколений

#define RED 0xFF0000 //#RRGGBB
#define GREEN 0x00FF00
#define BLUE  0x0000FF
#define WHITE 0xFFFFFF
#define BLACK 0x000000

typedef struct {
  char nm;
  uint8_t hc, bc;
  uint32_t flag;
} cell; //Структура для особи 

int fqnt, sqnt;

//Подсчет соседей клетки и заполнение массива рядом лежащих 
int nbr (cell sea[YQ][XQ], int y, int x, char tn, int target[4][2]) {
  int tq = 0;// target[n][0] -- y,  target[n][1] -- x
  int dx, dy, c;
  for(c = 0; c < 4; c++) {
    dx = ((c / 2) * 2 - 1) * (1 - c%2);
    dy = (1 - (c / 2) * 2) * (c%2);
    if(sea[(y+dy+YQ)%YQ][(x+dx+XQ)%XQ].nm == tn) {
      target[tq][1] = (x+dx+XQ)%XQ;
      target[tq++][0] = (y+dy+YQ)%YQ;
    }
  }
  for(c = tq, target[tq][0]=-1, target[tq][1]=-1; c < 4; c++, target[c][0]=-1, target[c][1]=-1);
  return tq;
}

//ход
int step (cell sea[YQ][XQ], int y, int x, int generation) {
  int tq, fq, tr[4][2], r, xn, yn;
  if((sea[y][x].nm == 'f') && (sea[y][x].flag <= generation)) {//Рыба 
    sea[y][x].bc++;
    tq = nbr(sea,y,x,'w',tr);//Подсчет свободных
    if(tq != 0) {
      r = rand()%tq;//Выбор случайной клетки
      yn = tr[r][0]; xn = tr[r][1];
      sea[yn][xn] = sea[y][x];
      sea[yn][xn].flag = generation + 1;//Запись флага о ходе
      if(sea[yn][xn].bc > FB){ //Проверка на размножение
	sea[yn][xn].bc = rand()%3;
      	sea[y][x] = (cell){.nm='f',.bc=0,.hc=0,.flag=generation+1};//Размножение
	fqnt++;
      }else
	sea[y][x] = (cell){.nm='w',.bc=0,.hc=0,.flag=0};//Обнуление
    }else if(sea[y][x].bc > FB)
      sea[y][x].bc = 0;
  } else if (sea[y][x].nm == 's' && sea[y][x].flag <= generation) {
    sea[y][x].bc++;//Становится старше
    sea[y][x].hc++;//Голоднее
    if(sea[y][x].hc > SH) {
      sea[y][x] = (cell){.nm='w',.bc=0,.hc=0,.flag=0};
      sqnt--;
      return 0;
    }
    fq = nbr(sea,y,x,'f',tr);//Подсчет рыб
    if(fq != 0) {
      sea[y][x].hc = 0;
      fqnt--;
      r = rand()%fq;//Выбор случайной клетки с рыбой
      yn = tr[r][0]; xn = tr[r][1];
      sea[yn][xn] = sea[y][x];
      sea[yn][xn].flag = generation + 1;//Запись флага о ходе
      if(sea[yn][xn].bc > SB){ //Проверка на размножение
	sea[yn][xn].bc = rand()%3;
	sqnt++;
      	sea[y][x] = (cell){.nm='s',.bc=0,.hc=0,.flag=generation+1};//Размножение
      }else{
	sea[y][x] = (cell){.nm='w',.bc=0,.hc=0,.flag=0};//Обнуление
      }
      return 0;
    }
    tq = nbr(sea,y,x,'w',tr);//Подсчет свободных
    if(tq != 0) {
      r = rand()%tq;//Выбор случайной клетки с рыбой
      yn = tr[r][0]; xn = tr[r][1];
      sea[yn][xn] = sea[y][x];
      sea[yn][xn].flag = generation + 1;//Запись флага о ходе
      if(sea[yn][xn].bc > SB){ //Проверка на размножение
	sea[yn][xn].bc = rand()%3;
	sqnt++;
      	sea[y][x] = (cell){.nm='s',.bc=0,.hc=0,.flag=generation+1};//Размножение
      }else
	sea[y][x] = (cell){.nm='w',.bc=0,.hc=0,.flag=0};//Обнуление
      return 0;
    }else if(sea[y][x].bc > SB)
      sea[y][x].bc = 0;
  }
  return 0;
}

//Обновление массива для фреймбуфера
void update_field(cell sea[YQ][XQ], uint32_t* color_field, uint32_t w_c, uint32_t f_c, uint32_t s_c) {
  for(size_t cy = 0; cy < YQ; cy++) {
    for(size_t cx = 0; cx < XQ; cx++) {
      switch(sea[cy][cx].nm) {
      case 'w': {
	color_field[cy * XQ + cx] = w_c;
	break;
      }
      case 'f': {
	color_field[cy * XQ + cx] = f_c;
	break;
      }
      case 's': {
	color_field[cy * XQ + cx] = s_c;
	break;
      }
      }
    }
  }
}

//Задание начальной конфигурации
void field_init(cell sea[YQ][XQ], double psb_fish, double psb_shark) {
  double r;
  // Заполнение
  for(int cy = 0; cy < YQ; cy++) {
    for (int cx = 0; cx < XQ; cx++) {
      r = rand()/(RAND_MAX + 1.0);
      if(r < psb_fish) {
	sea[cy][cx] = (cell){.nm='f',.bc=rand()%FB,.hc=0,.flag=0};
	fqnt++;
      } else if(r < psb_fish + psb_shark) {
	sea[cy][cx] = (cell){.nm='s',.bc=rand()%SB,.hc=rand()%SH,.flag=0};
	sqnt++;
      } else {
	sea[cy][cx] = (cell){.nm='w',.bc=0,.hc=0,.flag=0};
      }
    }
  }  
}

//Затравка случайного числа
void random_init() {
  int fd = open("/dev/urandom", O_RDONLY);
  int seed;
  read(fd, &seed, sizeof(seed));
  srand(seed);
}

//Ожидание
void sleep_ms(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

int main() {
  random_init();

  double fp = 0.03, sp = 0.005 + fp;
  cell (*sea)[XQ] = malloc(sizeof(cell) * XQ * YQ);
  field_init(sea, fp, sp);
  
  uint32_t color_map[YQ][XQ] = {0};
  sdl_fb_init(XQ, YQ, SIZE, "Some title", color_map);
  int stop = 0, delay_ms = 1000;
  
  //Вывод
  for(int c = 0; c < GQ && fqnt + sqnt > 0; c++) {
    update_field(sea, (uint32_t *)color_map, BLACK, GREEN, RED);
    sdl_fb_update();
    sdl_fb_handle_input(&stop, &delay_ms);
    if(stop == 2) break;
    sleep_ms(delay_ms);
    //    printf("%i %i %i\n", c, fqnt, sqnt);
    if(!stop)
    for (int cy = 0; cy < YQ; cy++) 
      for(int cx = 0; cx < XQ; cx++) {
	step(sea,cy,cx,c);
      }
  }
  sdl_fb_cleanup();
  free(sea);
  return 0;
}
