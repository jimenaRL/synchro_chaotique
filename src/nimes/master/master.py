import csv
import time
import socket
import random
import argparse
from copy import deepcopy

NB_NODES = 33
INIT = '0: 0.0,1.0; 1: 0.0,1.0; 2: 0.0,1.0; 3: 0.0,1.0; 4: 0.0,1.0; 5: 0.0,1.0; 6: 0.0,1.0; 7: 0.0,1.0; 8: 0.0,1.0; 9: 0.0,1.0; 10: 0.0,1.0; 11: 0.0,1.0; 12: 0.0,1.0; 13: 0.0,1.0; 14:0.0,1.0; 15: 0.0,1.0; 16: 0.0,1.0; 17: 0.0,1.0; 18: 0.0,1.0; 19: 0.0,1.0; 20: 0.0,1.0; 21: 0.0,1.0; 22: 0.0,1.0; 23: 0.0,1.0; 24: 0.0,1.0; 25: 0.0,1.0; 26: 0.0,1.0; 27: 0.0,1.0;28: 0.0,1.0; 29: 0.0,1.0; 30: 0.0,1.0; 31: 0.0,1.0; 32: 0.0,1.0'

class Node(object):

    def __init__(self, ip=-1, vertex=[0, 0, 0], weight=0.0):

        self.ip = ip
        self.vertex = vertex
        self.current = 0.0
        self.previous = 0.0
        self.tmp = None
        self.neighbors = None

    def __str__(self):
        _str =  "[Node]\n\tip: %i \n\tvertex: %s \n\tprevious %f  \n\tcurrent %f \n\ttmp %s \n\tneighbors" % (
            self.ip,
            str(self.vertex),
            self.current,
            self.previous,
            self.tmp)
        if self.neighbors:
            for n, w in self.neighbors:
                _str += "\n\t\t v %s w %f c %f p %f" % (n.vertex, w, n.current, n.previous)
        return _str

parser = argparse.ArgumentParser()

parser.add_argument('-ip',
                    dest='UDP_IP',
                    type=str,
                    default="localhost")
parser.add_argument('-port',
                    dest="UDP_PORT",
                    type=int,
                    default=5005)
parser.add_argument('-bufsize',
                    dest="BUFSIZE",
                    type=int,
                    default=256)
parser.add_argument('-C',
                    dest="C",
                    type=float,
                    default=0.051)
parser.add_argument('-MU',
                    dest="MU",
                    type=float,
                    default=0.001)
parser.add_argument('-steps',
                    dest="steps",
                    type=int,
                    default=0)
parser.add_argument('-init',
                    dest="init",
                    type=str,
                    default=INIT)

for k, v in parser.parse_args().__dict__.items():
    locals()[k] = v
    print("%s: %s" % (k,v))


init = {int(s.split(":")[0]):
(float(s.split(":")[1].split(",")[0]), float(s.split(":")[1].split(",")[1]))
for s in init.split(";")}

# ip - vertex table
IP_VERTEX = [
    (10, [0, -1, 1]),
    (11, [0, 1, 1]),
    (12, [0, 1, -1]),
    (13, [0, -1, -1]),
    (14, [1, -1, 2]),
    (15, [1, 1, 2]),
    (16, [1, -1, 0]),
    (17, [1, 1, 0]),
    (18, [1,-1, -2]),
    (19, [1, 1, -2]),
    (20, [2, -2, 2]),
    (21, [2, 0, 2]),
    (22, [2, -2, 0]),
    (23, [2, 0, 0]),
    (24, [2, 2, 0]),
    (25, [2, 0, -2]),
    (26, [2, 2, -2]),
    (27, [3, -2, 3]),
    (28, [3, 0, 3]),
    (29, [3, -2, 1]),
    (30, [3, 0, 1]),
    (31, [3, 2, 1]),
    (32, [3, -2, -1]),
    (33, [3, 0, -1]),
    (34, [3, 2, -1]),
    (35, [4, -1, 3]),
    (36, [4, -1, 1]),
    (37, [4, 1, 1]),
    (38, [4, -1, -1]),
    (39, [4, 1, -1]),
    (40, [5, 1, 0]),
    (41, [5, -1, 0]),
    (42, [5, -1, 2]),
]


def get_ip(vertex):
    for ip, v in IP_VERTEX:
        if vertex == v:
            return ip

# set of vertices
VERTEX_LIST = [v for ip, v in IP_VERTEX]

def voisin(vertex, vertexliste):
    vertexvoisin = []
    for V in vertexliste:
        if (abs(vertex[0]-V[0])+abs(vertex[1]-V[1])+abs(vertex[2]-V[2])<=2 and vertex[0]!=V[0]):
            vertexvoisin.append(V)
    return vertexvoisin


# create nodes
NODES = [Node(ip, v) for ip, v in IP_VERTEX]

def get_node(vertex):
    _ip = get_ip(vertex)
    for node in NODES:
        if node.ip == _ip:
            return node

# set intial condition
for n in NODES:
    n.current = -1
    n.previous = -1

# set dirac in last node
NODES[13].current = 32
NODES[13].previous = 32

# set neighbors and normalise them
for n in NODES:
    neighs = [get_node(v) for v in voisin(n.vertex, VERTEX_LIST)]
    n.neighbors = [(node, 1.0/len(neighs)) for node in neighs]

for n in NODES:
    print(n)

def wave(nodes):
    # compute
    for node in nodes:
        D2TP = 2.0 * node.current - node.previous
        D2XP_current = 0.0
        D2XP_previous = 0.0
        inv_weight =0.0
        for neigh, weight in node.neighbors:
            # weight is unusued here
            D2XP_previous += (neigh.previous - node.previous)
            D2XP_current += (neigh.current - node.current)
        # DTD2XP_current = (D2XP_current - D2XP_previous)
        node.tmp = D2TP + C * D2XP_current + MU * (D2XP_current - D2XP_previous)

    # update
    for node in nodes:
        node.previous = deepcopy(node.current)
        node.current = deepcopy(node.tmp)
        node.tmp = None

def print_nodes(nodes):
    for step in range(6):
        ss = "------------ %i ------------\n" % step
        for n in NODES:
            if n.vertex[0]==step:
                ss += "%s %1.2f\n" % (n.vertex, n.current)
        print(ss)

def mean(nodes):
    mean = 0.0
    for n in nodes:
        mean += n.current
    return mean


# iterations
print(0)
print_nodes(NODES)
for _ in range(steps):
    wave(NODES)
    print("iter %i mean %f" % (_, mean(NODES)))
print_nodes(NODES)



