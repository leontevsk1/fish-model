#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define XQ 100
#define YQ 100

// Параметры корма
#define NK 5    // Шагов до обновления корма (N_korm)

// Параметры Вида 1 (Стратегия: Блуждание)
#define NB1 12  // Шагов до размножения
#define NH1 15  // Лимит голода

// Параметры Вида 2 (Стратегия: Уничтожение)
#define NB2 12
#define NH2 15

#define GQ 2000 // Максимум поколений

typedef struct {
    char nm;    // 'w' - вода, '1' - вид 1, '2' - вид 2
    int bc;     // Возраст
    int hc;     // Голод
    int flag;   // Метка поколения
    int food;   // 1 - есть корм, 0 - нет корма
} cell;

cell sea[YQ][XQ];
int n1 = 0, n2 = 0, total_food = 0;

void step(int y, int x, int g) {
    if (sea[y][x].flag == g || sea[y][x].nm == 'w') return;

    sea[y][x].flag = g;
    sea[y][x].bc++;
    sea[y][x].hc++;

    char type = sea[y][x].nm;
    int f_tr[4][2], f_tq = 0; // Соседи с кормом
    int w_tr[4][2], w_tq = 0; // Свободные соседи (вода)
    int dy[] = {-1, 1, 0, 0}, dx[] = {0, 0, -1, 1};

    // Оценка окружения
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

    // Логика поведения согласно стратегиям
    if (type == '1') {
        // Стратегия 1: "Блуждание" (двигается к еде)
        if (f_tq > 0) {
            int r = rand() % f_tq;
            int ny = f_tr[r][0], nx = f_tr[r][1];
            sea[ny][nx].food = 0; total_food--; // Съедает
            sea[y][x].hc = 0;
            sea[ny][nx] = sea[y][x];            // Перемещается
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
        // Стратегия 2: "Уничтожение" (ест на месте, двигается если нет еды)
        if (f_tq > 0) {
            int r = rand() % f_tq;
            sea[f_tr[r][0]][f_tr[r][1]].food = 0; total_food--; // Съедает из соседней клетки
            sea[y][x].hc = 0; 
            // Остается на месте
        } else if (w_tq > 0) {
            int r = rand() % w_tq;
            int ny = w_tr[r][0], nx = w_tr[r][1];
            sea[ny][nx] = sea[y][x];
            sea[y][x] = (cell){'w', 0, 0, 0, 0};
            moved = 1; y = ny; x = nx;
        }
    }

    // Проверка смерти от голода
    int limit_h = (type == '1') ? NH1 : NH2;
    if (sea[y][x].hc >= limit_h) {
        sea[y][x] = (cell){'w', 0, 0, 0, 0};
        if (type == '1') n1--; else n2--;
        return;
    }

    // Размножение (потомок остается в старой клетке, если особь переместилась)
    int limit_b = (type == '1') ? NB1 : NB2;
    if (sea[y][x].bc >= limit_b) {
        if (moved) {
            sea[old_y][old_x] = (cell){type, 0, 0, g, 0};
            sea[y][x].bc = 0;
            if (type == '1') n1++; else n2++;
        }
        // Если особь не двигалась (Стратегия 2 за едой), она не может оставить потомка в "предыдущей" позиции.
        // Размножение откладывается до следующего движения.
    }
}

int main() {
    srand(time(NULL));
    FILE *f = fopen("results_var2.csv", "w");
    fprintf(f, "t,sp1,sp2,food\n");

    // Инициализация
    for (int y = 0; y < YQ; y++) {
        for (int x = 0; x < XQ; x++) {
            double r = rand() / (double)RAND_MAX;
            if (r < 0.05) { sea[y][x] = (cell){'1', rand() % NB1, 0, 0, 0}; n1++; }
            else if (r < 0.10) { sea[y][x] = (cell){'2', rand() % NB2, 0, 0, 0}; n2++; }
            else { sea[y][x] = (cell){'w', 0, 0, 0, 0}; }
        }
    }

    for (int g = 0; g <= GQ && (n1 + n2) > 0; g++) {
        // Рост корма каждые NK шагов
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

        fprintf(f, "%d,%d,%d,%d\n", g, n1, n2, total_food);

        for (int y = 0; y < YQ; y++)
            for (int x = 0; x < XQ; x++)
                step(y, x, g + 1);
    }

    fclose(f);
    return 0;
}
