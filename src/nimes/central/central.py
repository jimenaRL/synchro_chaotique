import random
import socket
import argparse
import numpy as np
from time import sleep
from copy import deepcopy
from osc import decodeOSC
from OSC import OSCClient, OSCMessage, OSCBundle

NB_NODES = 33

class Node(object):

    def __init__(self, ip=-1, vertex=[0, 0, 0]):

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
                    default=6666)
parser.add_argument('-rate',
                    dest="RATE",
                    type=float,
                    default=32)
parser.add_argument('-c',
                    dest="C",
                    type=float,
                    default=0.051)
parser.add_argument('-mu',
                    dest="MU",
                    type=float,
                    default=0.001)
parser.add_argument('-nb_steps',
                    dest="nb_steps",
                    type=int,
                    default=0)
parser.add_argument('-init',
                    dest="init",
                    type=str,
                    default="")
parser.add_argument('-deltas',
                    dest="deltas",
                    type=str,
                    default="")
parser.add_argument('-jeu',
                    dest="JEU",
                    type=str,
                    default="2osc")

for k, v in parser.parse_args().__dict__.items():
    locals()[k] = v
    print("%s: %s" % (k,v))

# set udp server
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

client = OSCClient()
client.connect((UDP_IP, UDP_PORT))

# init = {int(s.split(":")[0]):
# (float(s.split(":")[1].split(",")[0]), float(s.split(":")[1].split(",")[1]))
# for s in init.split(";")}

# ip - vertex table
IP_VERTEX = [
    (6, [0, -1, 1]),
    (7, [0, 1, 1]),
    (8, [0, 1, -1]),
    (9, [0, -1, -1]),
    (13, [1, -1, 2]),
    (11, [1, 1, 2]),
    (15, [1, -1, 0]),
    (10, [1, 1, 0]),
    (14, [1, -1, -2]),
    (12, [1, 1, -2]),
    (26, [2, -2, 2]),
    (24, [2, 0, 2]),
    (25, [2, -2, 0]),
    (20, [2, 0, 0]),
    (22, [2, 2, 0]),
    (21, [2, 0, -2]),
    (23, [2, 2, -2]),
    (35, [3, -2, 3]),
    (34, [3, 0, 3]),
    (36, [3, -2, 1]),
    (33, [3, 0, 1]),
    (32, [3, 2, 1]),
    (30, [3, -2, -1]),
    (37, [3, 0, -1]),
    (31, [3, 2, -1]),
    (44, [4, -1, 3]),
    (42, [4, -1, 1]),
    (40, [4, 1, 1]),
    (43, [4, -1, -1]),
    (41, [4, 1, -1]),
    (97, [5, 1, 0]),
    (98, [5, -1, 0]),
    (99, [5, -1, 2]),
]

IPS = [ip_v[0] for ip_v in IP_VERTEX]
VERTICES = [ip_v[1] for ip_v in IP_VERTEX]

def bump(x, smin, Mmin, Mmax, smax):
    if(x<smin or x>smax):
        return 0
    if(x<Mmin):
        return (x-smin)/(Mmin-smin)
    if(x>Mmax):
        return 1-(x-Mmax)/(smax-Mmax)
    return 1


def triplebump(x, smin, Mmin1, Mmax1, Mmin2, Mmax2, Mmin3, Mmax3, smax):
    a0 = bump(x, smin, Mmin1, Mmax1, Mmin2)
    a1 = bump(x, Mmax1, Mmin2, Mmax2, Mmin3)
    a2 = bump(x, Mmax2, Mmin3, Mmax3, smax)
    return a0, a1, a2

def triplebumpA(x):
    return triplebump(x, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8)

def triplebumpB(x):
    return triplebump(x, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4)

def triplebumpC(x):
    return triplebump(x, 0.0, 0.15, 0.12, 0.21, 0.25, 0.28, 0.35, 0.37)

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

# set initial condition (dirac in one node)
deltas = map(int, deltas.split(" ")) if deltas else [random.choice(IPS)]
nb_deltas = len(deltas)
print("Initial deltas %s" % deltas)
for n in NODES: 
    if n.ip in deltas:
        n.current = 1.0/nb_deltas
        n.previous = 1.0/nb_deltas
    else:
        n.current = -1.0/(NB_NODES-nb_deltas)
        n.previous = -1.0/(NB_NODES-nb_deltas)

def set_value(_ip, alpha, beta, step):
    "Force oscillation on one node."
    for n in NODES:
        if n.ip == delta:
            n.current += alpha * np.cos(beta*2*np.pi*step)
            n.previous += alpha * np.cos(beta*2*np.pi*step)

# set neighbors and normalise them
for n in NODES:
    neighs = [get_node(v) for v in voisin(n.vertex, VERTEX_LIST)]
    n.neighbors = [(node, 1.0/len(neighs)) for node in neighs]

# for n in NODES:
#     print(n)

def wave(nodes):
    # compute
    for node in nodes:
        D2TP = 2.0 * node.current - node.previous
        D2XP_current = 0.0
        D2XP_previous = 0.0
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
                ss += "%s %s %1.2f\n" % (n.ip, n.vertex, n.current)
        print(ss)

def mean(nodes):
    mean = 0.0
    for n in nodes:
        mean += n.current
    return mean

def gate(x, vmax=1.0, vmin=-1.0):
    return max(min(x, vmax), vmin)

def send(nodes):
    mean_p = 0.0
    mean_dp = 0.0
    for node in nodes:
        msg = OSCMessage("/%i" % node.ip)
        # presure and presure derivative (constants setted to assumre equal mean)
        p = gate(1.5*node.current)
        dp = gate(5*(node.current - node.previous))
        if JEU=="2osc":
            xA0, xA1, xA2 = dp, p, 0
            xB0, xB1, xB2 = 0, 0, 0
            xC0, xC1, xC2 = 0, 0, 0
        elif JEU=="3chords":
            xA0, xA1, xA2 = dp, p, dp+p
            xB0, xB1, xB2 = dp, p, dp+p
            xC0, xC1, xC2 = dp, p, dp+p
        # print("%i %f %f %f %f %f %f %f %f %f" % (node.ip, xA0, xA1, xA2, xB0, xB1, xB2, xC0, xC1, xC2))
        msg.append(xA0)
        msg.append(xA1)
        msg.append(xA2)
        msg.append(xB0)
        msg.append(xB1)
        msg.append(xB2)
        msg.append(xC0)
        msg.append(xC1)
        msg.append(xC2)
        bundle = OSCBundle()
        bundle.append(msg)
        client.send(bundle)
        # mean_p += p
        # mean_dp += dp
    # print("mean_p %f mean_dp %f" % (mean_p, mean_dp))

#print_nodes(NODES)

step = 0
while True:
    step += 1
    if nb_steps >= 0 and step > nb_steps:
        break
    sleep(1./RATE)
    wave(NODES)
    #set_value(_ip=20, alpha=0.0075, beta=1./float(3*RATE), step=step)
    #print("iter %i mean %1.64f" % (step, mean(NODES)))
    send(NODES)





