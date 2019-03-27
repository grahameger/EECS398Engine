#!/usr/bin/python3

import csv

with open('majestic_million.csv') as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    for row in csv_reader:
        print("http://" + row[2] + "/")
