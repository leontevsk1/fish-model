#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define XQ 100
#define YQ 100

// Параметры среды и видов
#define NK 3    // Регенерация корма
#define FB 10   // Размножение рыб
#define FH 20   // Лимит голода рыб (модификация: рыбы теперь могут умереть)
#define SB 45   // Размножение акул
#define SH 15   // Лимит голода акул
#define GQ 3000 // Увеличим число поколений, чтобы увидеть затухание

typedef enum {
    SCENARIO_STABLE_FOCUS,  // Сосуществование (затухающие колебания к P*)
    SCENARIO_PREY_BALANCE,  // Акулы вымирают, рыбы и корм в балансе (P3)
    SCENARIO_RESOURCE_ONLY, // Все вымирают, остается только корм (P2)
    SCENARIO_COUNT
} ScenarioType;

typedef struct {
    char nm;    
    int bc;     
    int hc;     
    int flag;   
    int food;   
} cell;

cell sea[YQ][XQ];
int fqnt = 0, sqnt = 0, total_food = 0;

void step(int y, int x, int g) {
    if (sea[y][x].flag == g || sea[y][x].nm == 'w') return;

    sea[y][x].flag = g;
    sea[y][x].bc++;
    sea[y][x].hc++;

    char type = sea[y][x].nm;
    int tr_pri[4][2], tq_pri = 0;
    int tr_sec[4][2], tq_sec = 0;
    int dy[] = {-1, 1, 0, 0}, dx[] = {0, 0, -1, 1};

    for (int i = 0; i < 4; i++) {
        int ny = (y + dy[i] + YQ) % YQ;
        int nx = (x + dx[i] + XQ) % XQ;
        if (type == 'f') {
            if (sea[ny][nx].nm == 'w') {
                if (sea[ny][nx].food == 1) {
                    tr_pri[tq_pri][0] = ny; tr_pri[tq_pri++][1] = nx;
                } else {
                    tr_sec[tq_sec][0] = ny; tr_sec[tq_sec++][1] = nx;
                }
            }
        } else if (type == 's') {
            if (sea[ny][nx].nm == 'f') {
                tr_pri[tq_pri][0] = ny; tr_pri[tq_pri++][1] = nx;
            } else if (sea[ny][nx].nm == 'w') {
                tr_sec[tq_sec][0] = ny; tr_sec[tq_sec++][1] = nx;
            }
        }
    }

    int limit_h = (type == 'f') ? FH : SH;
    if (sea[y][x].hc >= limit_h) {
        sea[y][x] = (cell){'w', 0, 0, 0, 0};
        if (type == 'f') fqnt--; else sqnt--;
        return;
    }

    int moved = 0, ny = y, nx = x;
    if (tq_pri > 0) {
        int r = rand() % tq_pri;
        ny = tr_pri[r][0]; nx = tr_pri[r][1];
        if (type == 'f') { sea[ny][nx].food = 0; total_food--; }
        else if (type == 's') { fqnt--; }
        sea[y][x].hc = 0;
        sea[ny][nx] = sea[y][x];
        sea[y][x] = (cell){'w', 0, 0, 0, 0};
        moved = 1;
    } else if (tq_sec > 0) {
        int r = rand() % tq_sec;
        ny = tr_sec[r][0]; nx = tr_sec[r][1];
        int keep_food = sea[ny][nx].food; 
        sea[ny][nx] = sea[y][x];
        sea[y][x] = (cell){'w', 0, 0, 0, keep_food};
        moved = 1;
    }

    int limit_b = (type == 'f') ? FB : SB;
    if (moved && sea[ny][nx].bc >= limit_b) {
        sea[ny][nx].bc = 0;
        
        // ПРОВЕРКА: Если в клетке, где рождается потомок, была еда — вычитаем её из счетчика
        if (sea[y][x].food == 1) {
            total_food--;
        }

        sea[y][x] = (cell){type, 0, 0, g, 0}; 
        if (type == 'f') fqnt++; else sqnt++;
    }
}

void init_scenario(ScenarioType type, int seed) {
    srand(seed);
    fqnt = sqnt = total_food = 0;
    double df = 0, ds = 0, dr = 0;

    switch(type) {
        case SCENARIO_STABLE_FOCUS: df = 0.12; ds = 0.04; dr = 0.40; break;
        case SCENARIO_PREY_BALANCE: df = 0.10; ds = 0.002; dr = 0.30; break;
        case SCENARIO_RESOURCE_ONLY: df = 0.001; ds = 0.05; dr = 0.10; break;
        default: break;
    }

    for (int y = 0; y < YQ; y++) {
        for (int x = 0; x < XQ; x++) {
            double r = rand() / (double)RAND_MAX;
            if (r < df) {
                sea[y][x] = (cell){'f', rand() % FB, rand() % FH, 0, 0}; fqnt++;
            } else if (r < df + ds) {
                sea[y][x] = (cell){'s', rand() % SB, rand() % SH, 0, 0}; sqnt++;
            } else {
                int has_food = (rand() / (double)RAND_MAX < dr) ? 1 : 0;
                sea[y][x] = (cell){'w', 0, 0, 0, has_food};
                if (has_food) total_food++;
            }
        }
    }
}

int main() {
    const char *names[] = {"stable_focus", "prey_balance", "resource_only"};

    for (int s = 0; s < SCENARIO_COUNT; s++) {
        char filename[64];
        sprintf(filename, "results_mod_%s.csv", names[s]);
        FILE *f = fopen(filename, "w");
        fprintf(f, "t,fish,sharks,food\n");

        init_scenario((ScenarioType)s, 42);
        printf("Running Modified Scenario: %s...\n", names[s]);

        for (int g = 0; g <= GQ && (fqnt + sqnt) > 0; g++) {
            if (g % NK == 0) {
                for (int y = 0; y < YQ; y++)
                    for (int x = 0; x < XQ; x++)
                        if (sea[y][x].nm == 'w' && sea[y][x].food == 0) {
                            sea[y][x].food = 1; total_food++;
                        }
            }
            fprintf(f, "%d,%d,%d,%d\n", g, fqnt, sqnt, total_food);
            for (int y = 0; y < YQ; y++)
                for (int x = 0; x < XQ; x++)
                    step(y, x, g + 1);
        }
        fclose(f);
    }
    return 0;
}
