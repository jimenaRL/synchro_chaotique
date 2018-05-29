import types
import argparse
from time import sleep

from OSC import OSCServer

from microcontroller import MicroController

parser = argparse.ArgumentParser()
parser.add_argument("-ip", help="ip", type=str, default='0.0.0.0')
parser.add_argument("-port", help="port", type=int, default=5213)
parser.add_argument('-neighbors', nargs='+', type=int)
parser.add_argument('-init', help="init value", type=float, default=0.0)
parser.add_argument('-frequency', help="frequency", type=float, default=0.0)
parser.add_argument('-couplings', nargs='+', type=float)
parser.add_argument('-tau', help="frame time of server", type=float, default=0.01)
parser.add_argument('-dt', help="time delta for updating microcontroller values", type=float, default=0.01)
parser.add_argument('-verbose', help='verbose', type=int, default=1)
pargs = parser.parse_args()


neighbors = pargs.neighbors if pargs.neighbors else []
micro = MicroController(address=pargs.port,
                        neighbors=neighbors,
                        init=pargs.init,
                        frequency=pargs.frequency,
                        couplings=pargs.couplings,
                        dt=pargs.dt,
                        verbose=pargs.verbose)

tau = pargs.tau

server = OSCServer((pargs.ip, pargs.port))
server.timeout = 0
run = True


def handle_timeout(self):
    """
    this method of reporting timeouts only works by convention
    that before calling handle_request() field .timed_out is set to False
    """
    self.timed_out = True

# add handle_timeout method to server class
server.handle_timeout = types.MethodType(handle_timeout, server)


def receive_callback(path, tags, args, source):
    """
    callback for received osc message that update a neighbor's value of in micro
    """
    neighbor = args[0]
    value = args[1]
    micro.update_neighbor(neighbor, value)


def quit_callback(path, tags, args, source):
    """ don't do this at home (or it'll quit blender)"""
    global run
    run = False

server.addMsgHandler("/quit", quit_callback)
server.addMsgHandler("/receive", receive_callback)


# script called every frame
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
    # send micro value to its neighbors
    micro.send_value()
    # wait
    sleep(tau/2.)
    # update value
    micro.update_value()

server.close()
