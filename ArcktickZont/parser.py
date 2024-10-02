# -*- coding: utf-8 -*-
import json


def parse_line(line):
    # Удаляем пробелы в начале и конце строки и разделяем строку по символу ':'
    parts = line.strip().split(' : ')

    # Проверяем, что мы получили ровно три части
    if len(parts) != 3:
        raise ValueError(f"Invalid line format: {line}")

    # Преобразуем части в соответствующие типы данных
    seconds = int(parts[0])
    particles = int(parts[1])
    dose_rate = float(parts[2])

    # Создаем словарь с нужными ключами
    data = {
        "seconds": seconds,
        "particles": particles,
        "dose_rate": dose_rate
    }

    # Преобразуем словарь в строку формата JSON
    json_data = json.dumps(data, ensure_ascii=False)

    return json_data


def parse_file(input_file, output_file):
    with open(input_file, 'r', encoding='utf-8') as infile, open(output_file, 'w', encoding='utf-8') as outfile:
        for line in infile:
            try:
                json_data = parse_line(line)
                outfile.write(json_data + '\n')
            except ValueError as e:
                print(f"Skipping line due to error: {e}")


# Пример использования:
input_file = 'device-monitor-240707-205255.log'
output_file = 'output.json'

parse_file(input_file, output_file)
