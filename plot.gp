# Настройка формата входных данных
set datafile separator ","
set grid

# Стили линий: зеленый для рыб, красный для акул
set style line 1 lc rgb '#00ad4e' lt 1 lw 1.5
set style line 2 lc rgb '#e63320' lt 1 lw 1.5

# Общая настройка терминала
set terminal pngcairo size 800,600 font "Arial,11"

# --- ГРАФИК 1: Сохранение временного ряда ---
set output "base_time_series.png"
set title "Динамика популяций (временной ряд)"
set xlabel "Поколение (t)"
set ylabel "Численность особей (N)"
set key top right

plot "results.csv" using 1:2 with lines ls 1 title "Рыбы (N_f)", \
     "results.csv" using 1:3 with lines ls 2 title "Акулы (N_s)"

# --- ГРАФИК 2: Сохранение фазового портрета ---
# Смена выходного файла
set output "base_phase_portrait.png"
set title "Фазовый портрет системы"
set xlabel "Численность рыб (N_f)"
set ylabel "Численность акул (N_s)"
# Для фазового портрета легенда обычно избыточна
unset key

# Траектория в пространстве состояний (N_f, N_s)
plot "results.csv" using 2:3 with lines lc rgb "#444444" lw 1.2


# 1. СОБИРАЕМ СТАТИСТИКУ (поиск максимумов)
# Gnuplot просканирует столбцы и сохранит переменные F_max и S_max
stats "results.csv" using 2 name "F" nooutput
stats "results.csv" using 3 name "S" nooutput

# Настройка терминала и файла
set terminal pngcairo size 800,400 font "Arial,11"
set output "base_time_series_normalized.png"

set title "Нормализованная динамика популяций (масштаб 0..1)"
set xlabel "Поколение (t)"
set ylabel "Доля от максимальной численности"

# Стили линий
set style line 1 lc rgb '#00ad4e' lt 1 lw 1.5
set style line 2 lc rgb '#e63320' lt 1 lw 1.5

# 2. ОТРИСОВКА С МАТЕМАТИКОЙ НА ЛЕТУ
# Выражение ($2/F_max) берет значение 2-го столбца и делит на максимум
plot "results.csv" using 1:($2/F_max) with lines ls 1 title "Рыбы (норм.)", \
     "results.csv" using 1:($3/S_max) with lines ls 2 title "Акулы (норм.)"
