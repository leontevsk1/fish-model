#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

/* Параметры системы */
#define XQ 500
#define YQ 500
#define FB 10   // Порог размножения рыб
#define SB 50   // Порог размножения акул
#define SH 15   // Лимит голодания акул
#define GQ 3000 // Максимум поколений

/* Типы сценариев для верификации аналитики */
typedef enum {
    SCENARIO_CENTER,    // Устойчивые циклы (вокруг точки P2)
    SCENARIO_SADDLE,    // Коллапс (уход в P1 по сепаратрисе)
    SCENARIO_MONOPOLY,  // Вытеснение хищников
    SCENARIO_COUNT
} ScenarioType;

typedef struct {
    char nm;    // 'w' - вода, 'f' - рыба, 's' - акула
    int bc;     // Счетчик возраста (размножение)
    int hc;     // Счетчик голода (только для акул)
    int flag;   // Защита от двойного хода
} cell;

/* Глобальное состояние */
cell sea[YQ][XQ];
int fqnt = 0;
int sqnt = 0;

/* Логика поведения отдельной особи */
void step(int y, int x, int g) {
    if (sea[y][x].flag == g || sea[y][x].nm == 'w') {
        return;
    }

    int tr[4][2]; 
    int tq = 0;
    int dy[] = {-1, 1, 0, 0};
    int dx[] = {0, 0, -1, 1};

    sea[y][x].flag = g;
    sea[y][x].bc++;

    char primary_target = (sea[y][x].nm == 's') ? 'f' : 'w';
    
    // Поиск цели (еда или свободное пространство)
    for (int i = 0; i < 4; i++) {
        int ny = (y + dy[i] + YQ) % YQ;
        int nx = (x + dx[i] + XQ) % XQ;
        if (sea[ny][nx].nm == primary_target) {
            tr[tq][0] = ny;
            tr[tq][1] = nx;
            tq++;
        }
    }
    
    // Дополнительный поиск для акул, если нет еды
    if (sea[y][x].nm == 's' && tq == 0) {
        sea[y][x].hc++; 
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
        sea[y][x].hc = 0;
    }

    // Смерть от голода
    if (sea[y][x].nm == 's' && sea[y][x].hc >= SH) {
        sea[y][x] = (cell){'w', 0, 0, 0};
        sqnt--;
        return;
    }

    // Перемещение и размножение
    if (tq > 0) {
        int r = rand() % tq;
        int ny = tr[r][0];
        int nx = tr[r][1];

        if (sea[ny][nx].nm == 'f') {
            fqnt--;
        }

        char type = sea[y][x].nm;
        int reproduction_limit = (type == 'f') ? FB : SB;

        sea[ny][nx] = sea[y][x];

        if (sea[ny][nx].bc >= reproduction_limit) {
            sea[ny][nx].bc = 0;
            sea[y][x] = (cell){type, 0, 0, g};
            if (type == 'f') fqnt++; else sqnt++;
        } else {
            sea[y][x] = (cell){'w', 0, 0, 0};
        }
    }
}

/* Инициализация расчетных начальных условий */
void init_scenario(ScenarioType type, int seed) {
    srand(seed);
    fqnt = 0;
    sqnt = 0;

    double fish_density = 0;
    double shark_density = 0;

    switch (type) {
        case SCENARIO_CENTER:
            fish_density = 0.15; 
            shark_density = 0.05;
            break;
        case SCENARIO_SADDLE:
            fish_density = 0.02;
            shark_density = 0.10;
            break;
        case SCENARIO_MONOPOLY:
            fish_density = 0.10;
            shark_density = 0.005;
            break;
        default: break;
    }

    for (int y = 0; y < YQ; y++) {
        for (int x = 0; x < XQ; x++) {
            double r = (double)rand() / RAND_MAX;
            if (r < fish_density) {
                sea[y][x] = (cell){'f', rand() % FB, 0, 0};
                fqnt++;
            } else if (r < fish_density + shark_density) {
                sea[y][x] = (cell){'s', rand() % SB, rand() % SH, 0};
                sqnt++;
            } else {
                sea[y][x] = (cell){'w', 0, 0, 0};
            }
        }
    }
}

int main() {
    const char *scenario_names[] = {"center", "saddle", "monopoly"};
    
    printf("Wa-Tor Research Suite: Starting Scenario Analysis...\n");

    for (int s = 0; s < SCENARIO_COUNT; s++) {
        char filename[64];
        sprintf(filename, "results_base_%s.csv", scenario_names[s]);
        
        FILE *f = fopen(filename, "w");
        if (!f) {
            fprintf(stderr, "Critical error: Cannot open %s for writing.\n", filename);
            continue;
        }

        // Фиксируем сид 42 для обеспечения повторяемости экспериментов в отчете
        init_scenario((ScenarioType)s, 42); 
        fprintf(f, "t,fish,sharks\n");

        printf("Running Scenario: %s... ", scenario_names[s]);
        fflush(stdout);

        for (int g = 0; g <= GQ && (fqnt + sqnt) > 0; g++) {
            fprintf(f, "%d,%d,%d\n", g, fqnt, sqnt);
            for (int y = 0; y < YQ; y++) {
                for (int x = 0; x < XQ; x++) {
                    step(y, x, g + 1);
                }
            }
        }

        fclose(f);
        printf("Done. Data saved to %s\n", filename);
    }

    printf("Analysis complete. All CSV files generated.\n");
    return 0;
}
