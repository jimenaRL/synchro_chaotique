import csv
import time
import socket
import random
import argparse

from osc import decodeOSC

EXPECTED_DECODED_LENGTH = 3

NODES = {
    "192.168.0.10": {"iter": 0, "value": -1.0},
    "192.168.0.11": {"iter": 0, "value": -1.0},
    "192.168.0.12": {"iter": 0, "value": -1.0},
    "192.168.0.13": {"iter": 0, "value": -1.0},
    "192.168.0.14": {"iter": 0, "value": -1.0},
    "192.168.0.15": {"iter": 0, "value": -1.0},
}

ENDPOINT = "/monitor"

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
parser.add_argument('-name',
                    dest="NAME",
                    type=str,
                    default="")
parser.add_argument('-show',
                    dest="SHOW",
                    action="store_true")

for k, v in parser.parse_args().__dict__.items():
    locals()[k] = v
    print("%s: %s" % (k,v))

# set udp server
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

def print_nodes():
    s = "| "
    for n, d in NODES.iteritems():
        s += "%s: %i %f | " % (n.split(".")[-1], d["iter"], d["value"])
    print (s)

nb_errors = 0
with open("%s.csv" % NAME, mode='w') as f:
    writer = csv.writer(f, delimiter=',')
    while True:
        try:
            data, address = sock.recvfrom(BUFSIZE)
            remote_ip, remote_port = address
            decoded = decodeOSC(data)
            if not len(decoded) == EXPECTED_DECODED_LENGTH:
                nb_errors += 1
                print("Error number %i: len(decoded) is %i, expected %i." % (
                    nb_errors,
                    len(decoded),
                    EXPECTED_DECODED_LENGTH))
                continue
            endpoint, _types = decoded[:2]
            if endpoint==ENDPOINT:
                value = decoded[2]
                iter_ = NODES[remote_ip]["iter"]+1
                if remote_ip in NODES.keys():
                    NODES[remote_ip]["iter"] = iter_
                    NODES[remote_ip]["value"] = value
                if SHOW:
                    print_nodes()
                writer.writerow([remote_ip.split(".")[-1], iter_, value])
            else:
                print("Wrong endpoint `%s`. Must be `%s`." % (endpoint, ENDPOINT))
                continue
        except Exception as e:
            print(e)
