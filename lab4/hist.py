def build_text_histogram_from_file(filename):
    """
    Считывает данные из файла, содержащего времена реакций в наносекундах,
    и строит текстовую гистограмму.

    Args:
        filename: Имя файла с данными.
    """
    try:
        with open(filename, 'r') as f:
            data = f.readlines()
    except FileNotFoundError:
        print(f"Error: File '{filename}' not found.")
        return

    # Преобразование строк в числа (наносекунды)
    try:
        times_ns = [int(line.strip().split()[0]) for line in data]
    except ValueError:
        print(f"Error: Invalid data format in '{filename}'. Each line should be '<number> ns'.")
        return

    # Если все считанные значения равны нулю, то нет смысла строить гистограмму
    if all(times == 0 for times in times_ns):
        print(f"Error: All values in '{filename}' are zero. Cannot build a histogram.")
        return

    # Определение диапазона значений и ширины столбцов
    min_val = min(times_ns)
    max_val = max(times_ns)
    num_bins = 10  # Количество столбцов гистограммы
    bin_width = (max_val - min_val) / num_bins

    # Подсчет количества значений в каждом столбце
    counts = [0] * num_bins
    for val in times_ns:
        bin_index = int((val - min_val) / bin_width)
        if bin_index == num_bins:  # Обработка случая, когда val == max_val
            bin_index -= 1
        counts[bin_index] += 1

    # Определение максимальной высоты столбца (для масштабирования)
    max_count = max(counts)

    # Построение текстовой гистограммы
    print("Text Histogram:")
    for i in range(num_bins):
        bar = "*" * int(counts[i] / max_count * 40)  # Масштабируем до 40 символов
        lower_bound = min_val + i * bin_width
        upper_bound = lower_bound + bin_width
        print(f"{lower_bound:.0f}-{upper_bound:.0f} ns: {bar} ({counts[i]})")

# Пример использования
build_text_histogram_from_file('histdata.txt')