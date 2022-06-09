import os
import csv


def write_to_c(number_of_values, number_of_lines, file_name):
    file_row_t = ""
    if "1" in file_name:
        file_row_t = """#define MAXCHAR 20

typedef struct file_row {
    char lat[MAXCHAR];
    char lot[MAXCHAR];
    char noise[MAXCHAR];
} file_row_t;
"""

    file_row_t += "\nstatic file_row_t file_entry_{}[{}] = {{\n".format(number_of_values, number_of_lines)
    counter = 0
    with open(file_name) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for row in csv_reader:
            counter += 1
            if counter > 1:
                file_row_t += ", "
            file_row_t += "{{ {}, {}, {} }}\n".format(row[0], row[1], row[2])

        file_row_t += "};\n"

    with open("region_data.c", "a+") as f:
        f.write(file_row_t)

