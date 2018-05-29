import argparse
from time import sleep

from OSC import OSCServer

from microcontroller import MicroController

parser = argparse.ArgumentParser()
parser.add_argument("-ip", help="ip", type=str, default='0.0.0.0')
parser.add_argument("-port", help="port", type=int, default=5213)
parser.add_argument('--neighbors', nargs='+', type=int)
pargs = parser.parse_args()

server = OSCServer((pargs.ip, pargs.port))
server.timeout = 0
run = True
tau = 4
dt = 0.01

neighbors = pargs.neighbors if pargs.neighbors else []
esp8266 = MicroController(address=pargs.port, neighbors=neighbors)


# this method of reporting timeouts only works by convention
# that before calling handle_request() field .timed_out is
# set to False
def handle_timeout(self):
    self.timed_out = True

# add handle_timeout method to server class
import types
server.handle_timeout = types.MethodType(handle_timeout, server)


def receive_callback(path, tags, args, source):
    """
    from OSC import OSCClient, OSCMessage
    client = OSCClient()
    client.connect(("localhost", 666))
    msg = OSCMessage("/receive")
    msg.append(777)
    msg.append(0.2)
    client.send(msg)
    """
    neighbor = args[0]
    value = args[1]
    esp8266.update_neighbor(neighbor, value)


def quit_callback(path, tags, args, source):
    # don't do this at home (or it'll quit blender)
    global run
    run = False

server.addMsgHandler("/receive", receive_callback)


# script that's called every frame
def each_frame():
    # clear timed_out flag
    server.timed_out = False
    # handle all pending requests then return
    while not server.timed_out:
        server.handle_request()


while run:
    # call user script
    each_frame()
    # wait
    sleep(tau/2.)
    # update value
    esp8266.update_value(dt)
    # wait
    sleep(tau/2.)
    # send value to others
    esp8266.send_value()


server.close()
