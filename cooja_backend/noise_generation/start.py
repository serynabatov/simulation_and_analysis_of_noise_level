import argparse
import json
import os

import file_write
import file_write_c

description = "Here you should specify the path to the file that contain the parameters for generating N files"
path = "./input.json"

number_of_files = 0

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=description)

    parser.add_argument("-p", "--path", help="path to the file containing the information about the experiments. "
                                             "Example of such a file is input.json")

    args = parser.parse_args()

    if args.path:
        path = args.path

#    os.remove("region_data.c")

    with open(path) as f:
        json_content = json.load(f)

    i = 1

    for json in json_content['files']:
        file_write.number_of_lines = 100
        file_write.latitude_first = 45.4641
        file_write.latitude_second = 45.4642
        file_write.longitude_first = 9.1900
        file_write.longitude_second = 9.1901
        file_write.probability_corrupted_value = 0.05
        file_write.distribution = "uniform"
        file_write.main_mean = 0

        if "length" in json:
            file_write.number_of_lines = json["length"]

        if "latitude" in json:
            file_write.latitude_first = json["latitude"][0]
            file_write.latitude_second = json["latitude"][1]

        if "longitude" in json:
            file_write.longitude_first = json["longitude"][0]
            file_write.latitude_second = json["longitude"][1]

        if "probability" in json:
            file_write.probability_corrupted_value = json["probability"]

        if "distribution" in json:
            file_write.distribution = json["distribution"]

        if "mean" in json:
            file_write.main_mean = json["mean"]

        file_write.main_file = "sample_{}.txt".format(i)
        print(file_write.main_file)
        file_write.populate_file()

        file_write_c.write_to_c(i, file_write.number_of_lines, file_write.main_file)
        i += 1


