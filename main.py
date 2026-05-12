import os
import glob
import pandas as pd
import matplotlib.pyplot as plt

# --- НАСТРОЙКИ АНАЛИЗА ---
BURN_IN = 500      # Сколько поколений отбрасываем на старте
WINDOW = 50        # Окно сглаживания для графиков
OUTPUT_DIR = 'from_py'

# Параметры из C-кода (нужны для аналитики)
FB = 10  # Порог размножения рыб
SH = 15  # Лимит голода акул


def setup_environment():
    """Создает директорию для результатов, если её нет."""
    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)


def calculate_effective_parameters(mean_fish, mean_sharks):
    """Рассчитывает эффективные коэффициенты Лотки-Вольтерры на основе эмпирических данных."""
    alpha_theor = 1.0 / FB
    gamma_theor = 1.0 / SH

    # Защита от деления на ноль, если популяция вымерла
    beta_eff = alpha_theor / mean_sharks if mean_sharks > 0 else 0
    delta_eff = gamma_theor / mean_fish if mean_fish > 0 else 0

    return alpha_theor, gamma_theor, beta_eff, delta_eff


def process_file(filepath):
    """Обрабатывает один CSV файл, строит график и возвращает метрики."""
    print(f"Обработка файла: {filepath}...")
    filename = os.path.basename(filepath)
    name_no_ext = os.path.splitext(filename)[0]

    try:
        df = pd.read_csv(filepath)
    except Exception as e:
        print(f"  Ошибка чтения {filepath}: {e}")
        return None

    # Отбрасываем стартовый хаос
    df_steady = df[df['t'] > BURN_IN].copy()
    if df_steady.empty:
        print(
            f"  Файл {filepath} слишком короткий для анализа (меньше {BURN_IN} строк).")
        return None

    # Определяем тип модели по колонкам
    metrics = {'file': filename}

    if 'fish' in df.columns and 'sharks' in df.columns:
        # --- БАЗОВАЯ МОДЕЛЬ ---
        mean_fish = df_steady['fish'].mean()
        mean_sharks = df_steady['sharks'].mean()

        alpha, gamma, beta, delta = calculate_effective_parameters(
            mean_fish, mean_sharks)

        metrics.update({
            'model_type': 'Wa-Tor Base',
            'mean_prey (X)': round(mean_fish, 2),
            'mean_predator (Y)': round(mean_sharks, 2),
            'alpha_theor': round(alpha, 4),
            'gamma_theor': round(gamma, 4),
            'beta_effective': round(beta, 6),
            'delta_effective': round(delta, 6)
        })

        # Сглаживание и построение графика
        df_steady['prey_smooth'] = df_steady['fish'].rolling(
            window=WINDOW).mean()
        df_steady['pred_smooth'] = df_steady['sharks'].rolling(
            window=WINDOW).mean()
        title = f'Фазовый портрет: {name_no_ext}'
        x_label, y_label = 'Сглаженная численность рыб', 'Сглаженная численность акул'

    elif 'sp1' in df.columns and 'sp2' in df.columns:
        # --- МОДЕЛЬ V2 (Конкуренция) или Модификации ---
        mean_sp1 = df_steady['sp1'].mean()
        mean_sp2 = df_steady['sp2'].mean()

        metrics.update({
            'model_type': 'Competition/Mod',
            'mean_prey (X)': round(mean_sp1, 2),
            'mean_predator (Y)': round(mean_sp2, 2),
            'alpha_theor': None,
            'gamma_theor': None,
            'beta_effective': None,
            'delta_effective': None
        })

        df_steady['prey_smooth'] = df_steady['sp1'].rolling(
            window=WINDOW).mean()
        df_steady['pred_smooth'] = df_steady['sp2'].rolling(
            window=WINDOW).mean()
        title = f'Фазовый портрет: {name_no_ext}'
        x_label, y_label = 'Вид 1 (сглаженно)', 'Вид 2 (сглаженно)'
    else:
        print(f"  Неизвестный формат колонок в {filepath}. Пропуск.")
        return None

    # Очистка NaN после сглаживания
    df_steady = df_steady.dropna(subset=['prey_smooth', 'pred_smooth'])

    # --- ОТРИСОВКА И СОХРАНЕНИЕ ГРАФИКА ---
    plt.figure(figsize=(8, 6))
    plt.plot(df_steady['prey_smooth'],
             df_steady['pred_smooth'], color='blue', linewidth=1)

    # Отмечаем эмпирический центр
    plt.scatter([metrics['mean_prey (X)']], [metrics['mean_predator (Y)']],
                color='red', marker='X', s=100, label='Эмпирическое равновесие', zorder=5)

    plt.title(title)
    plt.xlabel(x_label)
    plt.ylabel(y_label)
    plt.legend()
    plt.grid(True)

    plot_path = os.path.join(OUTPUT_DIR, f"{name_no_ext}_phase.png")
    plt.savefig(plot_path, dpi=150, bbox_inches='tight')
    plt.close()

    return metrics


def main():
    setup_environment()

    # Ищем все csv файлы в текущей директории и поддиректориях
    # Если нужно искать только конкретные, можно поменять шаблон
    csv_files = glob.glob('**/*.csv', recursive=True)

    # Исключаем файлы, которые могут лежать уже в папке from_py
    csv_files = [f for f in csv_files if OUTPUT_DIR not in f]

    if not csv_files:
        print("CSV файлы не найдены. Сначала запустите C-программы.")
        return

    all_metrics = []
    for f in csv_files:
        res = process_file(f)
        if res:
            all_metrics.append(res)

    # Сохраняем сводную таблицу с коэффициентами
    if all_metrics:
        report_df = pd.DataFrame(all_metrics)
        report_path = os.path.join(OUTPUT_DIR, 'analytical_summary.csv')
        report_df.to_csv(report_path, index=False)
        print(
            "\nАнализ завершен! Все графики и сводный отчет сохранены в папку", OUTPUT_DIR)
        print(f"Отчет с эффективными коэффициентами: {report_path}")


if __name__ == '__main__':
    main()
