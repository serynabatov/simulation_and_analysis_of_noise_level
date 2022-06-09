import argparse
import random

import numpy as np
from scipy.stats import truncnorm

description = "This is the script generating the files for the physical sensors. All arguments are not mandatory!"

number_of_lines = 100
latitude_first = 45.4641
latitude_second = 45.4642
longitude_first = 9.1900
longitude_second = 9.1901
probability_corrupted_value = 0.05
distribution = "uniform"
db_low = 0
db_high = 130
main_mean = 0
main_file = "./sample.txt"


def get_truncated_normal(mean=0, sd=1, low=0, upp=10):
    return truncnorm(
        (low - mean) / sd, (upp - mean) / sd, loc=mean, scale=sd)


def check_operation(op, val, sec):
    if op <= 0.3:
        return val - sec
    elif op <= 0.6:
        return val + sec
    else:
        return val * sec


def write_to_file(x, y, noise):
    with open(main_file, 'w') as f:
        for line in zip(x, y, noise):
            value = random.random()
            if value <= probability_corrupted_value:
                sec = np.random.uniform()
                operation = np.random.uniform()
                if sec <= 0.3:
                    v = check_operation(operation, line[0], sec)
                    st = str(v) + "," + str(line[1]) + "," + str(line[2]) + "\n"
                elif sec <= 0.6:
                    v = check_operation(operation, line[1], sec)
                    st = str(line[0]) + "," + str(v) + "," + str(line[2]) + "\n"
                else:
                    v = check_operation(operation, line[2], sec)
                    st = str(line[0]) + "," + str(line[1]) + "," + str(v) + "\n"
            else:
                st = str(line[0]) + "," + str(line[1]) + "," + str(line[2]) + "\n"
            f.write(st)


def populate_file():
    if distribution == "uniform":
        noise = np.random.uniform(db_low, db_high, number_of_lines)
    else:
        noise_distribution = get_truncated_normal(mean=main_mean, sd=1, low=db_low, upp=db_high)
        noise = noise_distribution.rvs(number_of_lines)

    x = np.random.uniform(latitude_first, latitude_second, number_of_lines)
    y = np.random.uniform(longitude_first, longitude_second, number_of_lines)

    write_to_file(x, y, noise)


# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=description)

    parser.add_argument("-l", "--length", help="number of lines in the file. Example: -l 100")
    parser.add_argument("-lat", "--latitude", help="range of latitude for the place, it waits for two arguments. "
                                                   "Example: -lat 45.4641 45.4642 (It's Milano basically)", nargs=2)
    parser.add_argument("-lot", "--longitude", help="range of longitude for the place, it waits for two arguments. "
                                                    "Example: -lot 9.1900 9.1901 (It's Milano basically)", nargs=2)
    parser.add_argument("-p", "--probability", help="Probability of corrupted value. Example: -p 0.05")
    parser.add_argument("-d", "--distribution", help="distribution of noise. Example: -d uniform. The supported "
                                                     "distributions are uniform, gauss, ")
    parser.add_argument("-m", "--mean",
                        help="Optional. Use it only with normal distribution! Example -m 40. Means that the decibels "
                             "will be normally spread around the mean = 40")
    parser.add_argument("-f", "--file", help="name of file")

    args = parser.parse_args()

    if args.length:
        number_of_lines = args.length

    if args.latitude:
        latitude_first = args.latitude[0]
        latitude_second = args.latitude[1]

    if args.probability:
        probability_corrupted_value = args.probability

    if args.distribution:
        distribution = args.distribution

    if distribution == "uniform" and args.mean:
        raise Exception("mean and uniform cannot be set in one line!")

    if args.mean:
        main_mean = args.mean

    if args.file:
        main_file = args.file

    populate_file()
