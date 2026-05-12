#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

/* Параметры поля */
#define XQ 100
#define YQ 100
#define GQ 2000 // Максимум поколений

/* Параметры корма */
#define NK 5    // Регенерация (соотв. емкости среды K)

/* Параметры Видов */
#define NB1 10  // Рождаемость Вид 1
#define NH1 18  // Голод Вид 1
#define NB2 12  // Рождаемость Вид 2
#define NH2 20// Голод Вид 2

typedef enum {
    SCENARIO_GAUSE_1,   // Доминирование Вида 1
    SCENARIO_GAUSE_2,   // Доминирование Вида 2
    SCENARIO_P4_SADDLE, // Попытка удержания в седловой точке (сосуществование)
    SCENARIO_COUNT
} ScenarioType;

typedef struct {
    char nm;    // 'w', '1', '2'
    int bc;     // Возраст
    int hc;     // Голод
    int flag;   // Метка поколения
    int food;   // Наличие корма
} cell;

cell sea[YQ][XQ];
int n1 = 0, n2 = 0, total_food = 0;

void step(int y, int x, int g) {
    if (sea[y][x].flag == g || sea[y][x].nm == 'w') return;

    sea[y][x].flag = g;
    sea[y][x].bc++;
    sea[y][x].hc++;

    char type = sea[y][x].nm;
    int f_tr[4][2], f_tq = 0;
    int w_tr[4][2], w_tq = 0;
    int dy[] = {-1, 1, 0, 0}, dx[] = {0, 0, -1, 1};

    for (int i = 0; i < 4; i++) {
        int ny = (y + dy[i] + YQ) % YQ;
        int nx = (x + dx[i] + XQ) % XQ;
        if (sea[ny][nx].nm == 'w') {
            w_tr[w_tq][0] = ny; w_tr[w_tq++][1] = nx;
            if (sea[ny][nx].food == 1) {
                f_tr[f_tq][0] = ny; f_tr[f_tq++][1] = nx;
            }
        }
    }

    int moved = 0, old_y = y, old_x = x;

    if (type == '1') {
        if (f_tq > 0) {
            int r = rand() % f_tq;
            int ny = f_tr[r][0], nx = f_tr[r][1];
            sea[ny][nx].food = 0; total_food--;
            sea[y][x].hc = 0;
            sea[ny][nx] = sea[y][x];
            sea[y][x] = (cell){'w', 0, 0, 0, 0};
            moved = 1; y = ny; x = nx;
        } else if (w_tq > 0) {
            int r = rand() % w_tq;
            int ny = w_tr[r][0], nx = w_tr[r][1];
            sea[ny][nx] = sea[y][x];
            sea[y][x] = (cell){'w', 0, 0, 0, 0};
            moved = 1; y = ny; x = nx;
        }
    } else if (type == '2') {
        if (f_tq > 0) {
            int r = rand() % f_tq;
            sea[f_tr[r][0]][f_tr[r][1]].food = 0; total_food--;
            sea[y][x].hc = 0; 
        } else if (w_tq > 0) {
            int r = rand() % w_tq;
            int ny = w_tr[r][0], nx = w_tr[r][1];
            sea[ny][nx] = sea[y][x];
            sea[y][x] = (cell){'w', 0, 0, 0, 0};
            moved = 1; y = ny; x = nx;
        }
    }

    int limit_h = (type == '1') ? NH1 : NH2;
    if (sea[y][x].hc >= limit_h) {
        sea[y][x] = (cell){'w', 0, 0, 0, 0};
        if (type == '1') n1--; else n2--;
        return;
    }

    int limit_b = (type == '1') ? NB1 : NB2;
    if (sea[y][x].bc >= limit_b) {
        if (moved) {
            sea[old_y][old_x] = (cell){type, 0, 0, g, 0};
            sea[y][x].bc = 0;
            if (type == '1') n1++; else n2++;
        }
    }
}

void init_scenario(ScenarioType type, int seed) {
    srand(seed);
    n1 = 0; n2 = 0; total_food = 0;
    
    double d1 = 0, d2 = 0;
    switch(type) {
        case SCENARIO_GAUSE_1: d1 = 0.15; d2 = 0.02; break; // Преимущество вида 1
        case SCENARIO_GAUSE_2: d1 = 0.02; d2 = 0.15; break; // Преимущество вида 2
        case SCENARIO_P4_SADDLE: d1 = 0.08; d2 = 0.08; break; // Начальный баланс
    }

    for (int y = 0; y < YQ; y++) {
        for (int x = 0; x < XQ; x++) {
            double r = rand() / (double)RAND_MAX;
            if (r < d1) { sea[y][x] = (cell){'1', rand() % NB1, 0, 0, 0}; n1++; }
            else if (r < d1 + d2) { sea[y][x] = (cell){'2', rand() % NB2, 0, 0, 0}; n2++; }
            else { sea[y][x] = (cell){'w', 0, 0, 0, 0}; }
        }
    }
}

int main() {
    const char *scen_names[] = {"exclusion_v1", "exclusion_v2", "saddle_p4"};
    
    for (int s = 0; s < SCENARIO_COUNT; s++) {
        char filename[64];
        sprintf(filename, "results_var2_%s.csv", scen_names[s]);
        FILE *f = fopen(filename, "w");
        fprintf(f, "t,sp1,sp2,food\n");

        init_scenario((ScenarioType)s, 42);
        printf("Running Scenario: %s\n", scen_names[s]);

        for (int g = 0; g <= GQ && (n1 + n2) > 0; g++) {
            if (g % NK == 0) {
                for (int y = 0; y < YQ; y++)
                    for (int x = 0; x < XQ; x++)
                        if (sea[y][x].nm == 'w' && sea[y][x].food == 0) {
                            sea[y][x].food = 1; total_food++;
                        }
            }
            fprintf(f, "%d,%d,%d,%d\n", g, n1, n2, total_food);
            for (int y = 0; y < YQ; y++)
                for (int x = 0; x < XQ; x++)
                    step(y, x, g + 1);
        }
        fclose(f);
    }
    return 0;
}
