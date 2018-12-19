import os
import csv
import argparse
import numpy as np
import matplotlib.pyplot as plt

parser = argparse.ArgumentParser()

parser.add_argument('-name',
                    dest="NAME",
                    type=str,
                    default="")

for k, v in parser.parse_args().__dict__.items():
    locals()[k] = v
    print("%s: %s" % (k,v))

data = {}
max_iter = -1
# rows attended: 'ip', 'iter', 'value'
with open("%s.csv" % NAME, mode='r') as f:
    reader = csv.reader(f, delimiter=',')
    for row in reader:
        ip, iter_, value = row
        ip = int(ip)
        iter_ = int(iter_)
        value = float(value)
        max_iter = max(max_iter, iter_)
        if not ip in data.keys():
            data[ip] = []
        data[ip].append([iter_, value])

nodes = sorted(data.keys())
index = {idx: node for idx, node in enumerate(nodes)}
matrix = np.zeros((len(nodes), max_iter))

for idx, node in index.items():
    for iter_, value in data[node]:
        matrix[idx, iter_-1] = value

plt.imsave("%s.png" % NAME, matrix)
os.system("open %s.png" % NAME)