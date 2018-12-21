import csv
import time
import socket
import random
import argparse

from osc import decodeOSC


class Node(object):

    def __init__(self, ip=-1, vertex=[0, 0, 0]):
        self.ip = ip
        self.vertex = vertex
        self.weight = 0.0
        self.current_val = 0.0
        self.previous_val  = 0.0
        self.neighbors  = []

    def __str__(self):
        return "[Node]\n\tip: %i\n\tvertex: %s\n\tweight: %f\n\tcurrent_val %f \n\tprevious_val %f\n\tneighbors: %s" % (
            self.ip,
            str(self.vertex),
            self.weight,
            self.current_val,
            self.previous_val,
            self.neighbors)


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
    # 16: [1,-1,0],
    # 17: [1,1,0],
    # 18: [1,-1,-2],
    # 19: [1,+1,-2],
    # 20: [2,-2,2],
    # 21: [2,0,2],
    # 22: [2,-2,0],
    # 23: [2,0,0],
    # 24: [2,2,0],
    # 25: [2,0,-2],
    # 26: [2,2,-2],
    # 27: [3,-2,3],
    # 28: [3,0,3],
    # 29: [3,-2,1],
    # 30: [3,0,1],
    # 31: [3,2,1],
    # 32: [3,-2,-1],
    # 33: [3,0,-1],
    # 34: [3,2,-1],
    # 35: [4,-1,3],
    # 36: [4,-1,1],
    # 37: [4,1,1],
    # 38: [4,-1,-1],
    # 39: [4,1,-1],
    # 40: [5,-1,0],
    # 41: [5,-1,0],
    # 41: [4,1,0]
}

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

# set neighbors
for n in NODES:
    n.neighbors = voisin(n.vertex, VERTEX_LIST)

# set dirac in last node
NODES[0].current_val = 10


for n in NODES:
    print(n)



