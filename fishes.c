#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define XQ 40
#define YQ 20
#define FB 10 // Шагов до размножения рыб (Mp)
#define SB 50 // Шагов до размножения акул (Ma)
#define SH 15 // Лимит голода акул (Na)
#define GQ 1000 // Максимальное количество поколений

typedef struct {
    char nm;   // 'w' - вода, 'f' - рыба, 's' - акула
    int bc;    // Возраст
    int hc;    // Голод
    int flag;  // Метка текущего хода
} cell;

cell sea[YQ][XQ];
int fqnt = 0, sqnt = 0;

// Поиск соседей заданного типа (тор)
int get_neighbors(int y, int x, char type, int tr[4][2]) {
    int count = 0;
    int dy[] = {-1, 1, 0, 0}, dx[] = {0, 0, -1, 1};
    for (int i = 0; i < 4; i++) {
        int ny = (y + dy[i] + YQ) % YQ;
        int nx = (x + dx[i] + XQ) % XQ;
        if (sea[ny][nx].nm == type) {
            tr[count][0] = ny;
            tr[count][1] = nx;
            count++;
        }
    }
    return count;
}

void step(int y, int x, int gen) {
    if (sea[y][x].flag == gen || sea[y][x].nm == 'w') return;

    int tr[4][2], count;
    sea[y][x].flag = gen;
    sea[y][x].bc++;

    if (sea[y][x].nm == 'f') { // ЛОГИКА РЫБЫ
        count = get_neighbors(y, x, 'w', tr);
        if (count > 0) {
            int r = rand() % count;
            int ny = tr[r][0], nx = tr[r][1];
            sea[ny][nx] = sea[y][x];
            if (sea[ny][nx].bc >= FB) {
                sea[ny][nx].bc = 0;
                sea[y][x] = (cell){'f', 0, 0, gen};
                fqnt++;
            } else {
                sea[y][x] = (cell){'w', 0, 0, 0};
            }
        }
    } else { // ЛОГИКА АКУЛЫ
        sea[y][x].hc++;
        count = get_neighbors(y, x, 'f', tr);
        int eating = (count > 0);
        
        if (!eating) count = get_neighbors(y, x, 'w', tr);
        
        if (sea[y][x].hc >= SH) {
            sea[y][x] = (cell){'w', 0, 0, 0};
            sqnt--;
        } else if (count > 0) {
            int r = rand() % count;
            int ny = tr[r][0], nx = tr[r][1];
            if (eating) fqnt--;
            sea[ny][nx] = sea[y][x];
            if (eating) sea[ny][nx].hc = 0;
            if (sea[ny][nx].bc >= SB) {
                sea[ny][nx].bc = 0;
                sea[y][x] = (cell){'s', 0, 0, gen};
                sqnt++;
            } else {
                sea[y][x] = (cell){'w', 0, 0, 0};
            }
        }
    }
}

int main() {
    srand(time(NULL));
    FILE *log = fopen("population.csv", "w");
    fprintf(log, "Gen,Fish,Sharks\n");

    // Инициализация
    for (int y = 0; y < YQ; y++) {
        for (int x = 0; x < XQ; x++) {
            double r = rand() / (double)RAND_MAX;
            if (r < 0.1) { sea[y][x] = (cell){'f', rand() % FB, 0, 0}; fqnt++; }
            else if (r < 0.15) { sea[y][x] = (cell){'s', rand() % SB, rand() % SH, 0}; sqnt++; }
            else sea[y][x] = (cell){'w', 0, 0, 0};
        }
    }

    for (int g = 1; g <= GQ && (fqnt + sqnt) > 0; g++) {
        // Отрисовка в консоль
        printf("\033[HGen: %d | Fish: %d | Sharks: %d\n", g, fqnt, sqnt);
        for (int y = 0; y < YQ; y++) {
            for (int x = 0; x < XQ; x++) putchar(sea[y][x].nm == 'f' ? 'f' : (sea[y][x].nm == 's' ? 's' : '.'));
            putchar('\n');
        }
        
        fprintf(log, "%d,%d,%d\n", g, fqnt, sqnt);

        for (int y = 0; y < YQ; y++)
            for (int x = 0; x < XQ; x++) step(y, x, g);
    }

    fclose(log);
    return 0;
}
