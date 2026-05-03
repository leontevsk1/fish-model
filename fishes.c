#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define XQ 100
#define YQ 100
#define FB 10
#define SB 50
#define SH 15
#define GQ 2000

typedef struct {
    char nm;   // 'w', 'f', 's'
    int bc, hc, flag;
} cell;

cell sea[YQ][XQ];
int fqnt, sqnt;

void step(int y, int x, int g) {
    if (sea[y][x].flag == g || sea[y][x].nm == 'w') return;
    int tr[4][2], tq = 0, dy[] = {-1, 1, 0, 0}, dx[] = {0, 0, -1, 1};
    sea[y][x].flag = g; sea[y][x].bc++;
    char target = (sea[y][x].nm == 's') ? 'f' : 'w';
    
    // Поиск целей
    for (int i = 0; i < 4; i++) {
        int ny = (y + dy[i] + YQ) % YQ, nx = (x + dx[i] + XQ) % XQ;
        if (sea[ny][nx].nm == target) { tr[tq][0] = ny; tr[tq++][1] = nx; }
    }
    
    // Если акула не нашла рыбу, ищет воду
    if (sea[y][x].nm == 's' && tq == 0) {
        sea[y][x].hc++;
        for (int i = 0; i < 4; i++) {
            int ny = (y + dy[i] + YQ) % YQ, nx = (x + dx[i] + XQ) % XQ;
            if (sea[ny][nx].nm == 'w') { tr[tq][0] = ny; tr[tq++][1] = nx; }
        }
    } else if (sea[y][x].nm == 's') sea[y][x].hc = 0;

    if (sea[y][x].nm == 's' && sea[y][x].hc >= SH) { sea[y][x] = (cell){'w', 0, 0, 0}; sqnt--; return; }

    if (tq > 0) {
        int r = rand() % tq, ny = tr[r][0], nx = tr[r][1];
        if (sea[ny][nx].nm == 'f') fqnt--;
        char type = sea[y][x].nm;
        int limit = (type == 'f') ? FB : SB;
        sea[ny][nx] = sea[y][x];
        if (sea[ny][nx].bc >= limit) {
            sea[ny][nx].bc = 0; sea[y][x] = (cell){type, 0, 0, g};
            if (type == 'f') fqnt++; else sqnt++;
        } else sea[y][x] = (cell){'w', 0, 0, 0};
    }
}

int main() {
    srand(time(NULL));
    FILE *f = fopen("results.csv", "w");
    fprintf(f, "t,fish,sharks\n");
    for (int y = 0; y < YQ; y++) for (int x = 0; x < XQ; x++) {
        double r = rand() / (double)RAND_MAX;
        if (r < 0.1) { sea[y][x] = (cell){'f', 0, 0, 0}; fqnt++; }
        else if (r < 0.02) { sea[y][x] = (cell){'s', 0, 0, 0}; sqnt++; }
        else sea[y][x] = (cell){'w', 0, 0, 0};
    }
    for (int g = 1; g <= GQ && (fqnt + sqnt) > 0; g++) {
        fprintf(f, "%d,%d,%d\n", g, fqnt, sqnt);
        for (int y = 0; y < YQ; y++) for (int x = 0; x < XQ; x++) step(y, x, g);
    }
    fclose(f);
    return 0;
}
