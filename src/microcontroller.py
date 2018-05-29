from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import numpy as np
from OSC import OSCClient, OSCMessage

import logging
logger = logging.getLogger(__name__)


class MicroController(object):

    def __init__(self,
                 address=0,
                 neighbors=[],
                 init=0.0,
                 frequency=0.1,
                 couplings=[-0.5, 0.2],
                 dt=0.01,
                 verbose=1):
        """
            address:
                [int] identifaction address (own port)
            neighbors:
                list of neighbors identifaction (neighbors ports)
        """

        self._verbose = verbose
        self._address = int(address)
        self._dt = np.float32(dt)

        self.data = {"iter": 0, "phi": init}

        # intrinsic frequency
        self._w = frequency

        # coupling constants
        self._k = couplings

        self.neighbors = {int(n): {"idx": idx, "iter": 0, "phi": None} for idx, n in enumerate(neighbors)}

        self.set_str()

        print(self)

    def _inited(self):
        return all([d["phi"] is not None for d in self.neighbors.values()])

    def set_str(self):
        self._str = "MicroController__{}".format(self._address)
        for add in self.neighbors.keys():
            self._str += "__neigh{}".format(add)

    def __str__(self):
        return self._str

    def update_neighbor(self, neighbor, value):
        """
        Update neighbors values
        """

        if not neighbor in self.neighbors.keys():
            raise ValueError("Wrong neighbor %s. Must be one in %s" % (neighbor, self.neighbors.keys()))

        if self._verbose:
            print('---------------------------')
            print("old neighbor: %s" % (self.neighbors[neighbor]))
        self.neighbors[neighbor]["phi"] = value
        self.neighbors[neighbor]["iter"] += 1
        if self._verbose:
            print("new neighbor: %s" % (self.neighbors[neighbor]))

    def update_value(self):
        """
        Update own value with Kuramoto ODE
        """

        # don't start unless values set for all neighbors
        if not self._inited():
            return

        if self._verbose:
            print('---------------------------')
            print("old data: %s" % (self.data))

        # print('---------------------------')
        # print("self._w %f" % self._w)
        # print("self._dt %f" % self._dt)
        old_y = self.data["phi"]
        # print("old_y %f" % old_y)
        tmp = 0.
        for d in self.neighbors.values():
            # print("k %f" % self._k[d['idx']])
            print (np.sin(old_y-d["phi"]))
            tmp += (self._k[d['idx']] * np.sin(old_y-d["phi"]))
        new_y = old_y + self._dt * (self._w+tmp)
        print("new_y %f" % new_y)

        self.data["phi"] = new_y
        self.data["iter"] += 1

        if self._verbose:
            print("new data: %s" % (self.data))

    def send_value(self):
        for add, d in self.neighbors.items():
            c = OSCClient()
            c.connect(("localhost", add))
            msg = OSCMessage("/receive")
            msg.append(self._address)
            msg.append(self.data["phi"])
            c.send(msg)
            c.close()
