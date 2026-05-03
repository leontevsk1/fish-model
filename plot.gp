# Настройка формата входных данных
set datafile separator ","
set grid

# Стили линий
set style line 1 lc rgb '#00ad4e' lt 1 lw 1.5 # Рыбы (Жертвы) - Зеленый
set style line 2 lc rgb '#e63320' lt 1 lw 1.5 # Акулы (Хищники) - Красный
set style line 3 lc rgb '#0072bd' lt 1 lw 1.5 # Корм (Водоросли) - Синий

# Общая настройка терминала
set terminal pngcairo size 800,600 font "Arial,11"

# --- ГРАФИК 1: Временной ряд (Абсолютные значения) ---
set output "mod_time_series.png"
set title "Динамика трехуровневой системы (Хищник - Жертва - Корм)"
set xlabel "Поколение (t)"
set ylabel "Численность особей / Объем ресурса"
set key top right

plot "results_mod.csv" using 1:2 with lines ls 1 title "Рыбы (N_f)", \
     "results_mod.csv" using 1:3 with lines ls 2 title "Акулы (N_s)", \
     "results_mod.csv" using 1:4 with lines ls 3 title "Корм"

# --- ГРАФИК 2: Фазовый портрет (Рыбы vs Акулы) ---
set output "mod_phase_portrait.png"
set title "Фазовый портрет при лимитированном ресурсе"
set xlabel "Численность рыб (N_f)"
set ylabel "Численность акул (N_s)"
unset key

# Отрисовываем фазовую траекторию для хищников и жертв
plot "results_mod.csv" using 2:3 with lines lc rgb "#444444" lw 1.2

# --- ГРАФИК 3: Нормализованный временной ряд ---
set key top right

# Сбор статистики для поиска максимумов
stats "results_mod.csv" using 2 name "F" nooutput
stats "results_mod.csv" using 3 name "S" nooutput
stats "results_mod.csv" using 4 name "K" nooutput

# Изменение пропорций для нормализованного графика
set terminal pngcairo size 800,400 font "Arial,11"
set output "mod_time_series_normalized.png"

set title "Нормализованная динамика популяций (масштаб 0..1)"
set xlabel "Поколение (t)"
set ylabel "Доля от максимальной численности"

# Отрисовка с делением на максимум
plot "results_mod.csv" using 1:($2/F_max) with lines ls 1 title "Рыбы (норм.)", \
     "results_mod.csv" using 1:($3/S_max) with lines ls 2 title "Акулы (норм.)", \
     "results_mod.csv" using 1:($4/K_max) with lines ls 3 title "Корм (норм.)"
