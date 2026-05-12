# Настройка формата данных
set datafile separator ","
set grid
set terminal pngcairo size 1000,700 font "Arial,12"

# Стили линий
set style line 1 lc rgb '#00ad4e' lt 1 lw 2 # Рыбы
set style line 2 lc rgb '#e63320' lt 1 lw 2 # Акулы
set style line 3 lc rgb '#444444' lt 1 lw 1 # Фазовая траектория

# Список сценариев, соответствующих именам файлов из программы
scenarios = "center saddle monopoly"

do for [scen in scenarios] {
    
    # Формируем имя входного файла
    infile = "results_base_".scen.".csv"
    
    # 1. Собираем статистику для нормализации и подписей
    stats infile using 2 name "F" nooutput
    stats infile using 3 name "S" nooutput

    # --- ГРАФИК 1: Временной ряд (Time Series) ---
    set output "base_".scen."_time_series.png"
    set title "Динамика популяций: Сценарий ".scen
    set xlabel "Поколение (t)"
    set ylabel "Численность особей (N)"
    set key top right
    
    plot infile using 1:2 with lines ls 1 title "Рыбы (N_f)", \
         infile using 1:3 with lines ls 2 title "Акулы (N_s)"

    # --- ГРАФИК 2: Фазовый портрет (Phase Portrait) ---
    set output "base_".scen."_phase_portrait.png"
    set title "Фазовый портрет: ".scen
    set xlabel "Численность рыб (N_f)"
    set ylabel "Численность акул (N_s)"
    unset key
    
    # Рисуем траекторию. Для "center" здесь должна быть петля, для "saddle" - уход в ноль.
    plot infile using 2:3 with lines ls 3

    # --- ГРАФИК 3: Нормализованная динамика ---
    # Позволяет увидеть фазовый сдвиг, если масштабы популяций сильно разнятся
    set output "base_".scen."_normalized.png"
    set title "Нормализованная динамика: ".scen
    set ylabel "Доля от N_{max}"
    set key outside bottom center horizontal
    
    plot infile using 1:($2/F_max) with lines ls 1 title "Рыбы (норм.)", \
         infile using 1:($3/S_max) with lines ls 2 title "Акулы (норм.)"
    
    print "Processed scenario: ".scen
}
