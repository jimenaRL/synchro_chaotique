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
                 init_value=0.0,
                 frequency=1,
                 coupling=10,
                 dt=0.01,
                 method='euler',
                 verbose=0,
                 monitoring=0):
        """
            address:
                [int] identifaction address (own port)
            neighbors:
                [list if ints] list of neighbors identifaction (neighbors ports)
            init_value:
                [float]
            frequency:
                [float]
            coupling:
                [float]
            dt:
                [float]
            method:
                [str]
            verbose:
                [int]
            monitoring:
                [int]
        """

        if method in ['euler', 'runge-kutta']:
            self._method = method
        else:
            raise ValueError("Wrong integration method '%s'. \
                             Must be one in %s" % (method, ['euler', 'runge-kutta']))
        if method == 'runge-kutta':
            from scipy.integrate import ode
            from kuramoto import kuramoto_ODE_1

        self._verbose = verbose
        self._monitoring = monitoring
        self._address = int(address)
        self._dt = np.float32(dt)

        self.data = {"iter": 0, "value": init_value}
        self._t = 0.

        self.neighbors = {int(n): {"iter": 0, "value": None} for n in neighbors}

        self.set_str()

        # intrinsic frequency
        self._w = frequency

        # coupling constant
        self._k = coupling / float(len(self.neighbors)+1)

        print(self)

    def _inited(self):
        return all([d["value"] is not None for d in self.neighbors.values()])

    def set_str(self):
        self._str = "MicroController__{}".format(self._address)
        for add in self.neighbors.keys():
            self._str += "__neigh{}".format(add)

    def __str__(self):
        return self._str

    def update_neighbor(self, neighbor, value, _iter):
        """
        Update a neighbor's value.
        """

        if not neighbor in self.neighbors.keys():
            print("WARNING:Wrong neighbor '%s'. Must be one in %s" % (neighbor, self.neighbors.keys()))
            return

        self.neighbors[neighbor]["value"] = value
        self.neighbors[neighbor]["iter"] = int(_iter)

        if self._verbose:
            print("new neighbor %i: %s" % (neighbor, self.neighbors[neighbor]))

    def update_value(self):
        if self._method == 'euler':
            return self.update_value_euler()
        else:  # self._method == 'runge-kutta':
            return self.update_value_runge_kutta()

    def update_value_euler(self):
        """
        Update own value with forward Euler method for integrating Kuramoto ODE.
        """

        # don't start unless values are set for all neighbors
        if not self._inited():
            return

        # check sychronisation with neighbors
        if self._verbose:
            if not all([(self.data["iter"]) == v["iter"] for v in self.neighbors.values()]):
                deca = [(n, self.data["iter"]-v["iter"]) for n, v in self.neighbors.items()]
                print("[sychro problem] %s: %s" % (self.data["iter"], deca))
        old_y = self.data["value"]
        tmp = 0.
        for d in self.neighbors.values():
            tmp += (self._k * np.sin(d["value"]-old_y))
        new_y = old_y + self._dt * (self._w + tmp)

        self.data["value"] = new_y
        self.data["iter"] += 1

        # check sychronisation with neighbors
        if self._monitoring:
            print("%s %s" % (self.data["iter"], self.data["value"]))

        if self._verbose:
            print("new data: %s" % (self.data))

        return self.data["value"]

    def update_value_runge_kutta(self):
        """
        Update own value with Runge Kuta method for integrating Kuramoto ODE.
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

        old_y = self.data["value"]
        old_t = self._t
        y_others = np.array([d["value"] for d in self.neighbors.values()])

        new_t = old_t + self._dt

        # Set parameters into model
        kODE.set_initial_value(old_y,  old_t)
        kODE.set_f_params((self._w, self._k, y_others))

        kODE.integrate(new_t)

        self.data["value"] = kODE.y
        self.data["iter"] += 1

        self._t = new_t

        if self._verbose:
            print("new data: %s" % (self.data))

        return self.data["value"]

    def send_value(self):
        for add, d in self.neighbors.items():
            c = OSCClient()
            c.connect(("localhost", add))
            msg = OSCMessage("/receive")
            msg.append(self._address)
            msg.append(self.data["value"])
            msg.append(self.data["iter"])
            c.send(msg)
            c.close()
