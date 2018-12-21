import csv
import time
import socket
import random
import argparse

from osc import decodeOSC


class Node(object):

    def __init__(self, ip=-1, vertex=[0, 0, 0], weight=0.0):

        self.ip = ip
        self.vertex = vertex
        self.weight = weight
        self.current_val = 0.0
        self.previous_val = 0.0
        self.neighbors = None

    def __str__(self):
        _str =  "[Node]\n\tip: %i\n\tvertex: %s\n\tcurrent_val %f \n\tprevious_val %f\n\tneighbors" % (
            self.ip,
            str(self.vertex),
            self.current_val,
            self.previous_val)
        if self.neighbors:
            for n in self.neighbors:
                _str += "\n\t\t v %s w %f c %f p %f" % (n.vertex, n.weight, n.current_val, n.previous_val)
        return _str


def update(vertices):
    for v in vertices:
        pass

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

# ip - vertex table
IP_VERTEX = {
    10: [0,-1,1],
    11: [0,1,1],
    12: [0,1,-1],
    13: [0,-1,-1],
    14: [1,-1,2],
    15: [1,+1,2],
    16: [1,-1,0],
    17: [1,1,0],
    18: [1,-1,-2],
    19: [1,+1,-2],
    20: [2,-2,2],
    21: [2,0,2],
    22: [2,-2,0],
    23: [2,0,0],
    24: [2,2,0],
    25: [2,0,-2],
    26: [2,2,-2],
    27: [3,-2,3],
    28: [3,0,3],
    29: [3,-2,1],
    30: [3,0,1],
    31: [3,2,1],
    32: [3,-2,-1],
    33: [3,0,-1],
    34: [3,2,-1],
    35: [4,-1,3],
    36: [4,-1,1],
    37: [4,1,1],
    38: [4,-1,-1],
    39: [4,1,-1],
    40: [5,1,0],
    41: [5,-1,0],
    42: [5,-1,0],
}

def get_ip(vertex):
    for ip, v in IP_VERTEX.items():
        if vertex == v:
            return ip

# set of vertices
VERTEX_LIST = IP_VERTEX.values()

def voisin(vertex, vertexliste):
    vertexvoisin = []
    for V in vertexliste:
        if (abs(vertex[0]-V[0])+abs(vertex[1]-V[1])+abs(vertex[2]-V[2])<=2 and vertex[0]!=V[0]):
            vertexvoisin.append(V)
    return vertexvoisin


# create nodes
NODES = [Node(ip, v) for ip, v in IP_VERTEX.items()]


# set dirac in last node
NODES[0].current_val = 10

# set neighbors and normalise
for n in NODES:
    neighs_vertex = voisin(n.vertex, VERTEX_LIST)
    n.neighbors = [Node(get_ip(v), v, 1.0/len(neighs_vertex)) for v in neighs_vertex]

for n in NODES:
    print(n)


