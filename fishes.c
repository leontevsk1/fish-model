#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define XQ 100
#define YQ 100

// Параметры среды и видов
#define NK 3    // Водоросли растут каждые NK шагов
#define FB 10   // Шагов до размножения рыб
#define FH 20   // Лимит голода рыб
#define SB 45   // Шагов до размножения акул
#define SH 15   // Лимит голода акул
#define GQ 2000 // Максимум поколений

typedef struct {
    char nm;    // 'w' - вода, 'f' - рыба, 's' - акула
    int bc;     // Возраст (размножение)
    int hc;     // Голод (для рыб и акул)
    int flag;   // Метка поколения
    int food;   // 1 - есть корм (только для 'w'), 0 - нет
} cell;

cell sea[YQ][XQ];
int fqnt = 0, sqnt = 0, total_food = 0;

void step(int y, int x, int g) {
    if (sea[y][x].flag == g || sea[y][x].nm == 'w') return;

    sea[y][x].flag = g;
    sea[y][x].bc++;
    sea[y][x].hc++;

    char type = sea[y][x].nm;
    int tr_pri[4][2], tq_pri = 0; // Приоритетные цели (Еда для рыбы, Рыба для акулы)
    int tr_sec[4][2], tq_sec = 0; // Вторичные цели (Пустая вода)
    int dy[] = {-1, 1, 0, 0}, dx[] = {0, 0, -1, 1};

    // 1. Оценка окружения
    for (int i = 0; i < 4; i++) {
        int ny = (y + dy[i] + YQ) % YQ;
        int nx = (x + dx[i] + XQ) % XQ;
        
        if (type == 'f') {
            // Рыба ищет воду. Приоритет — вода с кормом.
            if (sea[ny][nx].nm == 'w') {
                if (sea[ny][nx].food == 1) {
                    tr_pri[tq_pri][0] = ny; tr_pri[tq_pri++][1] = nx;
                } else {
                    tr_sec[tq_sec][0] = ny; tr_sec[tq_sec++][1] = nx;
                }
            }
        } else if (type == 's') {
            // Акула ищет рыбу. Приоритет — рыба. Вторично — любая вода.
            if (sea[ny][nx].nm == 'f') {
                tr_pri[tq_pri][0] = ny; tr_pri[tq_pri++][1] = nx;
            } else if (sea[ny][nx].nm == 'w') {
                tr_sec[tq_sec][0] = ny; tr_sec[tq_sec++][1] = nx;
            }
        }
    }

    // 2. Смерть от голода
    int limit_h = (type == 'f') ? FH : SH;
    if (sea[y][x].hc >= limit_h) {
        sea[y][x] = (cell){'w', 0, 0, 0, 0};
        if (type == 'f') fqnt--; else sqnt--;
        return;
    }

    // 3. Логика перемещения и питания
    int moved = 0, ny = y, nx = x;
    
    if (tq_pri > 0) { // Найдена приоритетная цель (Еда/Рыба)
        int r = rand() % tq_pri;
        ny = tr_pri[r][0]; nx = tr_pri[r][1];
        
        if (type == 'f') {
            sea[ny][nx].food = 0; total_food--; // Рыба съела корм
        } else if (type == 's') {
            fqnt--; // Акула съела рыбу
        }
        
        sea[y][x].hc = 0; // Голод сброшен
        sea[ny][nx] = sea[y][x];
        sea[y][x] = (cell){'w', 0, 0, 0, 0};
        moved = 1;
    } else if (tq_sec > 0) { // Найдена вторичная цель (Просто вода)
        int r = rand() % tq_sec;
        ny = tr_sec[r][0]; nx = tr_sec[r][1];
        
        // Перенос флага еды, если акула или рыба наступила на воду с едой, но не съела её
        // (Акула не ест водоросли, рыба на пустой воде еду не трогает)
        int keep_food = sea[ny][nx].food; 
        
        sea[ny][nx] = sea[y][x];
        sea[y][x] = (cell){'w', 0, 0, 0, keep_food}; // Оставляем еду на старом месте, если она там была (для акул)
        moved = 1;
    }

    // 4. Логика размножения
    int limit_b = (type == 'f') ? FB : SB;
    if (moved && sea[ny][nx].bc >= limit_b) {
        sea[ny][nx].bc = 0;
        sea[y][x] = (cell){type, 0, 0, g, 0}; // Потомку еда не достается
        if (type == 'f') fqnt++; else sqnt++;
    }
}

int main() {
    srand(time(NULL));
    FILE *f = fopen("results_mod.csv", "w");
    if (!f) return 1;
    
    fprintf(f, "t,fish,sharks,food\n");

    // Инициализация
    for (int y = 0; y < YQ; y++) {
        for (int x = 0; x < XQ; x++) {
            double r = rand() / (double)RAND_MAX;
            if (r < 0.15) { 
                sea[y][x] = (cell){'f', rand() % FB, rand() % FH, 0, 0}; fqnt++; 
            } else if (r < 0.17) { 
                sea[y][x] = (cell){'s', rand() % SB, rand() % SH, 0, 0}; sqnt++; 
            } else { 
                int has_food = (rand() % 2 == 0) ? 1 : 0; // 50% воды изначально с едой
                sea[y][x] = (cell){'w', 0, 0, 0, has_food};
                if (has_food) total_food++;
            }
        }
    }

    for (int g = 0; g <= GQ && (fqnt + sqnt) > 0; g++) {
        // Рост корма
        if (g % NK == 0) {
            for (int y = 0; y < YQ; y++) {
                for (int x = 0; x < XQ; x++) {
                    if (sea[y][x].nm == 'w' && sea[y][x].food == 0) {
                        sea[y][x].food = 1;
                        total_food++;
                    }
                }
            }
        }

        fprintf(f, "%d,%d,%d,%d\n", g, fqnt, sqnt, total_food);

        for (int y = 0; y < YQ; y++)
            for (int x = 0; x < XQ; x++)
                step(y, x, g + 1);
    }

    fclose(f);
    return 0;
}
