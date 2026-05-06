# Лабораторная работа номер 2

## Структура работа

Репозиторий имеет 3 ветки:

- main - реализация базовой модели Wa-Tor,
- v2 - реализация модели с двумя видами конкурирующих за одну кормовую базу,
- modifications - модель Wa-Tor с модификациями описанными в методичке.

## Инструкция по запуску

```bash
git clone https://github.com/leontevs1/fish.git
cd fish

gcc -o main fish.c
./main
gnuplot plot.gp

git checkout v2
gcc -o v2 fish.c
./v2
gnuplot plot.gp

git checkout modifications
gcc -o modifications fish.c
./modifications
gnuplot plot.gp
```

Результаты выполнения программы сохраняются в csv файл, резуальтаты визуализируются с помощью gnuplot.
