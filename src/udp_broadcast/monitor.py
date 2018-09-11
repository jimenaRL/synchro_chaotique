import socket

UDP_IP = "192.168.0.36"
UDP_PORT = 5005

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
sock.bind((UDP_IP, UDP_PORT))

nodes = {
    "192.168.0.43": {"iter": 0, "value": -1},
    "192.168.0.30": {"iter": 0, "value": -1},
    "192.168.0.13": {"iter": 0, "value": -1},
}

def print_nodes():
    s = "| "
    for n, d in nodes.iteritems():
        s += "%s: %1.5f | " % (n, d["value"])
    print (s)

while True:
    data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
    if addr[0] in nodes:
        nodes[addr[0]]["value"] = float(data)
        print_nodes()
    else:
        print(data, addr)