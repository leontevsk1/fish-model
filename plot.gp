# =================================================================
# Gnuplot script: Визуализация модифицированной модели Ва-Тор
# =================================================================

# Настройка формата входных данных
set datafile separator ","
set grid
set terminal pngcairo size 1000,700 font "Arial,11"

# Стили линий
set style line 1 lc rgb '#00ad4e' lt 1 lw 2.0 # Рыбы (Зеленый)
set style line 2 lc rgb '#e63320' lt 1 lw 2.0 # Акулы (Красный)
set style line 3 lc rgb '#0072bd' lt 1 lw 1.5 # Корм (Синий)
set style line 4 lc rgb '#444444' lt 1 lw 1.2 # Фазовая траектория

# Список сценариев из программы (результаты_mod_*.csv)
scenarios = "stable_focus prey_balance resource_only"

do for [scen in scenarios] {
    
    infile = "results_mod_".scen.".csv"
    
    # 1. Сбор статистики для нормализации
    # Gnuplot рассчитает максимумы F_max, S_max, K_max для каждого файла
    stats infile using 2 name "F" nooutput
    stats infile using 3 name "S" nooutput
    stats infile using 4 name "K" nooutput

    # --- ГРАФИК 1: Временной ряд (Абсолютные значения) ---
    set output "mod_".scen."_time_series.png"
    set title "Динамика системы: Сценарий ".scen
    set xlabel "Поколение (t)"
    set ylabel "Численность особей / Объем ресурса"
    set key top right
    
    plot infile using 1:2 with lines ls 1 title "Рыбы (N_f)", \
         infile using 1:3 with lines ls 2 title "Акулы (N_s)", \
         infile using 1:4 with lines ls 3 title "Корм (N_k)"

    # --- ГРАФИК 2: Фазовый портрет (Рыбы vs Акулы) ---
    set output "mod_".scen."_phase_portrait.png"
    set title "Фазовый портрет (Рыбы - Акулы): ".scen
    set xlabel "Численность рыб (N_f)"
    set ylabel "Численность акул (N_s)"
    unset key
    
    # Визуализация фокуса или предельного цикла
    plot infile using 2:3 with lines ls 4

    # --- ГРАФИК 3: Нормализованный временной ряд ---
    set output "mod_".scen."_normalized.png"
    set title "Нормализованная динамика: ".scen
    set ylabel "Доля от N_{max}"
    set key outside bottom center horizontal
    
    plot infile using 1:($2/F_max) with lines ls 1 title "Рыбы (норм.)", \
         infile using 1:($3/S_max) with lines ls 2 title "Акулы (норм.)", \
         infile using 1:($4/K_max) with lines ls 3 title "Корм (норм.)"
    
    print "Processed mod scenario: ".scen
}
