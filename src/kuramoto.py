import numpy as np


def kuramoto_ODE(t, y, arg):
    """General Kuramoto ODE of m'th harmonic order.
       Argument `arg` = (w, k), with
        w -- iterable frequency
        k -- 3D coupling matrix, unless 1st order
        """

    w, k = arg
    yt = y[:, None]
    dy = y - yt
    phase = w.astype(np.float32)
    # add noise
    # phase += 0.0001*np.random.uniform(len(phase))
    phase += np.sum(k*np.sin(dy), axis=1)
    return phase


def kuramoto_ODE_1(t, y, arg):
    """General Kuramoto ODE of m'th harmonic order.
       Argument `arg` = (w, k), with
        w -- iterable frequency
        k -- 3D coupling matrix, unless 1st order
        """

    w, k, y_others = arg
    phase = w
    # add noise
    # phase += 0.0001*np.random.uniform(len(phase))
    phase += np.array([k*np.sin(y - y_j) for y_j in y_others]).sum()
    return phase