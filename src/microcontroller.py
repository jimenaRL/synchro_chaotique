from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import numpy as np
from OSC import OSCClient, OSCMessage

import logging
logger = logging.getLogger(__name__)


class MicroController(object):

    def set_str(self):
        self._str = "MicroController__{}".format(self._address)
        for add in self.neighbors.keys():
            self._str += "__neigh{}".format(add)

    def __str__(self):
        return self._str

    def __init__(self, address='', neighbors=[]):
        """
            address:
                [int] identifaction address (own port)
            neighbors:
                list of neighbors identifaction (neighbors ports)
        """

        self._address = int(address)
        self.data = {"iter": 0, "phi": 0.}

        # intrinsic frequency
        self._w = .9

        # coupling constants
        self._k = [-0.5, 0.2]

        self.neighbors = {int(n): {"idx": idx, "iter": 0, "phi": 0.} for idx, n in enumerate(neighbors)}

        self.set_str()

        print(self)

    def update_neighbor(self, neighbor, value):
        """
        Update neighbors values
        """

        if not neighbor in self.neighbors.keys():
            raise ValueError("Wrong neighbor %s. Must be one in %s" % (neighbor, self.neighbors.keys()))

        print('---------------------------')
        print("old neighbor: %s" % (self.neighbors[neighbor]))
        self.neighbors[neighbor]["phi"] = value
        self.neighbors[neighbor]["iter"] += 1
        print("new neighbor: %s" % (self.neighbors[neighbor]))

    def update_value(self, dt):
        """
        Update own value with Kuramoto ODE
        """

        print('---------------------------')
        print("old data: %s" % (self.data))

        old_y = self.data["phi"]
        new_y = old_y + dt * self._w
        for d in self.neighbors.values():
            new_y += self._k[d['idx']] * np.sin(old_y-d["phi"])

        self.data["phi"] = new_y
        self.data["iter"] += 1

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
