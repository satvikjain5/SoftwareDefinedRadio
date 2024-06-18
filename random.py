#!/usr/bin/python3

freq = 30

import numpy as np
dt = 0.001
tmin = -0.05
tmax = 0.05
t = np.arange(tmin,tmax,dt)
NSamples = len(t)
from numpy import pi, exp
w = (1 - 2*(pi*freq*t)**2) * exp(-(pi*freq*t)**2)
import matplotlib.pyplot as plt

plt.plot(t, w)
plt.xlabel('Time, s')
plt.ylabel('Amplitude')
plt.show()
