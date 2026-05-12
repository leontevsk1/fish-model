# Настройка формата входных данных
set datafile separator ","
set grid
set terminal pngcairo size 1000,700 font "Arial,11"

# Стили линий: синий для Вида 1, оранжевый для Вида 2, зеленый для корма
set style line 1 lc rgb '#0072bd' lt 1 lw 2.0 # Вид 1 (Блуждание)
set style line 2 lc rgb '#d95319' lt 1 lw 2.0 # Вид 2 (Уничтожение)
set style line 3 lc rgb '#77ac30' lt 1 lw 1.5 # Корм
set style line 4 lc rgb '#444444' lt 1 lw 1.2 # Фазовая траектория

# Список сценариев из программы (v2)
scenarios = "exclusion_v1 exclusion_v2 saddle_p4"

do for [scen in scenarios] {
    
    infile = "results_var2_".scen.".csv"
    
    # 1. Сбор статистики для нормализации
    stats infile using 2 name "S1" nooutput
    stats infile using 3 name "S2" nooutput
    stats infile using 4 name "F" nooutput

    # --- ГРАФИК 1: Временной ряд ---
    set output "v2_".scen."_time_series.png"
    set title "Динамика конкуренции: Сценарий ".scen
    set xlabel "Поколение (t)"
    set ylabel "Численность особей / Единиц корма (N)"
    set key top right
    
    plot infile using 1:2 with lines ls 1 title "Вид 1 (Блуждание)", \
         infile using 1:3 with lines ls 2 title "Вид 2 (Уничтожение)", \
         infile using 1:4 with lines ls 3 title "Корм"

    # --- ГРАФИК 2: Фазовый портрет ---
    set output "v2_".scen."_phase_portrait.png"
    set title "Фазовый портрет: ".scen
    set xlabel "Численность Вида 1 (N_1)"
    set ylabel "Численность Вида 2 (N_2)"
    unset key
    
    # Визуализация аттракторов монополии
    plot infile using 2:3 with lines ls 4

    # --- ГРАФИК 3: Нормализованный временной ряд ---
    set output "v2_".scen."_normalized.png"
    set title "Нормализованная динамика: ".scen
    set ylabel "Доля от N_{max}"
    set key outside bottom center horizontal
    
    plot infile using 1:($2/S1_max) with lines ls 1 title "Вид 1 (норм.)", \
         infile using 1:($3/S2_max) with lines ls 2 title "Вид 2 (норм.)", \
         infile using 1:($4/F_max) with lines ls 3 title "Корм (норм.)"
    
    print "Processed v2 scenario: ".scen
}
