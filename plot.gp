# Настройка формата входных данных
set datafile separator ","
set grid

# Стили линий: синий для Вида 1, оранжевый для Вида 2, зеленый для корма
set style line 1 lc rgb '#0072bd' lt 1 lw 1.5 # Вид 1 (Блуждание)
set style line 2 lc rgb '#d95319' lt 1 lw 1.5 # Вид 2 (Уничтожение)
set style line 3 lc rgb '#77ac30' lt 1 lw 1.5 # Корм

# Общая настройка терминала
set terminal pngcairo size 800,600 font "Arial,11"

# --- ГРАФИК 1: Сохранение временного ряда ---
set output "time_series_var2.png"
set title "Динамика популяций (Конкуренция за ресурс)"
set xlabel "Поколение (t)"
set ylabel "Численность особей/единиц корма (N)"
set key top right

plot "results_var2.csv" using 1:2 with lines ls 1 title "Вид 1 (Блуждание)", \
     "results_var2.csv" using 1:3 with lines ls 2 title "Вид 2 (Уничтожение)", \
     "results_var2.csv" using 1:4 with lines ls 3 title "Корм"

# --- ГРАФИК 2: Сохранение фазового портрета ---
# Смена выходного файла
set output "phase_portrait_var2.png"
set title "Фазовый портрет (Конкурентное исключение)"
set xlabel "Численность Вида 1 (N_1)"
set ylabel "Численность Вида 2 (N_2)"
# Для фазового портрета легенда избыточна
unset key

# Траектория в пространстве состояний (N_1, N_2)
plot "results_var2.csv" using 2:3 with lines lc rgb "#444444" lw 1.2

# --- ГРАФИК 3: Нормализованный временной ряд ---
set key top right

# 1. СОБИРАЕМ СТАТИСТИКУ (поиск максимумов для 3 столбцов)
stats "results_var2.csv" using 2 name "S1" nooutput
stats "results_var2.csv" using 3 name "S2" nooutput
stats "results_var2.csv" using 4 name "F" nooutput

# Настройка терминала и файла
set terminal pngcairo size 800,400 font "Arial,11"
set output "time_series_normalized_var2.png"

set title "Нормализованная динамика популяций (масштаб 0..1)"
set xlabel "Поколение (t)"
set ylabel "Доля от максимальной численности"

# 2. ОТРИСОВКА С МАТЕМАТИКОЙ НА ЛЕТУ
plot "results_var2.csv" using 1:($2/S1_max) with lines ls 1 title "Вид 1 (норм.)", \
     "results_var2.csv" using 1:($3/S2_max) with lines ls 2 title "Вид 2 (норм.)", \
     "results_var2.csv" using 1:($4/F_max) with lines ls 3 title "Корм (норм.)"
