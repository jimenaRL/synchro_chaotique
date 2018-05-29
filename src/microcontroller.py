from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import numpy as np
from scipy.integrate import ode
from kuramoto import kuramoto_ODE_1

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
                 method='naive',
                 verbose=1):
        """
            address:
                [int] identifaction address (own port)
            neighbors:
                list of neighbors identifaction (neighbors ports)
        """

        if method in ['naive', 'runge-kutta']:
            self._method = method
        else:
            raise ValueError("Wrong integration method '%s'. \
                             Must be one in %s" % (method, ['naive', 'runge-kutta']))

        self._verbose = verbose
        self._address = int(address)
        self._dt = np.float32(dt)

        self.data = {"iter": 0, "phi": float(init)}
        self._t = 0.

        # intrinsic frequency
        self._w = frequency

        # coupling constants
        self._k = couplings

        self.neighbors = {int(n): {"idx": idx, "iter": 0, "phi": None}
                          for idx, n in enumerate(neighbors)}

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
            raise ValueError("Wrong neighbor '%s'. Must be one in %s" % (neighbor, self.neighbors.keys()))

        if self._verbose:
            print('---------------------------')
            print("old neighbor: %s" % (self.neighbors[neighbor]))
        self.neighbors[neighbor]["phi"] = value
        self.neighbors[neighbor]["iter"] += 1
        if self._verbose:
            print("new neighbor: %s" % (self.neighbors[neighbor]))

    def update_value(self):
        if self._method == 'naive':
            return self.update_value_naive()
        else:  # self._method == 'runge-kutta':
            return self.update_value_runge_kutta()

    def update_value_naive(self):
        """
        Update own value with naive approx of order 1 to Kuramoto ODE.
        """

        # don't start unless values set for all neighbors
        if not self._inited():
            return

        if self._verbose:
            print('---------------------------')
            print("old data: %s" % (self.data))

        old_y = self.data["phi"]
        tmp = 0.
        for d in self.neighbors.values():
            tmp += (self._k[d['idx']] * np.sin(old_y-d["phi"]))
        new_y = old_y + self._dt * (self._w+tmp)

        self.data["phi"] = new_y
        self.data["iter"] += 1

        if self._verbose:
            print("new data: %s" % (self.data))

        return self.data["phi"]

    def update_value_runge_kutta(self):
        """
        Update own value with Runge Kuta method for integrating to Kuramoto ODE.
        """

        # don't start unless values set for all neighbors
        if not self._inited():
            return

        if self._verbose:
            print('---------------------------')
            print("old data: %s" % (self.data))

        # Set ODE integrator (explicit runge-kutta method of order (4)5)
        kODE = ode(kuramoto_ODE_1)
        kODE.set_integrator("dopri5")

        old_y = self.data["phi"]
        old_t = self._t
        y_others = np.array([d["phi"] for d in self.neighbors.values()])

        new_t = old_t + self._dt

        # Set parameters into model
        kODE.set_initial_value(old_y,  old_t)
        kODE.set_f_params((self._w, self._k, y_others))

        kODE.integrate(new_t)

        self.data["phi"] = kODE.y
        self.data["iter"] += 1

        self._t = new_t

        if 1:  # self._verbose:
            print("new data: %s" % (self.data))

        return self.data["phi"]

    def send_value(self):
        for add, d in self.neighbors.items():
            c = OSCClient()
            c.connect(("localhost", add))
            msg = OSCMessage("/receive")
            msg.append(self._address)
            msg.append(self.data["phi"])
            c.send(msg)
            c.close()
