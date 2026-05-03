#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#define XQ 100
#define YQ 100
#define FB 10   // Порог размножения рыб
#define SB 50   // Порог размножения акул
#define SH 15   // Лимит голодания акул
#define GQ 2000 // Максимум поколений

typedef struct {
    char nm;    // 'w' - вода, 'f' - рыба, 's' - акула
    int bc;     // Счетчик возраста (размножение)
    int hc;     // Счетчик голода (только для акул)
    int flag;   // Номер текущего поколения (защита от двойного хода)
} cell;

cell sea[YQ][XQ];
int fqnt = 0;
int sqnt = 0;

void step(int y, int x, int g) {
    // Если ячейка уже ходила в этом поколении или это вода — выходим
    if (sea[y][x].flag == g || sea[y][x].nm == 'w') {
        return;
    }

    int tr[4][2]; 
    int tq = 0;
    int dy[] = {-1, 1, 0, 0};
    int dx[] = {0, 0, -1, 1};

    // Фиксируем ход и инкрементируем возраст
    sea[y][x].flag = g;
    sea[y][x].bc++;

    char primary_target = (sea[y][x].nm == 's') ? 'f' : 'w';
    
    // Поиск основной цели (рыба для акулы, вода для рыбы)
    for (int i = 0; i < 4; i++) {
        int ny = (y + dy[i] + YQ) % YQ;
        int nx = (x + dx[i] + XQ) % XQ;
        
        if (sea[ny][nx].nm == primary_target) {
            tr[tq][0] = ny;
            tr[tq][1] = nx;
            tq++;
        }
    }
    
    // Если акула не нашла еду, она ищет воду для перемещения
    if (sea[y][x].nm == 's' && tq == 0) {
        sea[y][x].hc++; // Увеличиваем голод
        for (int i = 0; i < 4; i++) {
            int ny = (y + dy[i] + YQ) % YQ;
            int nx = (x + dx[i] + XQ) % XQ;
            
            if (sea[ny][nx].nm == 'w') {
                tr[tq][0] = ny;
                tr[tq][1] = nx;
                tq++;
            }
        }
    } else if (sea[y][x].nm == 's') {
        // Если акула нашла еду, голод сбрасывается
        sea[y][x].hc = 0;
    }

    // Смерть акулы от голода
    if (sea[y][x].nm == 's' && sea[y][x].hc >= SH) {
        sea[y][x] = (cell){'w', 0, 0, 0};
        sqnt--;
        return;
    }

    // Логика перемещения и размножения
    if (tq > 0) {
        int r = rand() % tq;
        int ny = tr[r][0];
        int nx = tr[r][1];

        // Если на целевой клетке была рыба — она съедена
        if (sea[ny][nx].nm == 'f') {
            fqnt--;
        }

        char type = sea[y][x].nm;
        int reproduction_limit = (type == 'f') ? FB : SB;

        // Перемещаем особь в новую ячейку
        sea[ny][nx] = sea[y][x];

        if (sea[ny][nx].bc >= reproduction_limit) {
            // Размножение: обнуляем возраст и оставляем потомка в старой ячейке
            sea[ny][nx].bc = 0;
            sea[y][x] = (cell){type, 0, 0, g};
            
            if (type == 'f') {
                fqnt++;
            } else {
                sqnt++;
            }
        } else {
            // Обычное перемещение: старая ячейка становится водой
            sea[y][x] = (cell){'w', 0, 0, 0};
        }
    }
}

int main() {
    srand(time(NULL));
    FILE *f = fopen("results.csv", "w");
    if (f == NULL) {
        return 1;
    }
    
    fprintf(f, "t,fish,sharks\n");

    // Инициализация поля с рандомным распределением и случайным возрастом
    for (int y = 0; y < YQ; y++) {
        for (int x = 0; x < XQ; x++) {
            double r = rand() / (double)RAND_MAX;
            
            if (r < 0.10) { 
                // Создаем рыбу со случайным начальным возрастом (от 0 до FB-1)
                sea[y][x] = (cell){'f', rand() % FB, 0, 0}; 
                fqnt++; 
            } else if (r < 0.12) { 
                // Создаем акулу со случайным возрастом и голодом
                sea[y][x] = (cell){'s', rand() % SB, rand() % SH, 0}; 
                sqnt++; 
            } else {
                sea[y][x] = (cell){'w', 0, 0, 0};
            }
        }
    }

    // Основной цикл симуляции
    for (int g = 0; g <= GQ && (fqnt + sqnt) > 0; g++) {
        // Записываем текущее состояние ПЕРЕД ходом (начиная с поколения 0)
        fprintf(f, "%d,%d,%d\n", g, fqnt, sqnt);

        // Обход поля
        for (int y = 0; y < YQ; y++) {
            for (int x = 0; x < XQ; x++) {
                step(y, x, g + 1); // g+1 чтобы флаг ячейки был всегда актуальным
            }
        }
    }

    fclose(f);
    return 0;
}
